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
#include "testutils.h"
#include "testcorpus.h"
#include "config.h"
#include "itk.h"
#include "setupsentry.h"

#include <iostream>

#include "cpixsearch.h"


namespace
{
    const MVFTest FilesAndVols[] = {
        {
            FILE_TEST_CORPUS_PATH "\\en\\1.txt",
            "@c:root file",
            CPIX_TEST_INDEVICE_INDEXDB_PHMEM CPIX_FILE_IDXDB "_\\c",
        },

        {
            FILE_TEST_CORPUS_PATH "\\en\\2.txt",
            "@d:root file",
            CPIX_TEST_INDEVICE_INDEXDB_PHMEM CPIX_FILE_IDXDB "_\\d",
        },

        {
            FILE_TEST_CORPUS_PATH "\\en\\3.txt",
            "@e:root file",
            CPIX_TEST_INDEVICE_INDEXDB_PHMEM CPIX_FILE_IDXDB "_\\e",
        },

        {
            FILE_TEST_CORPUS_PATH "\\en\\4.txt",
            "@f:root file",
            CPIX_TEST_INDEVICE_INDEXDB_PHMEM CPIX_FILE_IDXDB "_\\f",
        },

        {
            NULL,
            NULL,
            NULL
        },
    };
}


class PrematureQryContext : public Itk::ITestContext
{
private:
    cpix_IdxDb         * idxDb_;
    cpix_IdxSearcher   * idxSearcher_;

    cpix_Analyzer      * analyzer_;
    cpix_QueryParser   * queryParser_;
    cpix_Query         * query_;


public:

    //
    // from interface Itk::ITestContext
    //
    virtual void setup() throw (Itk::PanicExc)
    {
        SetupSentry
            ss(*this);

        cpix_Result
            result;

        cpix_IdxDb_dbgScrapAll(&result);
        if (cpix_Failed(&result))
            {
                ITK_PANIC("Could not dbg scrapp all indexes");
            }

        analyzer_ = cpix_CreateSimpleAnalyzer(&result);
        if (cpix_Failed(&result))
            {
                ITK_PANIC("Could not create analyzer instance");
            }

        queryParser_ = cpix_QueryParser_create(&result,
                                               LCPIX_DEFAULT_FIELD,
                                               analyzer_);
        if (cpix_Failed(&result))
            {
                ITK_PANIC("Could not create query parser");
            }

        query_ = cpix_QueryParser_parse(queryParser_,
                                        L"ha*");
        if (cpix_Failed(queryParser_))
            {
                ITK_PANIC("Could not create query");
            }

        for (const MVFTest * test = FilesAndVols;
             test->qualifiedBaseAppClass_ != NULL;
             ++test)
            {
                bool
                    isThere = Cpt::directoryexists(test->idxDbPath_);

                if (isThere)
                    {
                        int
                            result = Cpt::removeall(test->idxDbPath_);
                        
                        if (result != 0)
                            {
                                ITK_PANIC("Could not purge index path %s",
                                          test->idxDbPath_);
                            }
                    }
            }

        ss.setupComplete();
    }    


    virtual void tearDown() throw()
    {
        cleanup();
    }


    //
    // lifetime mgmt
    //
    virtual ~PrematureQryContext()
    {
        cleanup();
    }
    

    
    PrematureQryContext()
        : idxDb_(NULL),
          idxSearcher_(NULL),
          analyzer_(NULL),
          queryParser_(NULL),
          query_(NULL)
    {
        ;
    }


    //
    // test functions
    //
    void testBeforeVolDefs(Itk::TestMgr * mgr)
    {
        printf("Before volume definitions\n");

        qry(mgr,
            false);
    }


    void testBeforeContentDefs(Itk::TestMgr * mgr)
    {
        printf("Defining volumes\n");

        cpix_Result
            result;

        for (const MVFTest * test = FilesAndVols;
             test->qualifiedBaseAppClass_ != NULL;
             ++test)
            {
                cpix_IdxDb_defineVolume(&result,
                                        test->qualifiedBaseAppClass_,
                                        test->idxDbPath_);
                ITK_ASSERT(mgr,
                           cpix_Succeeded(&result),
                           "Failed to define volume %s/%s",
                           test->qualifiedBaseAppClass_,
                           test->idxDbPath_);
            }

        printf("Defined volumes, but there are no indexes created yet\n");

        qry(mgr,
            true);
    }


