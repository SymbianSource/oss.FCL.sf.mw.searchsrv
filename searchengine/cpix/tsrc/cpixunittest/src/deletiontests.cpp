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
#include <algorithm>

#include "cpixfstools.h"

#include "itk.h"

#include "cpixidxdb.h"

#include "config.h"
#include "suggestion.h"
#include "testutils.h"
#include "testcorpus.h"
#include "setupsentry.h"


class DeletionContext : public Itk::ITestContext
{
private:
    SmsIdxUtil         * util_;
    cpix_Analyzer      * analyzer_;
    cpix_QueryParser   * queryParser_;
    cpix_Query         * query_;

    
public:
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

        util_ = new SmsIdxUtil;
        util_->init();

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


    virtual ~DeletionContext()
    {
        cleanup();
    }


    //
    // public operations
    //
    DeletionContext()
        : util_(NULL),
          analyzer_(NULL),
          queryParser_(NULL),
          query_(NULL)
          
    {
        ;
    }


    //
    // Test Member Function
    //
    void testAdd_00(Itk::TestMgr * testMgr)
    {
        testAddSms(testMgr,
                   1,
                   L"This is a happy message");
        util_->flush();
        testSearch(testMgr);
    }

    void testAdd_01(Itk::TestMgr * testMgr)
    {
        testAddSms(testMgr,
                   2,
                   L"This is another happy message");
        util_->flush();
        testSearch(testMgr);
    }

    void testFlush_02(Itk::TestMgr * testMgr)
    {
        testFlush(testMgr);
    }

    void testAdd_03(Itk::TestMgr * testMgr)
    {
        testAddSms(testMgr,
                   3,
                   L"This is a 3rd happy message");
        util_->flush();
        testSearch(testMgr);
    }

    void testDelete_04(Itk::TestMgr * testMgr)
    {
        testDeleteSms(3,
                      testMgr);
        util_->flush();
        testSearch(testMgr);
    }

    void testDelete_05(Itk::TestMgr * testMgr)
    {
        testDeleteSms(1,
                      testMgr);
        util_->flush();
        testSearch(testMgr);
    }

    void testReOpenIdx_06(Itk::TestMgr * testMgr)
    {
        testReOpenIdxDb(testMgr);
        testSearch(testMgr);
    }
    
    void testAdd_07(Itk::TestMgr * testMgr)
    {
        testAddSms(testMgr,
                   4,
                   L"This is the 4th happy message");
        util_->flush();
        testSearch(testMgr);
    }

    void testAdd_08(Itk::TestMgr * testMgr)
    {
        testAddSms(testMgr,
                   5,
                   L"This is the 5th happy message");
        util_->flush();
        testSearch(testMgr);
    }

    void testDelete_09(Itk::TestMgr * testMgr)
    {
        testDeleteSms(2,
                      testMgr);
        util_->flush();
        testSearch(testMgr);
    }

    void testDelete_10(Itk::TestMgr * testMgr)
    {
        testDeleteSms(4,
                      testMgr);
        util_->flush();
        testSearch(testMgr);
    }

    void testReOpenIdx_11(Itk::TestMgr * testMgr)
    {
        testReOpenIdxDb(testMgr);
        testSearch(testMgr);
    }
    


private:
    //
    // private methods
    //
    void cleanup()
    {
        delete util_;
        util_ = NULL;

        cpix_Analyzer_destroy(analyzer_);
        analyzer_ = NULL;

        cpix_Query_destroy(query_);
        query_ = NULL;

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;
    }


    
    void testReOpenIdxDb(Itk::TestMgr *)
    {
        delete util_;
        util_ = NULL;

        util_ = new SmsIdxUtil;
        util_->init(false); // don't create
    }


    void testAddSms(Itk::TestMgr  * testMgr,
                    size_t          docUid,
                    const wchar_t * body)
    {
        using namespace Itk;

        util_->indexSms(docUid,
                        body,
                        analyzer_,
                        testMgr);
    }


    void testFlush(Itk::TestMgr * testMgr)
    {
        cpix_IdxDb_flush(util_->idxDb());

        ITK_EXPECT(testMgr,
                   cpix_Succeeded(util_->idxDb()),
                   "Could not flush idx");
    }


    void testDeleteSms(size_t         docUid,
                       Itk::TestMgr * testMgr)
    {
        util_->deleteSms(docUid,
                         testMgr);
    }


    void testSearch(Itk::TestMgr * testMgr)
    {
        using namespace Itk;

        cpix_Hits
            * hits = cpix_IdxDb_search(util_->idxDb(),
                                       query_);

        if (cpix_Failed(util_->idxDb()))
            {
                ITK_EXPECT(testMgr,
                           false,
                           "Failed to search");
                cpix_ClearError(util_->idxDb());
            }
        else
            {
                util_->printHits(hits,
                                 testMgr);
                cpix_Hits_destroy( hits ); 
            }
    }


};



Itk::TesterBase * CreateDeletionTests()
{
    using namespace Itk;

    DeletionContext
        * context = new DeletionContext;

    ContextTester
        * contextTester = new ContextTester("deletion",
                                            context);

#define TEST "00_add"
    contextTester->add(TEST,
                       context,
                       &DeletionContext::testAdd_00,
                       TEST);
#undef TEST

#define TEST "01_add"
    contextTester->add(TEST,
                       context,
                       &DeletionContext::testAdd_01,
                       TEST);
#undef TEST

#define TEST "02_flush"
    contextTester->add(TEST,
                       context,
                       &DeletionContext::testFlush_02);
#undef TEST

#define TEST "03_add"
    contextTester->add(TEST,
                       context,
                       &DeletionContext::testAdd_03,
                       TEST);
#undef TEST

#define TEST "04_delete"
    contextTester->add(TEST,
                       context,
                       &DeletionContext::testDelete_04,
                       TEST);
#undef TEST

#define TEST "05_delete"
    contextTester->add(TEST,
                       context,
                       &DeletionContext::testDelete_05,
                       TEST);
#undef TEST

#define TEST "06_reopen"
    contextTester->add(TEST,
                       context,
                       &DeletionContext::testReOpenIdx_06,
                       TEST);
#undef TEST

#define TEST "07_add"
    contextTester->add(TEST,
                       context,
                       &DeletionContext::testAdd_07,
                       TEST);
#undef TEST

#define TEST "08_add"
    contextTester->add(TEST,
                       context,
                       &DeletionContext::testAdd_08,
                       TEST);
#undef TEST

#define TEST "09_delete"
    contextTester->add(TEST,
                       context,
                       &DeletionContext::testDelete_09,
                       TEST);
#undef TEST

#define TEST "10_delete"
    contextTester->add(TEST,
                       context,
                       &DeletionContext::testDelete_10,
                       TEST);
#undef TEST

#define TEST "11_reopen"
    contextTester->add(TEST,
                       context,
                       &DeletionContext::testReOpenIdx_11,
                       TEST);
#undef TEST



    return contextTester;
}
