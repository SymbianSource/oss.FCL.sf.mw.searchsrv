/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/

// general utilities
#include "wchar.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>

// clucene
#include "CLucene.h"

// support
#include "cpixparsetools.h"
#include "cpixfstools.h"

// internal
#include "analyzer.h"
#include "cpixanalyzer.h"
#include "cpixexc.h"
#include "document.h"
#include "cluceneext.h"
#include "indevicecfg.h"
#include "initparams.h"
#include "thaianalysis.h"

#include "analyzerexp.h"
#include "customanalyzer.h"
#include "common/cpixlog.h"

namespace
{
    const char AGGR_NONFILEREADERPROXY_ERR[] 
    = "Aggregated reader field should be FileReaderProxy instance";

    const char AGGR_STREAMREADER_ERR[] 
    = "Aggregating streamValue-fields not implemented";
    
    const char THAI_LANGUAGE_FILE[] 
    = "thaidict.sm";

    const char ANALYZER_FILE[]
    = "analyzer.loc";

    const wchar_t DEFAULT_ANALYZER_CONFIG[]
        = L"default";

    const wchar_t QUERY_ANALYZER_CONFIG[]
        = L"query";

    const wchar_t PREFIX_ANALYZER_CONFIG[]
        = L"prefix";

//    const wchar_t CPIX_ANALYZER_FALLBACK[]
//    = CPIX_ANALYZER_STANDARD;
//
//    const wchar_t CPIX_PREFIX_ANALYZER_FALLBACK[]
//    = CPIX_TOKENIZER_LETTER L">" CPIX_FILTER_LOWERCASE;

    
}


namespace Cpix {

	
Analysis* Analysis::theInstance_ = NULL; 

	void Analysis::init(InitParams& ip) {
		// Init thai analysis with thai dictionary
		std::string thai( Cpt::appendpath(ip.getResourceDir(),
										  THAI_LANGUAGE_FILE) );
		
		if ( Cpt::filesize( thai.c_str() ) ) {
			analysis::InitThaiAnalysis(thai.c_str());
		} else {
			logMsg(CPIX_LL_WARNING,
				   "Thai dictionary could not be found. Thai analysis will NOT work.");
		}
	
		// Setup the analysis instance
		theInstance_ = new Analysis(ip);
	}
	
	Analysis::Analysis(InitParams& ip) 
	:	defaultAnalyzer_(),
	 	queryAnalyzer_(), 
		prefixAnalyzer_() {
		
		auto_ptr<AnalyzerExp::Piping> p = parse( Cpt::appendpath( ip.getResourceDir(), ANALYZER_FILE ) );
		
		defaultAnalyzer_.reset( new CustomAnalyzer( *p, DEFAULT_ANALYZER_CONFIG ) ); 
		queryAnalyzer_.reset( new CustomAnalyzer( *p, QUERY_ANALYZER_CONFIG ) ); 
		prefixAnalyzer_.reset( new CustomAnalyzer( *p, PREFIX_ANALYZER_CONFIG ) ); 
	}
	
	auto_ptr<AnalyzerExp::Piping> Analysis::parse(std::string path) {
		std::wifstream in(path.c_str());
		auto_ptr<AnalyzerExp::Piping> ret; 
		if ( in ) {
		
			// Reserve constant size buffer and populate it with definition
			//
			int filesize = Cpt::filesize(path.c_str()); 
			Cpt::auto_array<wchar_t> buf( new wchar_t[filesize+1] );
			in.read(buf.get(), filesize);
			buf.get()[filesize] = '\0'; 
			if ( !in.fail() ) {
				try {
					ret = AnalyzerExp::ParsePiping( buf.get() );
				} catch (...) {}
			} 
			in.close();
		} 
		
		if ( !ret.get() ) { 
			THROW_CPIXEXC("Analyzer definition not found. %s could not be opened. ", path.c_str()); 
		}
		return ret; 
	}
	
	void Analysis::shutdown() {
		analysis::ShutdownThaiAnalysis(); 
		delete theInstance_;
		theInstance_ = NULL; 
	}

	lucene::analysis::Analyzer& Analysis::getDefaultAnalyzer() {
		// TODO: Assert( theInstance_ );
		return *theInstance_->defaultAnalyzer_; 
	}

	lucene::analysis::Analyzer& Analysis::getQueryAnalyzer() {
		// TODO: Assert( theInstance_ );
		return *theInstance_->queryAnalyzer_; 
	}

	lucene::analysis::Analyzer& Analysis::getPrefixAnalyzer() {
		// TODO: Assert( theInstance_ );
		return *theInstance_->prefixAnalyzer_; 
	}

	PrefixGenerator::PrefixGenerator(
		lucene::analysis::TokenStream* in, 
		bool deleteTS, 
		size_t maxPrefixLength) 
	: 	TokenFilter(in, deleteTS),
	  	token_(), 
	  	prefixLength_(0),
	  	maxPrefixLength_(maxPrefixLength) {}
	
	
	PrefixGenerator::~PrefixGenerator() {
	}

	
	bool PrefixGenerator::next(lucene::analysis::Token* token) {
		token_.setPositionIncrement(0); 

		while (prefixLength_ == 0) {
			token_.setPositionIncrement(1); // default position increment
			if (!input->next(&token_)) {
				return false;
			}
			prefixLength_ = std::min(token_.termTextLength(), maxPrefixLength_);
		}
			
		// Clip token
		std::wstring clipped; 
		clipped = token_.termText();
		token_.setText(clipped.substr(0, prefixLength_).c_str());
		
		// Copy
		token->set(token_.termText(), token_.startOffset(), token_.endOffset(), token_.type());
		token->setPositionIncrement(token_.getPositionIncrement());
		
		// Reduce prefixLength_
		prefixLength_--;
		return true; 
	}

