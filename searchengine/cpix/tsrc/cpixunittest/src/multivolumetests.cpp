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

#include "cpixfstools.h"

#include "itk.h"

#include "cpixidxdb.h"

#include "config.h"
#include "testutils.h"

#include "testcorpus.h"
#include "setupsentry.h"

#include "std_log_result.h"

class PlainMVContext : public Itk::ITestContext
{
    cpix_Analyzer    * analyzer_;
    cpix_QueryParser * queryParser_;

    const MVFTest    * mvfTests_;

public:

    PlainMVContext(const MVFTest * mvfTests)
        : analyzer_(NULL),
          queryParser_(NULL),
          mvfTests_(mvfTests)
    {
        ;
    }


    virtual void setup() throw (Itk::PanicExc)
    {
        SetupSentry
            ss(*this);

        cpix_Result
            result;

        cpix_IdxDb_dbgScrapAll(&result);

        analyzer_ = cpix_CreateSimpleAnalyzer(&result);
        if (analyzer_ == NULL)
            {
                ITK_PANIC("Could not create analyzer");
            }

        
        queryParser_ = cpix_QueryParser_create(&result,
                                               CONTENTS_FIELD,
                                               analyzer_);
        if (queryParser_ == NULL)
            {
                ITK_PANIC("Could not create query parser");
            }

        ss.setupComplete();
    }


    virtual void tearDown() throw ()
    {
        cleanup();
    }


    virtual ~PlainMVContext()
    {
        cleanup();
    }


    void testAddFiles(Itk::TestMgr * testMgr)
    {
        const MVFTest
            * mvfTest = mvfTests_;

        for (; mvfTest->qualifiedBaseAppClass_ != NULL; ++mvfTest)
            {
                using namespace std;

                printf("Creating volume %s ...\n",
                       mvfTest->qualifiedBaseAppClass_);

                {
                    auto_ptr<VolumeFileIdxUtil>
                        util(new VolumeFileIdxUtil(mvfTest));
                    util->init();

                    util->indexFile(mvfTest->textFilePath_,
                                    analyzer_,
                                    testMgr);
                }

                printf("... created volume %s\n",
                       mvfTest->qualifiedBaseAppClass_);
            }
    }


    void testSearchAllFiles(Itk::TestMgr * testMgr)
    {
        cpix_Result
            result;

        cpix_IdxSearcher
            * searcher = cpix_IdxSearcher_openDb(&result,
                                                 FILEAPPCLASS);

        ITK_ASSERT(testMgr,
                   cpix_Succeeded(&result),
                   "Failed to search multi volume");
    
        cpix_Query
            * query = cpix_QueryParser_parse(queryParser_,
                                             L"happ*");

        if (cpix_Failed(queryParser_))
            {
                cpix_IdxSearcher_releaseDb(searcher);
            }

        ITK_ASSERT(testMgr,
                   cpix_Succeeded(queryParser_),
                   "Could not parse 'happ*'");

        cpix_Hits
            * hits = cpix_IdxSearcher_search(searcher,
                                             query);

        if (cpix_Failed(searcher))
            {
                cpix_IdxSearcher_releaseDb(searcher);
                cpix_Query_destroy(query);
            }

        ITK_ASSERT(testMgr,
                   cpix_Succeeded(searcher),
                   "Could not perform (multi)search");

        PrintHits(hits,
                  testMgr);

        cpix_Hits_destroy(hits);
        cpix_IdxSearcher_releaseDb(searcher);
        cpix_Query_destroy(query);
    }


private:
    void cleanup()
    {
        cpix_Analyzer_destroy(analyzer_);
        analyzer_ = NULL;
        
        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;
    }
};



Itk::TesterBase * CreateMVPlainTests(const MVFTest * mvfTests)
{
    using namespace Itk;

    PlainMVContext
        * context = new PlainMVContext(mvfTests);

    ContextTester
        * contextTester = new ContextTester("plain",
                                            context);

#define TEST "adding"
    contextTester->add(TEST,
                       context,
                       &PlainMVContext::testAddFiles,
                       TEST);
#undef TEST

#define TEST "searchingAll"
    contextTester->add(TEST,
                       context,
                       &PlainMVContext::testSearchAllFiles,
                       TEST);
#undef TEST

    // ... add more

    return contextTester;
}



class DynMVContext : public Itk::ITestContext
{
    cpix_Analyzer    * analyzer_;
    cpix_Query       * query_;
    cpix_IdxSearcher * searcher_;
    cpix_QueryParser * queryParser_;

    const MVFTest    * mvfTests_;

public:

    DynMVContext(const MVFTest * mvfTests)
        : analyzer_(NULL),
          query_(NULL),
          searcher_(NULL),
          queryParser_(NULL),
          mvfTests_(mvfTests)
    {
        ;
    }


    virtual void setup() throw (Itk::PanicExc)
    {
        SetupSentry
            ss(*this);

        cpix_Result
            result;

        cpix_IdxDb_dbgScrapAll(&result);

        analyzer_ = cpix_CreateSimpleAnalyzer(&result);
        if (analyzer_ == NULL)
            {
                ITK_PANIC("Could not create analyzer");
            }


        queryParser_ = cpix_QueryParser_create(&result,
                                               CONTENTS_FIELD,
                                               analyzer_);
        if (queryParser_ == NULL)
            {
                ITK_PANIC("Could not create query parser");
            }

        query_ = cpix_QueryParser_parse(queryParser_,
                                        L"happ*");
        if (cpix_Failed(queryParser_))
            {
                ITK_PANIC("Could not create query parser");
            }

        ss.setupComplete();
    }