    void testReady(Itk::TestMgr * mgr)
    {
        printf("Defining contents\n");

        const MVFTest
            * mvfTest = FilesAndVols;

        for (; mvfTest->qualifiedBaseAppClass_ != NULL; ++mvfTest)
            {
                using namespace std;

                printf("Creating volume %s ...\n",
                       mvfTest->qualifiedBaseAppClass_);

                {
                    auto_ptr<VolumeFileIdxUtil>
                        vfiUtil(new VolumeFileIdxUtil(mvfTest));
                    vfiUtil->init();

                    vfiUtil->indexFile(mvfTest->textFilePath_,
                                       analyzer_,
                                       mgr);
                    vfiUtil->flush();
                }

                printf("... created volume %s\n",
                       mvfTest->qualifiedBaseAppClass_);
            }

        qry(mgr,
            true);
    }


private:
    void cleanup()
    {
        cpix_IdxDb_releaseDb(idxDb_);
        idxDb_ = NULL;

        cpix_IdxSearcher_releaseDb(idxSearcher_);
        idxSearcher_ = NULL;

        cpix_Query_destroy(query_);
        query_ = NULL;

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;

        cpix_Analyzer_destroy(analyzer_);
        analyzer_ = NULL;
    }


    void qryIdx(Itk::TestMgr * mgr,
                bool           shouldSucceed)
    {
        printf("Trying to search with cpix_IdxDb instance\n");

        cpix_Result
            result;
        bool
            succeeded;

        if (idxDb_ == NULL)
            {
                idxDb_ = cpix_IdxDb_openDb(&result,
                                           "@c:root file",
                                           cpix_IDX_OPEN);
                                           
                succeeded = cpix_Succeeded(&result);

                if (!succeeded)
                    {
                        goto exit;
                    }
            }

        cpix_Hits
            * hits = cpix_IdxDb_search(idxDb_,
                                       query_);

        succeeded = cpix_Succeeded(idxDb_);
        
        if (!succeeded)
            {
                goto exit;
            }

        PrintHits(hits,
                  mgr);

        cpix_Hits_destroy(hits);

    exit:
        ITK_EXPECT(mgr,
                   (shouldSucceed && succeeded) 
                   || (!shouldSucceed && !succeeded),
                   "Querying idx should %s have succeeded and it did %s",
                   shouldSucceed ? "" : "not",
                   succeeded ? "" : "not");
    }


    void qrySearcher(Itk::TestMgr * mgr,
                     bool           shouldSucceed)
    {
        printf("Trying to search with cpix_IdxSearcher instance\n");

        cpix_Result
            result;
        bool
            succeeded;

        if (idxSearcher_ == NULL)
            {
                idxSearcher_ = cpix_IdxSearcher_openDb(&result,
                                                       "root");
                                           
                succeeded = cpix_Succeeded(&result);

                if (!succeeded)
                    {
                        goto exit;
                    }
            }

        cpix_Hits
            * hits = cpix_IdxSearcher_search(idxSearcher_,
                                             query_);

        succeeded = cpix_Succeeded(idxSearcher_);
        
        if (!succeeded)
            {
                goto exit;
            }

        PrintHits(hits,
                  mgr);

        cpix_Hits_destroy(hits);

    exit:
        ITK_EXPECT(mgr,
                   (shouldSucceed && succeeded) 
                   || (!shouldSucceed && !succeeded),
                   "Querying searcher should %s have succeeded and it did %s",
                   shouldSucceed ? "" : "not",
                   succeeded ? "" : "not");
    }



    void qry(Itk::TestMgr * mgr,
             bool           shouldSucceed)
    {
        qryIdx(mgr,
               shouldSucceed);
        qrySearcher(mgr,
                    shouldSucceed);
    }



};



Itk::TesterBase * CreatePrematureQryTests()
{
    using namespace Itk;

    PrematureQryContext
        * pqc = new PrematureQryContext();
    ContextTester
        * contextTester = new ContextTester("prematureqry",
                                            pqc);

#define TEST "beforeVolDefs"
    contextTester->add(TEST,
                       pqc,
                       &PrematureQryContext::testBeforeVolDefs,
                       TEST);
#undef TEST

#define TEST "beforeContentDefs"
    contextTester->add(TEST,
                       pqc,
                       &PrematureQryContext::testBeforeContentDefs,
                       TEST);
#undef TEST
    
#define TEST "ready"
    contextTester->add(TEST,
                       pqc,
                       &PrematureQryContext::testReady,
                       TEST);
#undef TEST

    return contextTester;
}
