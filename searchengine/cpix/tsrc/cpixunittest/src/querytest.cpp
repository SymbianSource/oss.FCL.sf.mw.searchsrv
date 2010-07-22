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
#include <sstream>
#include "indevicecfg.h"

#include "cpixidxdb.h"

#include "itk.h"
//#include "xmllog.h"
#include "config.h"
#include "testutils.h"
#include "suggestion.h"

#include "std_log_result.h"

const char * docsToIndex[5] = {
CORPUS_PATH "\\query\\query1.txt",
CORPUS_PATH "\\query\\query2.txt",
CORPUS_PATH "\\query\\query3.txt",
CORPUS_PATH "\\query\\query4.txt",
NULL
};

void setupPlainQuery(Itk::TestMgr * testMgr)
    {
    cpix_Result
            result;

        cpix_IdxDb_dbgScrapAll(&result);
        ITK_ASSERT(testMgr,
                       cpix_Succeeded(&result),
                       "Could not get rid of all test qbac-idx pairs");
        std::auto_ptr<FileIdxUtil> util( new FileIdxUtil ); 
        util->init(true); 
        cpix_Analyzer* analyzer = cpix_Analyzer_create(&result, L"standard"); 
        if ( cpix_Failed( &result) ) ITK_PANIC("Analyzer could not be created");
        for (int i = 0; docsToIndex[i]; i++) 
            {
        util->indexFile( docsToIndex[i], analyzer, testMgr );
            }
        util->flush();
        cpix_Analyzer_destroy(analyzer);
    }

void setupPrefixQuery(Itk::TestMgr * testMgr)
    {
    cpix_Result
            result;

        cpix_IdxDb_dbgScrapAll(&result);
        ITK_ASSERT(testMgr,
                       cpix_Succeeded(&result),
                       "Could not get rid of all test qbac-idx pairs");
        std::auto_ptr<FileIdxUtil> util( new FileIdxUtil ); 
        util->init(true); 
        cpix_Analyzer* analyzer = cpix_Analyzer_create(&result, L"standard"); 
        if ( cpix_Failed( &result) ) ITK_PANIC("Analyzer could not be created");
       
        util->indexFile( CORPUS_PATH "\\query\\query5.txt", analyzer, testMgr );
       
        util->flush();
        util->indexFile( CORPUS_PATH "\\query\\query6.txt", analyzer, testMgr );
               
        util->flush();
        cpix_Analyzer_destroy(analyzer);
    }

void setupPrefixOptimiseQuery(Itk::TestMgr * testMgr)
    {
    cpix_Result
            result;

        cpix_IdxDb_dbgScrapAll(&result);
        ITK_ASSERT(testMgr,
                       cpix_Succeeded(&result),
                       "Could not get rid of all test qbac-idx pairs");
        std::auto_ptr<FileIdxUtil> util( new FileIdxUtil ); 
        util->init(true); 
        cpix_Analyzer* analyzer = cpix_Analyzer_create(&result, L"standard"); 
        if ( cpix_Failed( &result) ) ITK_PANIC("Analyzer could not be created");
       
        util->indexFile( CORPUS_PATH "\\query\\query7.txt", analyzer, testMgr );
               
        util->flush();
        util->indexFile( CORPUS_PATH "\\query\\query8.txt", analyzer, testMgr );
               
        util->flush();
        util->indexFile( CORPUS_PATH "\\query\\query9.txt", analyzer, testMgr );
               
        util->flush();
        cpix_Analyzer_destroy(analyzer);
    
    }

