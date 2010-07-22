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

#include <wchar.h>
#include <stddef.h>

#include <iostream>

#include "cpixidxdb.h"

#include "itk.h"

#include "config.h"
#include "testutils.h"
#include "cpixanalyzer.h"

#include "cpixdoc.h"

#include "std_log_result.h"

const char * AnalysisTestDocsToIndex[5] = {
    FILE_TEST_CORPUS_PATH "\\en\\1.txt",
    FILE_TEST_CORPUS_PATH "\\en\\2.txt",
    FILE_TEST_CORPUS_PATH "\\en\\3.txt",
    FILE_TEST_CORPUS_PATH "\\en\\4.txt",
    NULL
};

const wchar_t * AnalyzerTestTermsToSearch[5] = {
	L"happy",
	L"happiness",
	L"happening",
    NULL
};


void TestAnalyzerParsing(Itk::TestMgr * , const wchar_t* definition, int expected = 1) 
{
	cpix_Result result; 
	
	printf("Creating analyzer %S\n", definition);
	
	cpix_Analyzer* analyzer = cpix_Analyzer_create( &result, definition);

	if ( cpix_Failed( &result) ) {
        if(expected)
            assert_failed = 1;
        else
            assert_failed = 0;            
		printf("Analyzer creation failed with %S\n", result.err_->msg_);
		return; 
	}
	cpix_Analyzer_destroy( analyzer ); 
}

void TestAnalyzersParsing(Itk::TestMgr * testMgr) 
{
    char *xml_file = (char*)__FUNCTION__;
    assert_failed = 0;
	TestAnalyzerParsing(testMgr, L"stdtokens>lowercase"); 
	TestAnalyzerParsing(testMgr, L"whitespace>lowercase"); 
	TestAnalyzerParsing(testMgr, L"letter>lowercase"); 
	// special syntax
	TestAnalyzerParsing(testMgr, L"stdtokens()>lowercase"); 
	TestAnalyzerParsing(testMgr, L"stdtokens>lowercase()"); 
	// parameteres
	TestAnalyzerParsing(testMgr, L"stdtokens>lowercase>stem(en)"); 
	TestAnalyzerParsing(testMgr, L"letter>lowercase>stop(en)"); 
	TestAnalyzerParsing(testMgr, L"letter>lowercase>stop('a', 'an', 'the')");

	// bad syntaxes
	TestAnalyzerParsing(testMgr, L"letter><lowercase" ,0); 
	TestAnalyzerParsing(testMgr, L"38j_d fad23 4?q ca'wRA", 0 );
	TestAnalyzerParsing(testMgr, L"38.45_d fd23<ca'wRA", 0 ); 
	// parsing failures
	TestAnalyzerParsing(testMgr, L"letter>>lowercase", 0 ); 
	TestAnalyzerParsing(testMgr, L">letter>>lowercase lowercase", 0 ); 
	TestAnalyzerParsing(testMgr, L"letter lowercase", 0 );
	testResultXml(xml_file);
}
void TestSwitchParsing(Itk::TestMgr * testMgr) 
{
    char *xml_file = (char*)__FUNCTION__;
        assert_failed = 0;
	// Per field query syntax
	TestAnalyzerParsing(testMgr, L"switch {"
									 L"case '_docuid':          keyword; "
									 L"case '_appclass':        whitespace>lowercase;"
									 L"case 'title', 'message': standard>lowercase>stem(en)>stop(en);"
									 L"default:                 standard;"
								 L"}");
	TestAnalyzerParsing(testMgr, L"switch{ case '_qnr': whitespace; default: standard; }>lowercase");
	TestAnalyzerParsing(testMgr, L"switch{ default: 	standard; }");
	TestAnalyzerParsing(testMgr, L"switch{ case '_qnr': switch{ case '_docuid': keyword; default: whitespace; }; default: standard; }");
	TestAnalyzerParsing(testMgr, L"switch{ case '_mimetype': standard; default: whitespace; }; default: standard; }");
	testResultXml(xml_file);
}

