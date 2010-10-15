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
#include "thaianalysis.h"

#include "cpixfstools.h"

#include "CLucene/analysis/standard/StandardTokenizer.h"

#include <iostream>
#include <fstream>

#include "tinyunicode.h"

#include "thaistatemachine.h"

namespace analysis {

	void InitThaiAnalysis(const char* thaiDataFile) {
		ThaiAnalysisInfra::init(thaiDataFile);
	}
	void ShutdownThaiAnalysis() {
		ThaiAnalysisInfra::shutdown();
	}

	ThaiAnalysisInfra* ThaiAnalysisInfra::theInstance_ = NULL; 

	const char* ThaiAnalysisInfraNotInitialized::what() const throw() {
		return "Thai analyzer infra was not initialized.";
	}

	const char* StateMachineFileNotFound::what() const throw() {
		return "Thai analyzer infra could not find specified StateMachine file.";
	}

	const char* StateMachineLoadingFailed::what() const throw() {
		return "Thai analyzer infra failed reading the specified StateMachine file.";
	}

	
	void ThaiAnalysisInfra::init(const char* dataFile) 
	{
		shutdown(); 
		theInstance_ = new ThaiAnalysisInfra(dataFile); 
	}
	
	ThaiAnalysisInfra* ThaiAnalysisInfra::theInstance()
	{
		if ( !theInstance_ ) throw ThaiAnalysisInfraNotInitialized();
		return theInstance_; 
	}
			
	void ThaiAnalysisInfra::shutdown()
	{
		delete theInstance_; 
		theInstance_ = 0; 
	}
			
	std::auto_ptr<BreakIterator> ThaiAnalysisInfra::createBreakIterator()
	{
		if ( !blob_.get() )
		{	// load lazily
			off_t size = Cpt::filesize(dataFile_.c_str());
			
			if ( !size ) throw StateMachineFileNotFound();
			
			blob_.reset( new byte_t[size] );  
			
			std::ifstream in( dataFile_.c_str(), std::ifstream::in | std::ifstream::binary );
			
			if ( !in ) throw StateMachineFileNotFound();

			in.read( reinterpret_cast<char*>( blob_.get() ), size );
			
			if ( in.fail() ) throw StateMachineLoadingFailed();  
			
			in.close(); 
			
			stateMachine_.reset(blob_.get());
		}

		return std::auto_ptr<BreakIterator>( new StateMachineBreakIterator<ThaiSmEncoding>( stateMachine_ ) );
	}
			
	ThaiAnalysisInfra::ThaiAnalysisInfra(const char* dataFile)
	: 	blob_(0), 
	    stateMachine_(),
	  	dataFile_(dataFile) 
	{
		// sanity check
		if ( !Cpt::filesize(dataFile) ) throw StateMachineFileNotFound(); 
	}
			
	ThaiAnalysisInfra::~ThaiAnalysisInfra()
	{}
	
	ThaiWordFilter::ThaiWordFilter( lucene::analysis::TokenStream* input, 
									bool deleteTs )
	:	TokenFilter(input, deleteTs),
		breaks_(),
		thaiToken_()
	{
		breaks_ = ThaiAnalysisInfra::theInstance()->createBreakIterator(); 
	}
	
	using namespace lucene::analysis; 
		
	ThaiWordFilter::~ThaiWordFilter()
	{}
	
#define MAX_BUFSIZE 256
		
	bool ThaiWordFilter::next(Token* token)
	{
		if ( breaks_->hasNext() ) 
		{
			size_t wordBegin = breaks_->current(); 
			size_t wordLength = breaks_->next() - wordBegin;
			
			wchar_t buf[MAX_BUFSIZE];
			memcpy( buf, 
					thaiToken_.termText()+wordBegin, 
					wordLength * sizeof(wchar_t) );
			buf[wordLength] = '\0';
			
			token->set( buf, 
						thaiToken_.startOffset() + wordBegin,  
						thaiToken_.endOffset() + wordBegin + wordLength);
			return true; 
		}
		
		if ( input->next( token ) )
		{
			if ( unicode::IsThai( token->termText()[0] ) )
			{
				thaiToken_.set( token->termText(), token->startOffset(), token->endOffset() );
				breaks_->setText( thaiToken_.termText()); // reset
				return next( token );  
			} else {
				return true;
			}
		}
	
		return false;
	}
	

	using namespace lucene::analysis::standard; 
	
	ThaiAnalyzer::ThaiAnalyzer()
	:	stopWords_(false)
	{
		StopFilter::fillStopTable( &stopWords_,CL_NS(analysis)::StopAnalyzer::ENGLISH_STOP_WORDS);
	}
		
	lucene::analysis::TokenStream* ThaiAnalyzer::tokenStream(const wchar_t* fieldName, 
														    CL_NS(util)::Reader* reader)
	{
		auto_ptr<TokenStream> ret(  new StandardTokenizer(reader) ); 
		
		ret.reset( new LowerCaseFilter( ret.release(), true ) ); 
		ret.reset( new StandardFilter( ret.release(), true ) ); 
		ret.reset( new ThaiWordFilter( ret.release(), true ) ); 
		ret.reset( new StopFilter( ret.release(), true, &stopWords_ ) ); 
		
		return ret.release();  
	}
	
}