void testQuery(Itk::TestMgr * testMgr, const wchar_t *qryStr, int hitLen, Efield_type ftype = LCPIX_DEFAULT)
{
cpix_Result
        result;
int32_t hitsLength  = 0;
   
    std::auto_ptr<FileIdxUtil> util( new FileIdxUtil ); 
    
    util->init(false); 
    cpix_Analyzer* analyzer = cpix_Analyzer_create(&result, L"standard"); 
    if ( cpix_Failed( &result) ) ITK_PANIC("Analyzer could not be created");
        cpix_QueryParser
            * queryParser = NULL;
        switch(ftype)
        {
            case LCPIX_DEFAULT_PREFIX:
                queryParser = cpix_QueryParser_create(&result,LCPIX_DEFAULT_PREFIX_FIELD,analyzer );
                break;
            case LCPIX_DEFAULT:
                queryParser = cpix_QueryParser_create(&result,LCPIX_DEFAULT_FIELD,analyzer );
                break;
        }
        if (queryParser == NULL)
            {
                assert_failed = 1;
                cpix_Analyzer_destroy( analyzer );
                ITK_PANIC("Could not create query parser");
            }
        

    
        cpix_Query* query = cpix_QueryParser_parse(queryParser,
                                                   qryStr);
        if (cpix_Failed(queryParser)
            || query == NULL)
            {
                assert_failed = 1;
                cpix_Analyzer_destroy(analyzer);
                cpix_ClearError(queryParser);
                cpix_QueryParser_destroy(queryParser);
                ITK_PANIC("Could not parse query string");
            }
        cpix_QueryParser_destroy(queryParser);

        cpix_Hits
            * hits = cpix_IdxDb_search(util->idxDb(),
                                       query );
        
        if (cpix_Succeeded(hits))
                {
                hitsLength = cpix_Hits_length(hits);
                cpix_Hits_destroy( hits );
                }
                if(hitsLength == hitLen )
                    {
                   
                    ITK_MSG(testMgr, "Query %S, returned %d hits. Passed \n",qryStr,hitsLength );
                    }
                else
                    {
                    assert_failed = 1;
                    ITK_MSG(testMgr, "Query %S, didnt return expected hits. Expected is %d hits. Returned is %d. Failed \n",qryStr,hitLen,hitsLength);
                    }
                    
        cpix_Query_destroy( query ); 
        
        cpix_Analyzer_destroy(analyzer);
}



void CreatePlainQueryTest(Itk::TestMgr * testMgr) 
{
    char *xml_file = (char*)__FUNCTION__;
    assert_failed = 0;
    setupPlainQuery(testMgr);
    testQuery(testMgr,L"Nokia", 2);
    testQuery(testMgr,L"iNdia", 1);
    testQuery(testMgr,L"\"London Finland\"", 1);
    testQuery(testMgr,L"Contents:Nokia", 2);
    testQuery(testMgr,L"Contents:Nokia AND country", 1);
    testQuery(testMgr,L"Contents:Nokia && country", 1);
    testQuery(testMgr,L"Nokia Selvaraj", 3);
    testQuery(testMgr,L"Lo?don", 1);
    testQuery(testMgr,L"countr?", 1);
    testQuery(testMgr,L"country?", 0);
    testQuery(testMgr,L"?india", 0);
    testQuery(testMgr,L"nok*", 2);
    testQuery(testMgr,L"count?ry", 0);
    testQuery(testMgr,L"roam~", 2);
    testQuery(testMgr,L"ro~am", 0);
    testQuery(testMgr,L"\"london country\"~10", 1);
    testQuery(testMgr,L"\"nokia country\"~2", 0);
    testQuery(testMgr,L"nokia basker", 3);
    testQuery(testMgr,L"Nokia^5 basker", 3);
    testQuery(testMgr,L"Nokia basker^5", 3);
    testQuery(testMgr,L"Nokia || basker", 3);
    testQuery(testMgr,L"Nokia OR basker", 3);
    testQuery(testMgr,L"Nokia AND basker", 0);
    testQuery(testMgr,L"Nokia && basker", 0);
    testQuery(testMgr,L"+nokia country", 2);
    testQuery(testMgr,L"+nokia roam", 2);
    testQuery(testMgr,L"Nokia !country", 1);
    testQuery(testMgr,L"nokia NOT country", 1);
    testQuery(testMgr,L"nokia NOT basker", 2);
    testQuery(testMgr,L"NOT India", 1);
    testQuery(testMgr,L"(india OR Mobile) AND Nokia", 2);
    testQuery(testMgr,L"(india OR Mobile) AND Country", 1);
    testResultXml(xml_file);
}

