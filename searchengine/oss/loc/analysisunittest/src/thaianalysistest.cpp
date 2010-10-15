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
#include "thaistatemachine.h"


using namespace analysis; 
using namespace evaluationtool; 
using namespace lucene::util;

using namespace lucene::analysis; 

#define THAI_TEXTCORPUS "c:\\data\\analysisunittestcorpus\\thai\\corpus.txt"

#define BUFFER_SIZE 512

void printBreaks(BreakIterator& breaks, const wchar_t* text)
{
	breaks.setText( text );

	while (breaks.hasNext()) 
	{
		int begin = breaks.current(); 
		int end = breaks.next();
		
		wchar_t buf[BUFFER_SIZE];
		memcpy(buf, text+begin, sizeof(wchar_t)*(end-begin));
		buf[end-begin] = '\0';
		printf(" '%S'", buf);
	}
	printf("\n");
}

void thaiBreakIteratorTest(Itk::TestMgr* testMgr) 
{
	std::auto_ptr<BreakIterator> breaks( ThaiAnalysisInfra::theInstance()->createBreakIterator() );
	
	FileReader reader(THAI_TEXTCORPUS, "UTF-8");
	
	Corpus corpus(THAI_TEXTCORPUS); 
	
	for (int i = 0; i < corpus.size(); i++) {
		printBreaks(*breaks, corpus[i]);
	}
}

void thaiAnalyzerTest(Itk::TestMgr* testMgr)
{
	ThaiAnalyzer analyzer;
	
	Corpus corpus(THAI_TEXTCORPUS); 
	
	for (int i = 0; i < corpus.size(); i++) {
		printTokens(analyzer, corpus[i]);
	}
}

void thaiAnalyzerSpeed(Itk::TestMgr* testMgr) 
{
	ThaiAnalyzer analyzer;
	FileReader reader(THAI_TEXTCORPUS, "UTF-8");
	int filesize = Cpt::filesize(THAI_TEXTCORPUS);
	
	Itk::Timestamp begin;
	Itk::getTimestamp(&begin);

	auto_ptr<TokenStream> stream( analyzer.tokenStream( NULL, &reader ) );
	lucene::analysis::Token token; 
	while (stream->next(&token)); // go throught all tokens

	Itk::Timestamp end;
	Itk::getTimestamp(&end);
	
	long time = Itk::getElapsedMs(&end, &begin); 
	ITK_REPORT( testMgr, "Thai analysis time", "%d ms / %d KB", time, (filesize/1000));
	ITK_REPORT( testMgr, "Thai analysis speed", "%d KB/s", (filesize / time));
}


void thaiControlSpeed(Itk::TestMgr* testMgr) 
{
	lucene::analysis::standard::StandardAnalyzer analyzer;
	FileReader reader(THAI_TEXTCORPUS, "UTF-8");
	int filesize = Cpt::filesize(THAI_TEXTCORPUS);
	
	Itk::Timestamp begin;
	Itk::getTimestamp(&begin);

	auto_ptr<TokenStream> stream( analyzer.tokenStream( NULL, &reader ) );
	lucene::analysis::Token token; 
	while (stream->next(&token)); // go throught all tokens

	Itk::Timestamp end;
	Itk::getTimestamp(&end);
	
	long time = Itk::getElapsedMs(&end, &begin); 
	ITK_REPORT( testMgr, "Thai control time", "%d ms / %d KB", time, (filesize/1000));
	ITK_REPORT( testMgr, "Thai control speed", "%d KB/s", (filesize / time));
}



Itk::TesterBase * CreateThaiAnalysisUnitTest() 
{
	using namespace Itk;
	
	SuiteTester
		* testSuite = 
			new SuiteTester( "thai" );
	
	testSuite->add( "breaks", thaiBreakIteratorTest, "breaks" );
	testSuite->add( "analyzer", thaiAnalyzerTest, 	 "analyzer" );
	testSuite->add( "analyzerSpeed", thaiAnalyzerSpeed );
	testSuite->add( "controlSpeed", thaiControlSpeed );
	
	return testSuite;
}

