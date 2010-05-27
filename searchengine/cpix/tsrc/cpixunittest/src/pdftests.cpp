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
 *  Created on: 3.3.2010
 *      Author: anide
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

const char * PdfDocsToIndex[7] = {
    "c:\\data\\cpixunittestcorpus\\pdf\\ctutor.pdf",
	"c:\\data\\cpixunittestcorpus\\stem\\pdf\\geology.pdf",
	"c:\\data\\cpixunittestcorpus\\stem\\pdf\\samplepdf.pdf",
	"c:\\data\\cpixunittestcorpus\\stem\\pdf\\windjack.pdf",
	"c:\\data\\cpixunittestcorpus\\stem\\pdf\\DCTDecode.pdf",
	"c:\\data\\cpixunittestcorpus\\stem\\pdf\\Empty.pdf",
    NULL
};


const wchar_t * PdfSearchParameters[5] = {
	L"inline",
	L"CALDEBA",
	L"sample",
	L"reset",
    NULL
};

void pdfTestAppclassFilteredTermSearch(Itk::TestMgr * testMgr, const wchar_t* appclassPrefix)
{
    cpix_Result
        result;

    cpix_IdxDb_dbgScrapAll(&result);

    ITK_ASSERT(testMgr,
               cpix_Succeeded(&result),
               "Could not get rid of all test qbac-idx pairs");

    std::auto_ptr<FileIdxUtil> util( new FileIdxUtil ); 
    
    util->init(TRUE); 
    
    cpix_Analyzer* analyzer = cpix_CreateSimpleAnalyzer(&result); 
    
    if ( cpix_Failed( &result) ) 
        {
            ITK_PANIC("Analyzer could not be created");
            assert_failed = 1;
        }
    
    for (int i = 0; PdfDocsToIndex[i]; i++) 
    {
        util->indexFile( PdfDocsToIndex[i], analyzer, testMgr ); 
    }

    util->flush();
    
    for (int i = 0; PdfSearchParameters[i]; i++) 
    {
        cpix_QueryParser
            * queryParser = cpix_QueryParser_create(&result,
                                                    LCPIX_DEFAULT_FIELD,
                                                    analyzer );
        if (queryParser == NULL)
            {
                cpix_Analyzer_destroy( analyzer );
                ITK_PANIC("Could not create query parser");
                assert_failed = 1;
            }
        
        std::wostringstream queryString;
        queryString<<L"adobe";
        cpix_Query* query = cpix_QueryParser_parse(queryParser,
                                                   queryString.str().c_str());
        if (cpix_Failed(queryParser)
            || query == NULL)
            {
                cpix_Analyzer_destroy(analyzer);
                cpix_ClearError(queryParser);
                cpix_QueryParser_destroy(queryParser);
                ITK_PANIC("Could not parse query string");
                assert_failed = 1;
            }
       
        cpix_Hits
            * hits = cpix_IdxDb_search(util->idxDb(),
                                       query );
        
        int32_t hitsLength = cpix_Hits_length(hits);
                 
        wprintf(L"Results for %S:\n", PdfSearchParameters[i]);
        
        Suggestion::printSuggestions(hits,
                                     testMgr);        
        printf("\n"); 
        
        cpix_Analyzer_destroy(analyzer);
        cpix_Hits_destroy( hits );
        cpix_Query_destroy( query );
        cpix_QueryParser_destroy(queryParser);
        
    }

}
// int32_t hitsLength = cpix_Hits_length(hits);
void CreateSimplePdfSearch(Itk::TestMgr * testMgr) 
{
    char *xml_file = (char*)__FUNCTION__;
    assert_failed = 0;
    pdfTestAppclassFilteredTermSearch(testMgr, LPDFAPPCLASS);
    testResultXml(xml_file);
}


Itk::TesterBase * CreatePdfSearchTests()
{
    using namespace Itk;

    SuiteTester
        * pdfTests = new SuiteTester("pdfTests");

    pdfTests->add("pdfterms", &CreateSimplePdfSearch, "pdfterms");
        
    return pdfTests;
}
