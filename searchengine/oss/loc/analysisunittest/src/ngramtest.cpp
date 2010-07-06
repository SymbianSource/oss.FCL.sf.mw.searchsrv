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
#include "itk.h"

#include "thaianalysis.h"

#include "CLucene.h"

#include <iostream>

#include "testutils.h"

#include "evaluationtool.h"
#include "analysisunittest.h"
#include "cjkanalyzer.h"
#include "ngram.h"

using namespace analysis; 
using namespace evaluationtool; 
using namespace lucene::util;

using namespace lucene::analysis; 

template<int N>
void KoreanNgramAnalyzerTest(Itk::TestMgr* /*testMgr*/) {
	CjkNGramAnalyzer analyzer(N);
	
	Corpus corpus(KOREAN_TEXTCORPUS); 
	
	for (int i = 0; i < corpus.size(); i++) {
		printTokens(analyzer, corpus[i]);
	}
}

template<int N>
void JamuNgramAnalyzerTest(Itk::TestMgr* /*testMgr*/) {
    JamuNGramAnalyzer analyzer(N);
    
    Corpus corpus(KOREAN_TEXTCORPUS); 
    
    for (int i = 0; i < corpus.size(); i++) {
        printTokens(analyzer, corpus[i]);
    }
}

template<int N>
void ChinesePrcNgramAnalyzerTest(Itk::TestMgr* /*testMgr*/) {
	CjkNGramAnalyzer analyzer(N);
	
	Corpus corpus(CHINESE_PRC_TEXTCORPUS); 
	
	for (int i = 0; i < corpus.size(); i++) {
		printTokens(analyzer, corpus[i]);
	}
}

Itk::TesterBase * CreateNgramAnalyzerUnitTest() 
{
	using namespace Itk;
	SuiteTester
		* testSuite = 
			new SuiteTester( "ngram" );
	
	testSuite->add( "korean_1gram", KoreanNgramAnalyzerTest<1>, "korean_1gram" );
    testSuite->add( "korean_2gram", KoreanNgramAnalyzerTest<2>, "korean_2gram" );
	testSuite->add( "chinese_prc_1gram", ChinesePrcNgramAnalyzerTest<1>, "chinese_prc_1gram" );
    testSuite->add( "chinese_prc_2gram", ChinesePrcNgramAnalyzerTest<2>, "chinese_prc_2gram" );
    testSuite->add( "jamu_1gram", JamuNgramAnalyzerTest<1>, "jamu_1gram" );
    testSuite->add( "jamu_2gram", JamuNgramAnalyzerTest<2>, "jamu_2gram" );
    
	return testSuite;
}
