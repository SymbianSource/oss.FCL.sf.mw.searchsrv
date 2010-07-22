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

#include "CLucene.h"

#include <iostream>

#include "testutils.h"

#include "evaluationtool.h"
#include "analysisunittest.h"
#include "koreananalyzer.h"

using namespace analysis; 
using namespace evaluationtool; 
using namespace lucene::util;

using namespace lucene::analysis; 

void KoreanAnalyzerTest(Itk::TestMgr* /*testMgr*/) {
	analysis::KoreanAnalyzer analyzer;
	
	Corpus corpus(KOREAN_TEXTCORPUS); 
	
	for (int i = 0; i < corpus.size(); i++) {
		printTokens(analyzer, corpus[i]);
	}
}

Itk::TesterBase * CreateKoreanAnalyzerUnitTest() 
{
	using namespace Itk;
	SuiteTester
		* testSuite = 
			new SuiteTester( "korean" );
	
	testSuite->add( "korean", KoreanAnalyzerTest, "korean" );
	return testSuite;
}
