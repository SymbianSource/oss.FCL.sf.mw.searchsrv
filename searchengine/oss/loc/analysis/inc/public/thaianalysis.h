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

#ifndef THAIWORDFILTER_H_
#define THAIWORDFILTER_H_

#include <memory>

#include "cpixstrtools.h"

#include "Clucene.h"

namespace analysis 
{
	// Forward declarations
	class BreakIterator;

	
	/**
	 * Thai analysis uses compiled dictionary and it needs to 
	 * know the dictionary's location in the file system. This dictionary 
	 * is then loaded when thai analysis is used for first time.
	 * The dictionary is kept in memory until shutdown.
	 */ 
	void InitThaiAnalysis(const char* thaiDataFile);
	
	/**
	 * Releases the thai dictionary, if loaded to memory 
	 */
	void ShutdownThaiAnalysis(); 
	
	/**
	 * Dictionary based token filtering. Reads tokens from the token stream. 
	 * If token beginning with Thai character is encountered, it breaks
	 * the token into a number of thai word tokens based on the dictionary
	 * with the longest matching algorithm.
	 */
	class ThaiWordFilter : public lucene::analysis::TokenFilter 
	{
	public:
	
		ThaiWordFilter( lucene::analysis::TokenStream* input, bool deleteTs );
		
		~ThaiWordFilter(); 
		
		bool next(lucene::analysis::Token* token); 
	
	private: 

		std::auto_ptr<BreakIterator> breaks_;
		
		lucene::analysis::Token thaiToken_;
		
	};
	
	/**
	 * Analyzer for thai language. Uses StandardAnalyzer to make preliminary
	 * tokenization and ThaiWordFilter to split Thai sentence tokens 
	 * into a number of Thai word tokens.  
	 */
	class ThaiAnalyzer : public lucene::analysis::Analyzer 
	{
	public:
	
		ThaiAnalyzer(); 
		
	public: 
		
		lucene::analysis::TokenStream* tokenStream(const wchar_t* fieldName, 
												   lucene::util::Reader* reader);
		
	private: 
		
		CL_NS(util)::CLSetList<const TCHAR*> stopWords_;

	};

}

#endif /* THAIWORDFILTER_H_ */
