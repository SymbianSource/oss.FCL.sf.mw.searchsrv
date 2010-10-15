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

#include "std_log_result.h"

const char * Utf8DocsToIndex[5] = {
    FILE_TEST_CORPUS_PATH "\\fi\\1.txt",
    FILE_TEST_CORPUS_PATH "\\fi\\2.txt",
    FILE_TEST_CORPUS_PATH "\\fi\\3.txt",
    FILE_TEST_CORPUS_PATH "\\fi\\4.txt",
    NULL
};

const wchar_t * UcsTermsToSearch[5] = {
	L"teetä",
	L"näin",
	L"hyvä",
	L"yötyössä",
    NULL
};

void TestUtf8(Itk::TestMgr * testMgr)
{
        cpix_Result
            result;

        cpix_IdxDb_dbgScrapAll(&result);

        ITK_ASSERT(testMgr,
                   cpix_Succeeded(&result),
                   "Could not get rid of all test qbac-idx pairs");

	std::auto_ptr<FileIdxUtil> util( new FileIdxUtil ); 
	
	util->init(); 
	
	cpix_Analyzer* analyzer = cpix_CreateSimpleAnalyzer(&result); 
	
	if ( cpix_Failed( &result) ) 
	    {

        ITK_PANIC("Analyzer could not be created");
	    }
	
	for (int i = 0; Utf8DocsToIndex[i]; i++) 
	{
		util->indexFile( Utf8DocsToIndex[i], analyzer, testMgr ); 
	}

        util->flush();
	
	for (int i = 0; UcsTermsToSearch[i]; i++) 
	{
		cpix_QueryParser
			* queryParser = cpix_QueryParser_create(&result,
													CONTENTS_FIELD,
													analyzer );
		if (queryParser == NULL)
			{

				cpix_Analyzer_destroy( analyzer );
				ITK_PANIC("Could not create query parser");
			}
	
		cpix_Query* query = cpix_QueryParser_parse(queryParser,
												   UcsTermsToSearch[i]);
		if (cpix_Failed(queryParser)
			|| query == NULL)
			{

				cpix_Analyzer_destroy(analyzer);
				cpix_ClearError(queryParser);
				cpix_QueryParser_destroy(queryParser);
				ITK_PANIC("Could not parse query string");
			}
		cpix_QueryParser_destroy(queryParser);

		cpix_Hits
			* hits = cpix_IdxDb_search(util->idxDb(),
									   query );
		
		cpix_Query_destroy( query ); 
		
		util->printHits( hits, testMgr ); 
		
		cpix_Hits_destroy( hits ); 
	}
	
	cpix_Analyzer_destroy( analyzer ); 

}

Itk::TesterBase * CreateUtf8Tests()
{
    using namespace Itk;

    SuiteTester
        * whiteBox = new SuiteTester("utf8"); // default context

    whiteBox->add("utf8",
                  &TestUtf8,
                  "utf8");

    // TODO add more

    return whiteBox;
}