void CreatePrefixQueryTest(Itk::TestMgr * testMgr) 
{
    char *xml_file = (char*)__FUNCTION__;
    assert_failed = 0;
    setupPrefixQuery(testMgr);
    testQuery(testMgr,L"$prefix(\"new-notes\")", 1);
    testQuery(testMgr,L"$prefix(\"notes\")", 1);
    testQuery(testMgr,L"$prefix(\"new\")", 1);
    testQuery(testMgr,L"$prefix(\"-india\")", 1);
    testQuery(testMgr,L"$prefix(\"tamil-nadu\")", 1);
    testQuery(testMgr,L"$prefix(\"testing\")", 2);
    testQuery(testMgr,L"$prefix(\"*shankar\")", 2);
    testQuery(testMgr,L"$prefix(\"Ani*rban\")", 1);
    testQuery(testMgr,L"$prefix(\"kumar*\")", 1);
    testQuery(testMgr,L"$prefix(\"carrot^\")", 1);
    testQuery(testMgr,L"$prefix(\"carrot\")", 1);
    testQuery(testMgr,L"$prefix(\"ani\")", 1);
    testQuery(testMgr,L"$prefix(\"question\")", 1);
    testQuery(testMgr,L"$prefix(\"question?\")", 1);
    testQuery(testMgr,L"$prefix(\"|pipe\")", 1);
    testQuery(testMgr,L"$prefix(\"&&ambersend\")", 1);
    testQuery(testMgr,L"$prefix(\"!=Exclamation\")", 1);
    testQuery(testMgr,L"$prefix(\":colon\")", 1);
    testQuery(testMgr,L"$prefix(\"http:\\www.nokia.com\")", 1);
    testQuery(testMgr,L"$prefix(\"%percentage\")", 1);
    testQuery(testMgr,L"$prefix(\"(testing)\")", 2);
    testQuery(testMgr,L"$prefix(\"mail-id\")", 1);
    testQuery(testMgr,L"$prefix(\"mail id\")", 1);
    testQuery(testMgr,L"$prefix(\"shankar.rajendran@nokia.com\")", 1);
    testQuery(testMgr,L"$prefix(\"~tild\")", 1);
    testQuery(testMgr,L"$prefix(\"shankar\")", 2);
    testQuery(testMgr,L"$prefix(\"`singlequote\")", 1);
    testQuery(testMgr,L"$prefix(\"singlequote\")", 1);
    testQuery(testMgr,L"$prefix(\"\"doublequote\")", 1);
    testQuery(testMgr,L"$prefix(\"doublequote\")", 1);
    testQuery(testMgr,L"$prefix(\";semicolon\")", 1);
    testQuery(testMgr,L"$prefix(\"/slash\")", 1);
    testQuery(testMgr,L"$prefix(\"slash\")", 1);
    testQuery(testMgr,L"$prefix(\"\\backslash\")", 1);
    testQuery(testMgr,L"$prefix(\"backslash\")", 1);
    testQuery(testMgr,L"$prefix(\"[squarebracket]\")", 1);
    testQuery(testMgr,L"$prefix(\"{flowerbracket}\")", 1);
    testQuery(testMgr,L"$prefix(\"<lessthan\")", 1);
    testQuery(testMgr,L"$prefix(\">greaterthan\")", 1);
    testQuery(testMgr,L"$prefix(\"worked for motorola .\")", 1);
    testResultXml(xml_file);
}

void CreatePrefixOptimiseQueryTest(Itk::TestMgr * testMgr)
    {
    char *xml_file = (char*)__FUNCTION__;
    assert_failed = 0;
    setupPrefixOptimiseQuery(testMgr);
    testQuery(testMgr,L"i*", 3,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"in*", 2,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"i?", 3,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"id*", 2,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"c*", 2,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"c?", 2,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"cu*", 2,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"co*", 2,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"d*", 3,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"d?", 3,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"de*", 2,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"do*", 1,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"l*", 3,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"lo*", 2,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"li*", 1,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"wo*", 1,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"pr*", 1,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"r*", 3,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"ru*", 2,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"ra*", 2,LCPIX_DEFAULT_PREFIX );
    testQuery(testMgr,L"ri*", 2,LCPIX_DEFAULT_PREFIX );
    testResultXml(xml_file);
    }
Itk::TesterBase * CreateQueryTests()
{
    using namespace Itk;

    SuiteTester
        * qryTests = new SuiteTester("querytests");

    qryTests->add("plainquerytest", &CreatePlainQueryTest, "plainquerytest");
    qryTests->add("prefixquerytest", &CreatePrefixQueryTest, "prefixquerytest");
    qryTests->add("prefixoptimisequerytest", &CreatePrefixOptimiseQueryTest, "prefixoptimisequerytest");
    
    return qryTests;
}


