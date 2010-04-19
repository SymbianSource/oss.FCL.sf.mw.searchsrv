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
#include <unistd.h>

#include <memory>
#include <iostream>
#include <list>
#include <map>
#include <vector>

#include "cpixfstools.h"

#include "itk.h"

#include "cpixidxdb.h"
#include "cpixsearch.h"
#include "idxdbmgr.h" // from private folder: whitebox testing

#include "config.h"
#include "suggestion.h"
#include "testutils.h"
#include "testcorpus.h"
#include "setupsentry.h"

//
// these constants are set so that a cpix_XXX entity should be
// eligible for internal (transparent) release between the 2nd and 3rd
// period (sleep)
//
const unsigned int SleepSec   = 2;
const char         LastAccessedPattern[] = "(last accessed: ";


class HeartbeatContext : public Itk::ITestContext
{
    cpix_Analyzer            * analyzer_;
    cpix_QueryParser         * queryParser_;
    cpix_Query               * query_;

    LineTestCorpusRef          corpus_;
    SmsIdxUtil               * smsIdxUtil_;
    FileIdxUtil              * fileIdxUtil_;
    cpix_IdxSearcher         * searcher_;

    cpix_LogLevel              oldLogLevel_;


public:
    virtual void setup() throw (Itk::PanicExc)
    {
        SetupSentry
            ss(*this);

        cpix_Result
            result;

        oldLogLevel_ = cpix_setLogLevel(CPIX_LL_TRACE);

        cpix_IdxDb_dbgScrapAll(&result);

        if (cpix_Failed(&result))
            {
                ITK_PANIC("Could not dbg scrapp all indexes");
            }

        analyzer_ = cpix_CreateSimpleAnalyzer(&result);

        if (analyzer_ == NULL)
            {
                ITK_PANIC("Could not create analyzer");
            }

        queryParser_ = cpix_QueryParser_create(&result,
                                               LBODY_FIELD,
                                               analyzer_);
        if (queryParser_ == NULL)
            {
                ITK_PANIC("Could not create query parser");
            }

        query_ = cpix_QueryParser_parse(queryParser_,
                                        L"happy");
        if (cpix_Failed(queryParser_)
            || query_ == NULL)
            {
                ITK_PANIC("Could not parse query string");
            }

        ss.setupComplete();
    }

    virtual void tearDown() throw()
    {
        cleanup();
    }


    virtual ~HeartbeatContext()
    {
        cleanup();
    }


    //
    // public operations
    //
    HeartbeatContext()
        : analyzer_(NULL),
          queryParser_(NULL),
          query_(NULL),
          corpus_(DEFAULT_TEST_CORPUS_PATH),
          smsIdxUtil_(NULL),
          fileIdxUtil_(NULL),
          searcher_(NULL)
          
    {
        ;
    }


    //
    // public (test) operations
    //
    void testCreateFileIdxDb(Itk::TestMgr * testMgr)
    {
        using namespace std;

        doCommonJobs(testMgr,
                     false);

        auto_ptr<FileIdxUtil>
            fiu(new FileIdxUtil);
        fiu->init(true);

        fileIdxUtil_ = fiu.release();
    }


    void testCreateSmsIdxDb(Itk::TestMgr * testMgr)
    {
        using namespace std;

        doCommonJobs(testMgr);

        auto_ptr<SmsIdxUtil>
            siu(new SmsIdxUtil);
        siu->init(true);
        
        smsIdxUtil_ = siu.release();
    }
    

    void testAddSomeSms(Itk::TestMgr * testMgr)
    {
        using namespace std;

        doCommonJobs(testMgr);

        for (size_t i = 0; i < 5; ++i)
            {
                std::wstring
                    body = corpus_.item(i);
                smsIdxUtil_->indexSms(i,
                                      body.c_str(),
                                      analyzer_,
                                      testMgr);
                if ((i % 5) == 0)
                    {
                        ITK_DBGMSG(testMgr,
                                   ".");
                    }
            }
    }


    void testIdle(Itk::TestMgr * testMgr)
    {
        doCommonJobs(testMgr);
    }


