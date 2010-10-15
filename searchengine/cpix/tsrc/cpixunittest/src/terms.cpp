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
/*
 * terms.cpp
 *
 *  Created on: 4.6.2009
 *      Author: arau
 */


#include <wchar.h>
#include <stddef.h>

#include <iostream>
#include <sstream>
#include "indevicecfg.h"

#include "cpixidxdb.h"

#include "itk.h"

#include "config.h"
#include "testutils.h"
#include "suggestion.h"

#include "std_log_result.h"

const char * TermTestDocsToIndex[5] = {
	FILE_TEST_CORPUS_PATH "\\en\\1.txt",
	FILE_TEST_CORPUS_PATH "\\en\\2.txt",
	FILE_TEST_CORPUS_PATH "\\en\\3.txt",
	FILE_TEST_CORPUS_PATH "\\en\\4.txt",
    NULL
};

const char * testpath = FILE_TEST_CORPUS_PATH "\\en\\longline.txt" ;

#define SEARCH_TERM L"JDFFH"

const wchar_t * TermsSearchParameters[5] = {
	L"a*",
	L"e*",
	L"h*",
	L"w*",
    NULL
};


void TestAppclassFilteredTermSearch(Itk::TestMgr * testMgr, const wchar_t* appclassPrefix)
{
	cpix_Result
		result;
	
	int32_t hits_len=0;

	cpix_IdxDb_dbgScrapAll(&result);

	ITK_ASSERT(testMgr,
			   cpix_Succeeded(&result),
			   "Could not get rid of all test qbac-idx pairs");
	if(!cpix_Succeeded(&result))
	    {
	
	    }
	std::auto_ptr<FileIdxUtil> util( new FileIdxUtil ); 
	
	util->init(); 
	
	cpix_Analyzer* analyzer = cpix_CreateSimpleAnalyzer(&result); 
	
	if ( cpix_Failed( &result) ) ITK_PANIC("Analyzer could not be created");
	   if(!cpix_Succeeded(&result))
	        {
	    
	        }
	
	for (int i = 0; TermTestDocsToIndex[i]; i++) 
	{
		util->indexFile( TermTestDocsToIndex[i], analyzer, testMgr ); 
	}

	for (int i = 0; Mp3TestCorpus[i]; i++) 
	{
		util->indexFile( Mp3TestCorpus[i], analyzer, testMgr ); 
	}

        util->flush();
	
	for (int i = 0; TermsSearchParameters[i]; i++) 
	{
		cpix_QueryParser
			* queryParser = cpix_QueryParser_create(&result,
													LCPIX_DEFAULT_FIELD,
													analyzer );
		if (queryParser == NULL)
			{
				cpix_Analyzer_destroy( analyzer );
			    
			    ITK_PANIC("Could not create query parser");
			}
		
		std::wostringstream queryString;
		if ( appclassPrefix ) {
			queryString<<L"$terms<5,'"<<appclassPrefix<<L"'>("<<TermsSearchParameters[i]<<L")";
		} else {
			queryString<<L"$terms<5>("<<TermsSearchParameters[i]<<L")";
		}
	
		cpix_Query* query = cpix_QueryParser_parse(queryParser,
												   queryString.str().c_str());
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
		
		wprintf(L"Results for %S:\n", TermsSearchParameters[i]);
		
		Suggestion::printSuggestions(hits,
                                     testMgr);
        
		printf("\n"); 
				
		cpix_Hits_destroy( hits ); 
	}
	
	//
	util->indexFile( testpath, analyzer, testMgr );
	
	util->flush();
	
	cpix_QueryParser *queryParser_ = cpix_QueryParser_create(&result,
                                     LCPIX_DEFAULT_FIELD,
                                     analyzer );
	    
    if (queryParser_ == NULL)
        {
        
        ITK_PANIC("Could not create query parser");
        }                

    cpix_Query* query = cpix_QueryParser_parse(queryParser_, SEARCH_TERM);

    if (cpix_Failed(queryParser_))
        {
        ITK_PANIC("Could not create query parser");
        }
    
    if (cpix_Failed(queryParser_)
                || query == NULL)
        {
            
            cpix_Analyzer_destroy(analyzer);
            cpix_ClearError(queryParser_);
            cpix_QueryParser_destroy(queryParser_);
            ITK_PANIC("Could not parse query string");
        }
    
    cpix_QueryParser_destroy(queryParser_);
    
    cpix_Hits
        * hits = cpix_IdxDb_search(util->idxDb(),
                                   query );
    if (cpix_Succeeded(hits))
            {
            hits_len = cpix_Hits_length(hits);
            cpix_Hits_destroy( hits );
            }
    if(hits_len >= 1)
        {
        ITK_MSG(testMgr, "Search Term %S found in longline text",SEARCH_TERM);
        }
    else
        ITK_MSG(testMgr, "Search Term %S not Indexed",SEARCH_TERM);
    
    cpix_Query_destroy( query );
	
	cpix_Analyzer_destroy( analyzer ); 
}

void TestAllTermSearch(Itk::TestMgr * testMgr) 
{    
	TestAppclassFilteredTermSearch(testMgr, 0);

}

void TestMp3TermSearch(Itk::TestMgr * testMgr) 
{
	TestAppclassFilteredTermSearch(testMgr, LMP3APPCLASS);

}

void TestTextTermSearch(Itk::TestMgr * testMgr) 
{
	TestAppclassFilteredTermSearch(testMgr, LTEXTAPPCLASS);

}

Itk::TesterBase * CreateTermSearchTests()
{
    using namespace Itk;

    SuiteTester
        * whiteBox = new SuiteTester("terms"); // default context

    whiteBox->add("allterms",
                  &TestAllTermSearch,
                  "allterms");

    whiteBox->add("mp3terms",
                  &TestMp3TermSearch,
                  "mp3terms");
    
    whiteBox->add("textterms",
                  &TestTextTermSearch,
                  "textterms");

    // TODO add more

    return whiteBox;
}