    virtual void tearDown() throw()
    {
        cleanup();
    }


    virtual ~DynMVContext()
    {
        cleanup();
    }


    void testCreatingVolumes(Itk::TestMgr * testMgr)
    {
        const MVFTest
            * mvfTest = mvfTests_;

        for (; mvfTest->qualifiedBaseAppClass_ != NULL; ++mvfTest)
            {
                using namespace std;

                printf("Creating volume %s ...\n",
                       mvfTest->qualifiedBaseAppClass_);

                {
                    auto_ptr<VolumeFileIdxUtil>
                        util(new VolumeFileIdxUtil(mvfTest));
                    util->init();
                    
                    util->indexFile(mvfTest->textFilePath_,
                                    analyzer_,
                                    testMgr);
                    util->flush();
                }

                printf("... created volume %s\n",
                       mvfTest->qualifiedBaseAppClass_);

                // after the first volume has been defined, there is
                // something to search on
                createSearcherIfNecessary(testMgr);
                searchAll(testMgr);
            }
    }

    
    void testUnmountingVolumes(Itk::TestMgr * testMgr)
    {
        const MVFTest
            * mvfTest = mvfTests_;

        for (; mvfTest->qualifiedBaseAppClass_ != NULL; ++mvfTest)
            {
                using namespace std;

                searchAll(testMgr);

                printf("Undefining (unmounting) volume %s ...\n",
                       mvfTest->qualifiedBaseAppClass_);

                cpix_IdxDb_undefineVolume(mvfTest->qualifiedBaseAppClass_);

                printf("... undefined (unmounted) volume %s.\n",
                       mvfTest->qualifiedBaseAppClass_);
            }

        searchAll(testMgr);
    }


    void testMountingVolumes(Itk::TestMgr * testMgr)
    {
        const MVFTest
            * mvfTest = mvfTests_;

        for (; mvfTest->qualifiedBaseAppClass_ != NULL; ++mvfTest)
            {
                using namespace std;

                searchAll(testMgr);

                printf("Defining (mounting) volume %s ...\n",
                       mvfTest->qualifiedBaseAppClass_);

                cpix_Result
                    result;

                cpix_IdxDb_defineVolume(&result,
                                        mvfTest->qualifiedBaseAppClass_,
                                        mvfTest->idxDbPath_);

                ITK_ASSERT(testMgr,
                           cpix_Succeeded(&result),
                           "Could not define (mount) volume %s",
                           mvfTest->qualifiedBaseAppClass_);

                printf("... defined (mounted) volume %s.\n",
                       mvfTest->qualifiedBaseAppClass_);
            }

        searchAll(testMgr);
    }


private:

    void createSearcherIfNecessary(Itk::TestMgr * testMgr)
    {
        if (searcher_ == NULL)
            {
                cpix_Result
                    result;
                searcher_ = cpix_IdxSearcher_openDb(&result,
                                                    FILEAPPCLASS);
                
                ITK_ASSERT(testMgr,
                           cpix_Succeeded(&result),
                           "Could not create idx searcher");
            }
    }

    
    void searchAll(Itk::TestMgr * testMgr)
    {
        printf("\nAnd now searching all 'root file'!\n");

        cpix_Hits
            * hits = cpix_IdxSearcher_search(searcher_,
                                             query_);

        if (cpix_Failed(searcher_))
            {
                cpix_IdxSearcher_releaseDb(searcher_);
            }

        ITK_ASSERT(testMgr,
                   cpix_Succeeded(searcher_),
                   "Could not perform (multi)search");

        PrintHits(hits,
                  testMgr);

        cpix_Hits_destroy(hits);
    }


    void cleanup()
    {
        cpix_Analyzer_destroy(analyzer_);
        analyzer_ = NULL;

        cpix_Query_destroy(query_);
        query_ = NULL;

        cpix_IdxSearcher_releaseDb(searcher_);
        searcher_ = NULL;

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;
    }
};



Itk::TesterBase * CreateMVDynTests(const MVFTest * mvfTests)
{
    using namespace Itk;

    DynMVContext
        * context = new DynMVContext(mvfTests);

    ContextTester
        * contextTester = new ContextTester("dyn",
                                            context);

#define TEST "creatingVols"
    contextTester->add(TEST,
                       context,
                       &DynMVContext::testCreatingVolumes,
                       TEST);
#undef TEST

#define TEST "unmountingVols"
    contextTester->add(TEST,
                       context,
                       &DynMVContext::testUnmountingVolumes,
                       TEST);
#undef TEST

#define TEST "mountingVols"
    contextTester->add(TEST,
                       context,
                       &DynMVContext::testMountingVolumes,
                       TEST);
#undef TEST

    // ... add more

    return contextTester;
}


struct MVFTest MVFTests[] = {

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




Itk::TesterBase * CreateMultiVolumeTests()
{
    using namespace Itk;

    SuiteTester
        * multiVolume = new SuiteTester("multivolume");

    multiVolume->add(CreateMVPlainTests(MVFTests));
    multiVolume->add(CreateMVDynTests(MVFTests));

    // TODO add more tests to suite
        
    return multiVolume;
}