    void testUseMultiSearcher(Itk::TestMgr * testMgr)
    {
        doCommonJobs(testMgr);

        if (searcher_ == NULL)
            {
                cpix_Result
                    result;

                searcher_ = cpix_IdxSearcher_openDb(&result,
                                                    "root");
                                     
                if (searcher_ == NULL)
                    {
                        ITK_ASSERT(testMgr,
                                   false,
                                   "Could not create multi searcher");
                    }
            }

        cpix_Hits
            * hits = cpix_IdxSearcher_search(searcher_,
                                             query_);
        ITK_EXPECT(testMgr,
                   cpix_Succeeded(query_),
                   "Could not search with multisearcher");
        if (hits != NULL)
            {
                PrintHits(hits,
                          testMgr);

                cpix_Hits_destroy(hits);
            }
    }

    
    void testAddSomeFile(Itk::TestMgr * testMgr)
    {
        doCommonJobs(testMgr);

        fileIdxUtil_->indexFile(FILE_TEST_CORPUS_PATH "\\en\\1.txt",
                                analyzer_,
                                testMgr);
    }

    
    void testReleaseAll(Itk::TestMgr * testMgr)
    {
        doCommonJobs(testMgr);

        delete fileIdxUtil_;
        fileIdxUtil_ = NULL;

        delete smsIdxUtil_;
        smsIdxUtil_ = NULL;

        cpix_IdxSearcher_releaseDb(searcher_);
        searcher_ = NULL;
    }
    

private:
    //
    // private methods
    //
    void cleanup()
    {
        delete smsIdxUtil_;
        smsIdxUtil_ = NULL;

        delete fileIdxUtil_;
        fileIdxUtil_ = NULL;

        cpix_IdxSearcher_releaseDb(searcher_);
        searcher_ = NULL;

        cpix_Analyzer_destroy(analyzer_);
        analyzer_ = NULL;

        cpix_Query_destroy(query_);
        query_ = NULL;

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;
        

        cpix_setLogLevel(oldLogLevel_);
    }
    

    void doCommonJobs(Itk::TestMgr * testMgr,
                      bool           doSleep = true)
    {
        if (doSleep)
            {
                printf("Sleeping %d seconds...\n",
                       SleepSec);
                ITK_EXPECT(testMgr,
                           sleep(SleepSec) == 0,
                           "Could not sleep");
                printf("... slept.\n");
            }

        cpix_doHousekeeping();

        printf("Housekept.\n");

        // TODO
        // measure memory consumption (not tested by I/O)

        cpix_dbgDumpState();
    }

};


Itk::TesterBase * CreateHeartbeatTests()
{
    using namespace Itk;

    HeartbeatContext
        * heartbeatContext = new HeartbeatContext();
    ContextTester
        * contextTester = new ContextTester("heartbeat",
                                            heartbeatContext);
#define TEST "00_createFileIdxDb"
    contextTester->add(TEST,
                       heartbeatContext,
                       &HeartbeatContext::testCreateFileIdxDb,
                       TEST,
                       LastAccessedPattern);
#undef TEST

#define TEST "01_createSmsIdxDb"
    contextTester->add(TEST,
                       heartbeatContext,
                       &HeartbeatContext::testCreateSmsIdxDb,
                       TEST,
                       LastAccessedPattern);
#undef TEST

#define TEST "02_addSomeSms"
    contextTester->add(TEST,
                       heartbeatContext,
                       &HeartbeatContext::testAddSomeSms,
                       TEST,
                       LastAccessedPattern);
#undef TEST

#define TEST "03_idle"
    contextTester->add(TEST,
                       heartbeatContext,
                       &HeartbeatContext::testIdle,
                       TEST,
                       LastAccessedPattern);
#undef TEST

#define TEST "04_idle"
    contextTester->add(TEST,
                       heartbeatContext,
                       &HeartbeatContext::testIdle,
                       TEST,
                       LastAccessedPattern);
#undef TEST

#define TEST "05_idle"
    contextTester->add(TEST,
                       heartbeatContext,
                       &HeartbeatContext::testIdle,
                       TEST,
                       LastAccessedPattern);
#undef TEST

#define TEST "06_useMultiSearcher"
    contextTester->add(TEST,
                       heartbeatContext,
                       &HeartbeatContext::testUseMultiSearcher,
                       TEST,
                       LastAccessedPattern);
#undef TEST

#define TEST "07_addSomeFile"
    contextTester->add(TEST,
                       heartbeatContext,
                       &HeartbeatContext::testAddSomeFile,
                       TEST,
                       LastAccessedPattern);
#undef TEST

#define TEST "08_idle"
    contextTester->add(TEST,
                       heartbeatContext,
                       &HeartbeatContext::testIdle,
                       TEST,
                       LastAccessedPattern);
#undef TEST

#define TEST "09_idle"
    contextTester->add(TEST,
                       heartbeatContext,
                       &HeartbeatContext::testIdle,
                       TEST,
                       LastAccessedPattern);
#undef TEST

#define TEST "10_idle"
    contextTester->add(TEST,
                       heartbeatContext,
                       &HeartbeatContext::testIdle,
                       TEST,
                       LastAccessedPattern);
#undef TEST

#define TEST "11_useMultiSearcher"
    contextTester->add(TEST,
                       heartbeatContext,
                       &HeartbeatContext::testUseMultiSearcher,
                       TEST,
                       LastAccessedPattern);
#undef TEST

#define TEST "12_releaseAll"
    contextTester->add(TEST,
                       heartbeatContext,
                       &HeartbeatContext::testReleaseAll,
                       TEST,
                       LastAccessedPattern);
#undef TEST


#define TEST "13_idle"
    contextTester->add(TEST,
                       heartbeatContext,
                       &HeartbeatContext::testIdle,
                       TEST,
                       LastAccessedPattern);
#undef TEST

    // ... add more

    return contextTester;
}
