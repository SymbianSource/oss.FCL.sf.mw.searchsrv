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
#include <set>

#include "cpixtools.h"

#include "itk.h"

#include "cpixidxdb.h"
#include "cpixsearch.h"

#include "config.h"
#include "testutils.h"
#include "setupsentry.h"

#include "std_log_result.h"

const Volume Volumes[] = {
    { "@fin:root maps city",
      MAPS_TEST_CORPUS_PATH "\\root\\maps\\city\\fin"
    },

    { "@0:root maps country",
      MAPS_TEST_CORPUS_PATH "\\root\\maps\\country"
    },

    { "@fin:root maps street",
      MAPS_TEST_CORPUS_PATH "\\root\\maps\\street\\fin"
    },

    { NULL,
      NULL
    },
};

#define VATTUNIEMENRANTA_2_GPS L"60.154023,24.887724"

const wchar_t * MapsSearchTerms[] = {

    // country: FIN
    L"soome",
    L"finsko",
    L"suomi",

    // country: HUN
    L"unkari",
    L"hungary",

    // cities
    L"espoo",
    L"kirkkonummi",
    L"lammi",
    L"lahti",
    L"oulu",
    L"tampere",

    // streets
    L"lehdontie",
    L"sentinoja",
    L"karvosentie",
    L"kivitie",
    L"ludvig filipintie",

    // specific ordered results
    L"Revontulentie", // 1st result "Revontulentie,  Kuusamo, Distance 677.39km", 
    // 2nd result "Revontulentie, Salla, Distance 756.83km", (more results follow)
    L"Revontulentie Salla", // 1st result "Revontulentie, Salla, Distance 756.83km"

    NULL
};


class MapsContext : public Itk::ITestContext
{
private:
    //
    // private members
    //

    cpix_Analyzer    * analyzer_;
    cpix_QueryParser * queryParser_;
    cpix_IdxSearcher * idxSearcher_;


public:
    
    //
    // from ITestContext
    //
    virtual void setup() throw (Itk::PanicExc)
    {
        cpix_Result
            result;

        SetupSentry
            ss(*this);

        analyzer_ = cpix_CreateSimpleAnalyzer(&result);
        if (analyzer_ == NULL)
            {
                ITK_PANIC("Could not create analyzer");
            }

        queryParser_ = cpix_QueryParser_create(&result,
                                               LCPIX_DEFAULT_FIELD,
                                               analyzer_);
        if (queryParser_ == NULL)
            {
                ITK_PANIC("Could not create query parser");
            }

        ss.setupComplete();
    }

    
    virtual void tearDown() throw()
    {
        cleanup();
    }

    
    MapsContext()
        : analyzer_(NULL),
          queryParser_(NULL),
          idxSearcher_(NULL)
    {
        ;
    }


    ~MapsContext()
    {
        cleanup();
    }


    static void printHit(cpix_Document  * doc,
                         Itk::TestMgr   * testMgr)
    {
        using namespace std;
        
        fprintf(stdout,
                "NEXT ADDRESS:\n");
        
        cpix_DocFieldEnum
            * dfe = cpix_Document_fields(doc);
        if (cpix_Succeeded(doc))
            {
                cpix_Field
                    field;

                while (cpix_DocFieldEnum_hasMore(dfe))
                    {
                        cpix_DocFieldEnum_next(dfe,
                                               &field);

                        const wchar_t
                            * name = cpix_Field_name(&field);

                        fprintf(stdout,
                                " o %S: ",
                                name);

                        bool
                            binary = static_cast<bool>(cpix_Field_isBinary(&field));

                        fprintf(stdout,
                                "%S\n",
                                (binary ? 
                                 L"(binary)" 
                                 : cpix_Field_stringValue(&field)
                                 )
                                );
                    }

                cpix_DocFieldEnum_destroy(dfe);
            }
        else
            {
                wchar_t
                    report[128];
                cpix_Error_report(doc->err_,
                                  report,
                                  sizeof(report)/sizeof(wchar_t));
                ITK_EXPECT(testMgr,
                           false,
                           "Could not create doc field enum: %S",
                           report);
                cpix_ClearError(doc);
            }
    }


    void testLoadMaps(Itk::TestMgr * testMgr)
    {
        cpix_Result
            result;

        cpix_IdxDb_dbgScrapAll(&result);

        ITK_ASSERT(testMgr,
                   cpix_Succeeded(&result),
                   "Could not reset qbac-path registry");
       

        const Volume
            *p = Volumes;

        for (; p->qbac_ != NULL; ++p)
            {
                cpix_IdxDb_defineVolume(&result,
                                        p->qbac_,
                                        p->path_);

                ITK_ASSERT(testMgr,
                           cpix_Succeeded(&result),
                           "Could not define volume '%s'",
                           p->qbac_);
            }

        idxSearcher_ = cpix_IdxSearcher_openDb(&result,
                                               "root maps");

        ITK_ASSERT(testMgr,
                   cpix_Succeeded(&result),
                   "Could not create searcher for 'root maps'");
       
       
    }

    void testSearchMaps(Itk::TestMgr * testMgr)
    {
        printf("Testing searching maps index\n");
        const wchar_t
            ** p = MapsSearchTerms;
        for (; *p != NULL; ++p)
            {
                printf("\n\nSearching for '%S':\n",
                       *p);

                std::wstring
                    addressQryStr(L"$address<" VATTUNIEMENRANTA_2_GPS L">(");
                addressQryStr += *p;
                addressQryStr += L")";
                
                cpix_Query
                    * query = cpix_QueryParser_parse(queryParser_,
                                                     addressQryStr.c_str());
                ITK_EXPECT(testMgr,
                           cpix_Succeeded(queryParser_),
                           "Could not query parse %S",
                           *p);

                if (cpix_Failed(queryParser_))
                    {
                        continue;
                    }

                cpix_Hits
                    * hits = cpix_IdxSearcher_search(idxSearcher_,
                                                     query);

                if (cpix_Succeeded(idxSearcher_))
                    {
                        CustomPrintHits(hits,
                                        testMgr,
                                        &printHit);

                        cpix_Hits_destroy(hits);
                    }
                else
                    {
                        ITK_EXPECT(testMgr,
                                   false,
                                   "Failed to do address search");
                        
                    }

                cpix_Query_destroy(query);
            }

    }


private:
    void cleanup()
    {
        cpix_Analyzer_destroy(analyzer_);
        analyzer_ = NULL;

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;

        cpix_IdxSearcher_releaseDb(idxSearcher_);
        idxSearcher_ = NULL;
    }

};



Itk::TesterBase * CreateMapsTests()
{
    using namespace Itk;

    MapsContext
        * mapsContext = new MapsContext;
    ContextTester
        * contextTester = new ContextTester("maps",
                                            mapsContext);

#define TEST "loadMaps"
    contextTester->add(TEST,
                       mapsContext,
                       &MapsContext::testLoadMaps);
#undef TEST


#define TEST "searchMaps"
    contextTester->add(TEST,
                       mapsContext,
                       &MapsContext::testSearchMaps,
                       TEST);
#undef TEST 


    // TODO add more tests to suite
        
    return contextTester;
}