    AggregateFieldTokenStream::AggregateFieldTokenStream(lucene::analysis::Analyzer& analyzer, 
                                                         DocumentFieldIterator* fields) 
	: stream_(), analyzer_( analyzer ), reader_(), fields_( fields ) {
        getNextStream(); 
    }
													   
    AggregateFieldTokenStream::~AggregateFieldTokenStream() {
        _CLDELETE( stream_ ); 
        delete fields_; 
    }
	
    bool AggregateFieldTokenStream::next(lucene::analysis::Token* token) {
        while ( stream_ ) {
            if ( stream_->next( token ) ) {
                return true;
            }
            getNextStream();
        }
        return false;
    }
		
    void AggregateFieldTokenStream::close() {
        if (stream_) stream_->close(); 
        _CLDELETE( stream_ ); 
        _CLDELETE( reader_ ); 
    }
	
    void AggregateFieldTokenStream::getNextStream()
    {
        using namespace lucene::document;
        using namespace lucene::util; 
	
        if ( stream_ ) stream_->close(); 
        _CLDELETE( stream_ ); 
        _CLDELETE( reader_ );
		
        Field* field = 0; 
        while (*fields_ && field == NULL)
            {
                field = (*fields_)++;
	
                if (!field->isAggregated()) 
                    {
						field = 0;
                    }
            }
        if (field) {
            if (field->stringValue() != NULL)
                {
                    reader_ = _CLNEW CL_NS(util)::StringReader(field->stringValue(),_tcslen(field->stringValue()),false);
                }
            else if (field->native().readerValue() != NULL)
                {
                    Reader* r = field->native().readerValue();
                    FileReaderProxy
                        * frp = 
                        dynamic_cast<FileReaderProxy*>(r);
                    if (frp == NULL)
                        {
                            _CLTHROWA(CL_ERR_IO, AGGR_NONFILEREADERPROXY_ERR);
                        }
                    else
                        {
                            reader_ = frp->clone();
                        }
                }
            else
                {
                    _CLTHROWA(CL_ERR_IO, AGGR_STREAMREADER_ERR);
                }
            if(field->isFreeText())
                {
                    using namespace lucene::analysis;
                    stream_ = _CLNEW standard::StandardTokenizer(reader_);
                    stream_ = _CLNEW standard::StandardFilter(stream_,true);
                    stream_ = _CLNEW LowerCaseFilter(stream_,true);
                }
			// if it is phonenumber, use phone number analyser
            else if(field->isPhoneNumber())
                {
                    lucene::analysis::Analyzer *PhoneNumerAnalyzer_;
                    
                    PhoneNumerAnalyzer_  = _CLNEW lucene::analysis::PhoneNumberAnalyzer(); 
                    stream_ = PhoneNumerAnalyzer_->tokenStream( field->name(), reader_ );
                    _CLDELETE(PhoneNumerAnalyzer_); 
                }
            else
            stream_ = analyzer_.tokenStream( field->name(), reader_ ); 
        }
    }
	
		
    AggregateFieldAnalyzer::AggregateFieldAnalyzer(Cpix::Document& document, 
                                                   lucene::analysis::Analyzer& analyzer) 
	:	analyzer_(analyzer), document_(document)
    {
    }
	
    lucene::analysis::TokenStream* AggregateFieldAnalyzer::tokenStream(const TCHAR     * fieldName, 
                                                                       lucene::util::Reader * reader) {
        if ( wcscmp( fieldName, LCPIX_DEFAULT_FIELD ) == 0 ) {
            return new AggregateFieldTokenStream( analyzer_, document_.fields()); 
        } else if ( wcscmp( fieldName, LCPIX_DEFAULT_PREFIX_FIELD ) == 0 ) {
            return
				new PrefixGenerator(
					new AggregateFieldTokenStream( analyzer_, document_.fields()),
					true,
					OPTIMIZED_PREFIX_MAX_LENGTH);
        } else {
            return analyzer_.tokenStream( fieldName, reader ); 
        }
    }
	
    SystemAnalyzer::SystemAnalyzer(lucene::analysis::Analyzer* analyzer) : analyzer_(analyzer) {} 
    SystemAnalyzer::~SystemAnalyzer() { _CLDELETE(analyzer_); }
	
    lucene::analysis::TokenStream* SystemAnalyzer::tokenStream(const TCHAR      	* fieldName, 
                                                               lucene::util::Reader * reader) {
        using namespace lucene::analysis; 
        if ( wcscmp( fieldName, LCPIX_DEFAULT_FIELD ) == 0 ) {
            // Use standard analyzer without stop filter for this task
            TokenStream* ret = _CLNEW standard::StandardTokenizer(reader);
            ret = _CLNEW standard::StandardFilter(ret,true);
            ret = _CLNEW LowerCaseFilter(ret,true);
            return ret;
        } else if (wcscmp( fieldName, LCPIX_DOCUID_FIELD) == 0){
            // Use standard analyzer without stop filter for this task
            return  _CLNEW KeywordTokenizer(reader);
        } else if (wcscmp( fieldName, LCPIX_APPCLASS_FIELD )  == 0){
            // Use standard analyzer without stop filter for this task
            TokenStream* ret = _CLNEW WhitespaceTokenizer(reader);
            ret = _CLNEW LowerCaseFilter(ret,true);
            return ret;
        } else if (wcscmp( fieldName, LCPIX_MIMETYPE_FIELD ) == 0) {
            TokenStream* ret = _CLNEW KeywordTokenizer(reader);
            return ret;
        } else {
            return analyzer_->tokenStream( fieldName, reader ); 
        }									 
    }
}