void TestAnalyzerUsage(Itk::TestMgr * testMgr, const wchar_t* definition) 
{
	printf("Indexing and searching with %S\n", definition); 
	cpix_Result
        result;

    cpix_IdxDb_dbgScrapAll(&result);

	std::auto_ptr<FileIdxUtil> util( new FileIdxUtil ); 
	
	util->init(); 
	
	cpix_Analyzer* analyzer = cpix_Analyzer_create( &result, definition );
			
	if ( cpix_Failed( &result) ) {
		printf("Analyzer creation failed with %S\n", result.err_->msg_); 
		return; 
	}
	
	//
	// Add first few simple documents from english stem corpus
	// English test corpus is used, because part of the analyzers contain
	// english specific functinality, like stop words and stemming. 
	
	for (int i = 0; AnalysisTestDocsToIndex[i]; i++) 
	{
		util->indexFile( AnalysisTestDocsToIndex[i], analyzer, testMgr ); 
	}
	
	//
	// Then continue by adding an empty document. It is inserted as 
	// a special case.
	
	cpix_Document* doc = cpix_Document_create( &result, L"empty", "root file text", L"", LTEXTFILE_MIMETYPE );
	
	cpix_Field field;
	cpix_Field_initialize( &field, CONTENTS_FIELD, L"", cpix_STORE_YES | cpix_INDEX_TOKENIZED );
	
	cpix_Document_add( doc, &field ); 
	
	cpix_IdxDb_add( util->idxDb(), doc, analyzer );
	
	cpix_Document_destroy( doc );
	
	printf("\nIndexed empty item.\n"); 
	
        util->flush();

	// 
	// Commit searches and print the results
	
	cpix_QueryParser
		* queryParser = cpix_QueryParser_create(&result,
												CONTENTS_FIELD,
												analyzer );
	if (queryParser == NULL)
		{
			cpix_Analyzer_destroy( analyzer );
			ITK_PANIC("Could not create query parser");
		}
	
	for (int i = 0; AnalyzerTestTermsToSearch[i]; i++) 
	{
		cpix_Query* query = cpix_QueryParser_parse(queryParser,
												    AnalyzerTestTermsToSearch[i]);
		if (cpix_Failed(queryParser)
			|| query == NULL)
			{
				cpix_Analyzer_destroy(analyzer);
				cpix_ClearError(queryParser);
				cpix_QueryParser_destroy(queryParser);
				ITK_PANIC("Could not parse query string");
			}
		cpix_Hits
			* hits = cpix_IdxDb_search(util->idxDb(),
									   query );

		cpix_Query_destroy( query ); 

		if (cpix_Failed(util->idxDb())) 
			{
            assert_failed = 1;
			cpix_Analyzer_destroy(analyzer);
			cpix_ClearError(queryParser);
			cpix_QueryParser_destroy(queryParser);
			ITK_PANIC("Searching index database failed.");
			}
		else 
			{
			util->printHits( hits, testMgr ); 
			cpix_Hits_destroy( hits ); 
			}
	}
	cpix_QueryParser_destroy(queryParser);
	cpix_Analyzer_destroy( analyzer ); 
	
}

void TestAnalyzersUsage(Itk::TestMgr * testMgr) 
	{
    char *xml_file = (char*)__FUNCTION__;
    assert_failed = 0;
	TestAnalyzerUsage(testMgr, L"whitespace" ); 
	TestAnalyzerUsage(testMgr, L"letter>lowercase" ); 
	TestAnalyzerUsage(testMgr, L"stdtokens>lowercase>stem(en)"); 
	TestAnalyzerUsage(testMgr, L"letter>lowercase>stop(en)"); 
	TestAnalyzerUsage(testMgr, L"letter>lowercase>stop('a', 'an', 'the')");
	testResultXml(xml_file);
	}


Itk::TesterBase * CreateAnalysisWhiteBoxTests();


Itk::TesterBase * CreateAnalysisTests()
{
    using namespace Itk;

    SuiteTester
        * analysis = new SuiteTester("analysis");


    analysis->add(CreateAnalysisWhiteBoxTests());

    analysis->add("parsing",
                  &TestAnalyzersParsing,
                  "parsing");

    analysis->add("switchParsing",
                  &TestSwitchParsing,
                  "switchParsing");

    analysis->add("usage",
                  &TestAnalyzersUsage,
                  "usage");

    // TODO add more

    return analysis;
}
