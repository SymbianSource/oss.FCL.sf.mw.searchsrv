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

#include <stdlib.h>
#include <wchar.h>
#include <glib.h>

#include <algorithm>

#include "CLucene.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"

#include "CLucene/util/VoidMap.h"
#include "CLucene/util/Reader.h"
#include "CLucene/util/bufferedstream.h"
#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/analysis/Analyzers.h"
#include "CLucene/analysis/standard/StandardTokenizerConstants.h"
#include "CLucene/analysis/standard/StandardTokenizer.h"


// Snowball API
#include "libstemmer.h"

#include "cpixstrtools.h"

#include "cpixsearch.h"
#include "cpixidxdb.h"
#include "cpixanalyzer.h"
#include "idxdb.h"
#include "initparams.h"
#include "cpixutil.h"
#include "cluceneext.h"
#include "cpixexc.h"
#include "indevicecfg.h"

namespace lucene 
{ 

    namespace analysis {

        

        /**
         * Method definitions
         */
        SnowballFilter::SnowballFilter(TokenStream * in,
                                       bool          deleteTokenStream,
                                       cpix_LangCode langCode)
            : TokenFilter(in,
                          deleteTokenStream),
              stemmer_(NULL)
        {
            // FIXME TODO CHECK: is it always UTF_8 we are going to use?
            stemmer_ = sb_stemmer_new(langCode.code_,
                                      "UTF_8");

            if (stemmer_ == NULL)
                {
                    THROW_CPIXEXC("Failed to create stemmer for '%s'",
                                  langCode.code_);
                }
        }



        SnowballFilter::~SnowballFilter()
        {
            if (stemmer_ != NULL)
                {
                    sb_stemmer_delete(stemmer_);
                    stemmer_ = NULL;
                }
        }
        
        

        bool SnowballFilter::next(Token * token)
        {
            if (!input->next(token))
                {
                    return false;
                }

            using namespace std;

            /* OBS
            char
                * utf8Text = new char[token->termTextLength() + 1];
            wcstombs(utf8Text,
                     token->termText(),
                     token->termTextLength() + 1);
            */

            Cpt::auto_array<char>
                utf8Text(token->termText(),
                         token->termTextLength());

            const sb_symbol
                * stemmed = sb_stemmer_stem(stemmer_,
                                            // OBS (unsigned char*)utf8Text,
                                            (unsigned char*)utf8Text.get(),
                                            token->termTextLength());

            // OBS delete[] utf8Text;

            if (stemmed == NULL)
                {
                    THROW_CPIXEXC(L"Stem failure. Out of memory.");
                }

            /* OBS
            Cpt::auto_array<wchar_t>
                ucs2Text(new wchar_t[sb_stemmer_length(stemmer_) + 1]);
            mbstowcs(ucs2Text.get(),
                     (const char*)stemmed,
                     sb_stemmer_length(stemmer_) + 1);
            */

            Cpt::auto_array<wchar_t>
                ucs2Text(reinterpret_cast<const char*>(stemmed));

            token->setText(ucs2Text.get());

            return true;
        }



        SnowballAnalyzer::SnowballAnalyzer(cpix_LangCode langCode)
            : stopSet_(false),
              langCode_(langCode)
        {
            // TODO FIXME
            // (1) check if we want to filter out stop words
            // (2) if so, we need 
            StopFilter::fillStopTable(&stopSet_,
                                      StopAnalyzer::ENGLISH_STOP_WORDS);
        }


        SnowballAnalyzer::SnowballAnalyzer(const TCHAR      ** stopWords,
                                           cpix_LangCode       langCode)
            : stopSet_(false),
              langCode_(langCode)
        {
            StopFilter::fillStopTable(&stopSet_,
                                      stopWords);
        }


        SnowballAnalyzer::~SnowballAnalyzer()
        {
            ;
        }


        TokenStream* SnowballAnalyzer::tokenStream(const TCHAR * fieldName, 
                                                   lucene::util::Reader * reader)
        {
            TokenStream
                * ret = _CLNEW lucene::analysis::standard::StandardTokenizer(reader);
            ret = _CLNEW SnowballFilter(ret, 
                                        true,
                                        langCode_);
            // FIXME TODO CHECK if this is the right order?
            ret = _CLNEW LowerCaseFilter(ret, true);
            ret = _CLNEW StopFilter(ret,
                                    true,
                                    &stopSet_);

            return ret;
        }
        
                	
    } // namespace analysis



} // namespace lucene


namespace lucene
{
    namespace util
    {
        FileReaderProxy * FileReaderProxy::clone() const
        {
            FileReaderProxy
                * rv = _CLNEW FileReaderProxy(path_.c_str(),
											  enc_.c_str());

            return rv;
        }
    
    
        FileReaderProxy::~FileReaderProxy()
        {
            if (fileReader_ != NULL)
                {
                    delete fileReader_;
                    fileReader_ = NULL;
                    reader = NULL;
                }
        }
    
    
        FileReaderProxy::FileReaderProxy(const char * path,
                                         const char * enc)
            : Reader(NULL, false),
              fileReader_(NULL),
              path_(path),
              enc_(enc)
        {
            ;
        }
    
    
        void FileReaderProxy::initialize()
        {
            if (fileReader_ == NULL)
                {
                    fileReader_ = new FileReader(path_.c_str(),
                                                 enc_.c_str());

                    reader = fileReader_->reader;
                    deleteReader = false;
                }
        }




    } // ns
} // ns
