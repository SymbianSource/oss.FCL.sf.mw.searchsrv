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


const char * PdfDocsToIndex[8] = {
    "c:\\data\\cpixunittestcorpus\\pdf\\ctutor.pdf",
	"c:\\data\\cpixunittestcorpus\\pdf\\geology.pdf",
	"c:\\data\\cpixunittestcorpus\\pdf\\samplepdf.pdf",
	"c:\\data\\cpixunittestcorpus\\pdf\\windjack.pdf",
	"c:\\data\\cpixunittestcorpus\\pdf\\DCTDecode.pdf",
	"c:\\data\\cpixunittestcorpus\\pdf\\Empty.pdf",
	"c:\\data\\cpixunittestcorpus\\pdf\\start_enter.pdf",
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
                
            }
        

        cpix_Query* query = cpix_QueryParser_parse(queryParser,PdfSearchParameters[i]);
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
        
        int32_t hitsLength = cpix_Hits_length(hits);
                 
        wprintf(L"Results for %S, and the hit len is : %d\n", PdfSearchParameters[i], hitsLength);
        
        
        cpix_Hits_destroy( hits );
        cpix_Query_destroy( query );
        cpix_QueryParser_destroy(queryParser);
        
    }
    cpix_Analyzer_destroy(analyzer);

}
// int32_t hitsLength = cpix_Hits_length(hits);
void CreateSimplePdfSearch(Itk::TestMgr * testMgr) 
{
    
    pdfTestAppclassFilteredTermSearch(testMgr, LPDFAPPCLASS);
   
}


Itk::TesterBase * CreatePdfSearchTests()
{
    using namespace Itk;

    SuiteTester
        * pdfTests = new SuiteTester("pdfTests");

    pdfTests->add("pdfterms", &CreateSimplePdfSearch, "pdfterms");
        
    return pdfTests;
}
