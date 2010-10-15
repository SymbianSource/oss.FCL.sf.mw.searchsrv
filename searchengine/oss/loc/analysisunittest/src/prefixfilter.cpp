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
#include "prefixfilter.h"

using namespace analysis; 
using namespace evaluationtool; 
using namespace lucene::util;

using namespace lucene::analysis; 

void FrenchAnalyzerTest(Itk::TestMgr* /*testMgr*/) {
	analysis::FrenchAnalyzer analyzer;
	
	Corpus corpus(FRENCH_TEXTCORPUS); 
	
	for (int i = 0; i < corpus.size(); i++) {
		printTokens(analyzer, corpus[i]);
	}
}

void HebrewAnalyzerTest(Itk::TestMgr* /*testMgr*/) {
	analysis::HebrewAnalyzer analyzer;
	
	Corpus corpus(HEBREW_TEXTCORPUS); 
	
	for (int i = 0; i < corpus.size(); i++) {
		printTokens(analyzer, corpus[i]);
	}
}

Itk::TesterBase * CreatePrefixAnalyzerUnitTest() 
{
	using namespace Itk;
	SuiteTester
		* testSuite = 
			new SuiteTester( "prefix" );
	
	testSuite->add( "french", FrenchAnalyzerTest, "french" );
	testSuite->add( "hebrew", HebrewAnalyzerTest, "hebrew" );
	return testSuite;
}
