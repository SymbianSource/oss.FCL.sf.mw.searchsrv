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

#include "std_log_result.h"

/****
 * Simple test cases (one client)
 */

class SimpleSmsContext : public Itk::ITestContext
{
protected:
    //
    // protected members
    //
    SmsIdxUtil        * util_;
    cpix_Analyzer     * analyzer_;
    cpix_Query        * query_;
    cpix_Query        * termsQuery_;

    cpix_QueryParser  * queryParser_;

    LineTestCorpusRef   corpus_;
    

public:
    //
    // From interface Itk::ITestContext
    //
    virtual void setup() throw (Itk::PanicExc)
    {
        SetupSentry
            ss(*this);

        util_ = new SmsIdxUtil;
        util_->init();

        cpix_Result
            result;

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
        
        // terms (suggestions) query using unified search API
        termsQuery_ = cpix_QueryParser_parse(queryParser_,
                                             L"$terms<50>(h*)");

        if (cpix_Failed(queryParser_)
            || termsQuery_ == NULL)
            {
                ITK_PANIC("Could not parse terms query string");
            }

        ss.setupComplete();
    }


    virtual void tearDown() throw()
    {
        cleanup();
    }


    virtual ~SimpleSmsContext()
    {
        cleanup();
    }


    //
    // public operations
    //
    SimpleSmsContext()
        : util_(NULL),
          analyzer_(NULL),
          query_(NULL),
          termsQuery_(NULL),
          queryParser_(NULL),
          corpus_(DEFAULT_TEST_CORPUS_PATH)
    {
        ;
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

        cpix_Query_destroy(termsQuery_);
        termsQuery_ = NULL;

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;
    }
};


/********
 * Simple Indexing Test Sequence
 */
class SimpleIdxSeq : public SimpleSmsContext
{
public:

    void testAddSome(Itk::TestMgr * testMgr)
    {
        using namespace Itk;

        Timestamp
            start,
            end;
        getTimestamp(&start);

        for (size_t i = 0; i < 200; ++i) 
            {
                std::wstring
                    body = corpus_.item(i);
                util_->indexSms(i,
                                body.c_str(),
                                analyzer_,
                                testMgr);
                if ((i % 5) == 0)
                    {
                        ITK_DBGMSG(testMgr,
                                   ".");
                    }
            }

        getTimestamp(&end);

        long
            ms = getElapsedMs(&end,
                              &start);
        ITK_REPORT(testMgr,
                   "Adding 200 SMS",
                   "%d ms",
                   ms);

        util_->flush();
    }


    void testSearch(Itk::TestMgr * testMgr)
    {
        using namespace Itk;

        Timestamp
            start,
            end;
        getTimestamp(&start);

        cpix_Hits
            * hits = cpix_IdxDb_search(util_->idxDb(),
                                       query_);

        if (cpix_Failed(util_->idxDb()))
            {
                ITK_EXPECT(testMgr,
                           false,
                           "Failed to search index");
                cpix_ClearError(util_->idxDb());
            }
        else
            {
                util_->printHits(hits,
                                 testMgr);
            }

        getTimestamp(&end);
        
        cpix_Hits_destroy( hits );  

        long
            ms = getElapsedMs(&end,
                              &start);
        ITK_REPORT(testMgr,
                   "Searching 'happy' in 200 SMS idx",
                   "%d ms",
                   ms);
    }



    void testSuggest(Itk::TestMgr * testMgr)
    {
        using namespace Itk;

        Timestamp
            start,
            end;
        getTimestamp(&start);

        // getting terms (suggestions) using unified search API
        cpix_Hits
            * hits = cpix_IdxDb_search(util_->idxDb(),
                                       termsQuery_);

        if (cpix_Failed(util_->idxDb()))
            {
                ITK_EXPECT(testMgr,
                           false,
                           "Failed to get suggestions form the index");
                cpix_ClearError(util_->idxDb());
            }
        else
            {
                using namespace std;

                int32_t
                    hitCount = cpix_Hits_length(hits);

                if (cpix_Failed(hits))
                    {
                        ITK_EXPECT(testMgr,
                                   false,
                                   "Failed to get number of hits");
                        cpix_ClearError(hits);
                        return;
                    }

                cout << "Number of hits: " << hitCount << endl;
            }

        getTimestamp(&end);
        
        Suggestion::printSuggestions(hits,
                                     testMgr);
        
        cpix_Hits_destroy( hits );  
        
        long
            ms = getElapsedMs(&end,
                              &start);
        ITK_REPORT(testMgr,
                   "Suggesting 'h' in 200 SMS idx",
                   "%d ms",
                   ms);
    }
    

    void testDump(Itk::TestMgr  * testMgr,
                  const wchar_t * dumpQryStr)
    {
        using namespace Itk;

        cpix_Query
            * dumpQuery = cpix_QueryParser_parse(queryParser_,
                                                 dumpQryStr);

        ITK_EXPECT(testMgr,
                   cpix_Succeeded(queryParser_),
                   "Could not parse dump query string '%S'",
                   dumpQryStr);

        if (cpix_Failed(queryParser_))
            {
                return;
            }

        Timestamp
            start,
            end;
        getTimestamp(&start);

        cpix_Hits
            * hits = cpix_IdxDb_search(util_->idxDb(),
                                       dumpQuery);

        getTimestamp(&end);

        if (cpix_Failed(util_->idxDb()))
            {
                ITK_EXPECT(testMgr,
                           false,
                           "Failed to dump");
                cpix_ClearError(util_->idxDb());
            }
        else
            {
                util_->printHits(hits,
                                 testMgr);
                
            }

        long
            ms = getElapsedMs(&end,
                              &start);
        ITK_REPORT(testMgr,
                   "Dumping in 200 SMS idx",
                   "%d ms",
                   ms);

        cpix_Hits_destroy(hits);  
        cpix_Query_destroy(dumpQuery);
    }


    void testDump1(Itk::TestMgr * testMgr)
    {
        testDump(testMgr,
                  L"*");
    }

    
    void testDump2(Itk::TestMgr * testMgr)
    {
        testDump(testMgr,
                  L"$dump");
    }


    void testDump3(Itk::TestMgr * testMgr)
    {
        testDump(testMgr,
                  L"* AND _aggregate:happy");
    }


    void testDump4(Itk::TestMgr * testMgr)
    {
        testDump(testMgr,
                  L"$dump(_aggregate:happy)");
    }


    void testUpdate(Itk::TestMgr * testMgr)
    {
        using namespace Itk;

        Timestamp
            start,
            end;
        getTimestamp(&start);

        int32_t
            maxInsertBufSize = 32 * 1024;

        cpix_IdxDb_setMaxInsertBufSize(util_->idxDb(),
                                          maxInsertBufSize);
        ITK_EXPECT(testMgr,
                   cpix_Succeeded(util_->idxDb()),
                   "Failed to set max insert buffer size to %d",
                   maxInsertBufSize);

        util_->indexSms(23,
                        L"This UPDATED msg body does not have the h.appy word in it anymore",
                        analyzer_,
                        testMgr,
                        true); // update
        util_->indexSms(32,
                        L"This UPDATED msg body does have the happy word in it",
                        analyzer_,
                        testMgr,
                        true); // update

        for (int i = 0; i < 10; ++i)
            {
                util_->indexSms(40 + i,
                                L"Just to update couple of times, to fill up the insert buffer in update state too. Garble, gobledegook.",
                                analyzer_,
                                testMgr,
                                true); // update
            }

        getTimestamp(&end);

        long
            ms = getElapsedMs(&end,
                              &start);
        ITK_REPORT(testMgr,
                   "Updating 2 docs in 200 SMS idx",
                   "%d ms",
                   ms);

        util_->flush();
    }
};


#define SUITE "partsms"

Itk::TesterBase * CreateSimpleIdxSequence()
{
    using namespace Itk;

#define SEQUENCE "simpleidx"

    SimpleIdxSeq
        * simpleIdxSeq = new SimpleIdxSeq;
    ContextTester
        * simpleIdxer = new ContextTester(SEQUENCE,
                                          simpleIdxSeq);

#define TEST "addSome"
    simpleIdxer->add(TEST,
                     simpleIdxSeq,
                     &SimpleIdxSeq::testAddSome,
                     TEST);
#undef TEST

#define TEST "search"
    simpleIdxer->add(TEST,
                     simpleIdxSeq,
                     &SimpleIdxSeq::testSearch,
                     TEST);
#undef TEST

#define TEST "suggest"
    simpleIdxer->add(TEST,
                     simpleIdxSeq,
                     &SimpleIdxSeq::testSuggest,
                     TEST);
#undef TEST

#define TEST "dump1"
    simpleIdxer->add(TEST,
                     simpleIdxSeq,
                     &SimpleIdxSeq::testDump1,
                     TEST);
#undef TEST

    
#define TEST "dump2"
    simpleIdxer->add(TEST,
                     simpleIdxSeq,
                     &SimpleIdxSeq::testDump2,
                     TEST);
#undef TEST

#define TEST "dump3"
    simpleIdxer->add(TEST,
                     simpleIdxSeq,
                     &SimpleIdxSeq::testDump3,
                     TEST);
#undef TEST

#define TEST "dump4"
    simpleIdxer->add(TEST,
                     simpleIdxSeq,
                     &SimpleIdxSeq::testDump4,
                     TEST);
#undef TEST

#define TEST "update"
    simpleIdxer->add(TEST,
                     simpleIdxSeq,
                     &SimpleIdxSeq::testUpdate,
                     TEST);
#undef TEST

#define TEST "search2"
    simpleIdxer->add(TEST,
                     simpleIdxSeq,
                     &SimpleIdxSeq::testSearch,       // Same test func with 
                     TEST);  // different name! :-)
#undef TEST

#define TEST "suggest2"
    simpleIdxer->add(TEST,
                     simpleIdxSeq,
                     &SimpleIdxSeq::testSuggest,
                     TEST);
#undef TEST

    return simpleIdxer;

#undef SEQUENCE
}



    /****
     * Parallel test cases (two clients)
     */
    // TODO perhaps move it to a common utils file
class ParallelSmsContext : public Itk::ITestContext
{
protected:
    //
    // protected members
    //
    SmsIdxUtil       * util1_;
    SmsIdxUtil       * util2_;
    cpix_Analyzer    * analyzer_;
    cpix_Query       * query_;
    LineTestCorpusRef  corpus_;
    cpix_QueryParser * queryParser_;
    

public:
    //
    // From interface Itk::ITestContext
    //
    virtual void setup() throw (Itk::PanicExc)
    {
        SetupSentry
            ss(*this);

        util1_ = new SmsIdxUtil;
        util1_->init();
        util2_ = new SmsIdxUtil;
        util2_->init(false);  // only open

        cpix_Result
            result;

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


    virtual ~ParallelSmsContext()
    {
        cleanup();
    }


    //
    // public operations
    //
    ParallelSmsContext()
        : util1_(NULL),
          util2_(NULL),
          analyzer_(NULL),
          query_(NULL),          
          corpus_(DEFAULT_TEST_CORPUS_PATH),
          queryParser_(NULL)
    {
        ;
    }

    
private:
    //
    // private methods
    //
    void cleanup()
    {
        delete util1_;
        delete util2_;
        util1_ = NULL;
        util2_ = NULL;

        cpix_Analyzer_destroy(analyzer_);
        analyzer_ = NULL;

        cpix_Query_destroy(query_);
        query_ = NULL;

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;
    }
};




    /********
     * Parallel Indexing Test Sequence
     *
     * One client is indexing and updating content while the other is
     * searching - and it should make no difference whatsoever.
     */
class ParallelIdxSeq : public ParallelSmsContext
{
private:
    size_t curId_;

public:

    virtual void setup() throw (Itk::PanicExc)
    {
        ParallelSmsContext::setup();

        curId_ = 0;
    }


    void testAddSome(Itk::TestMgr * testMgr)
    {
        // the same function is invoked multiple times to add
        // more and more documents (in batches of 50)
        for (size_t i = 0; i < 50; ++i)
            {
                ++curId_;
                std::wstring
                    body = corpus_.item(curId_);
                util1_->indexSms(curId_,
                                 body.c_str(),
                                 analyzer_,
                                 testMgr);
            }
        util1_->flush();
    }


    void testSearch(Itk::TestMgr * testMgr)
    {
        cpix_Hits
            * hits = cpix_IdxDb_search(util2_->idxDb(),
                                       query_);

        if (cpix_Failed(util2_->idxDb()))
            {
                ITK_EXPECT(testMgr,
                           false,
                           "Failed to search index");
                cpix_ClearError(util2_->idxDb());
            }
        else
            {
                util2_->printHits(hits,
                                  testMgr);
                
                cpix_Hits_destroy( hits );  
            }
    }

    
    void testUpdate(Itk::TestMgr * testMgr)
    {
        int32_t
            maxInsertBufSize = 32 * 1024; // 32 KB

        cpix_IdxDb_setMaxInsertBufSize(util1_->idxDb(),
                                       maxInsertBufSize);
        ITK_EXPECT(testMgr,
                   cpix_Succeeded(util1_->idxDb()),
                   "Failed to set max insert buffer size to %d",
                   maxInsertBufSize);

        util1_->indexSms(23,
                         L"This UPDATED msg body does not have the h.appy word in it anymore",
                         analyzer_,
                         testMgr,
                         true); // update
        util1_->indexSms(32,
                         L"This UPDATED msg body does have the happy word in it",
                         analyzer_,
                         testMgr,
                         true); // update

        for (int i = 0; i < 10; ++i)
            {
                util1_->indexSms(40 + i,
                                 L"Just to update couple of times, to fill up the insert buffer in update state too. Garble, gobledegook.",
                                 analyzer_,
                                 testMgr,
                                 true); // update
            }
        util1_->flush();
    }
};



Itk::TesterBase * CreateParallelIdxSequence()
{
    using namespace Itk;

#define SEQUENCE "parallelidx"

    ParallelIdxSeq
        * parallelIdxSeq = new ParallelIdxSeq;
    ContextTester
        * parallelIdxer = new ContextTester(SEQUENCE,
                                            parallelIdxSeq);

#define TEST "addSome1"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParallelIdxSeq::testAddSome,
                       TEST);
#undef TEST

#define TEST "search1"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParallelIdxSeq::testSearch,
                       TEST);
#undef TEST

#define TEST "addSome2"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParallelIdxSeq::testAddSome,
                       TEST);
#undef TEST

#define TEST "search2"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParallelIdxSeq::testSearch,
                       TEST);
#undef TEST

#define TEST "addSome3"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParallelIdxSeq::testAddSome,
                       TEST);
#undef TEST

#define TEST "search3"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParallelIdxSeq::testSearch,
                       TEST);
#undef TEST

#define TEST "addSome4"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParallelIdxSeq::testAddSome,
                       TEST);
#undef TEST

#define TEST "search4"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParallelIdxSeq::testSearch,
                       TEST);
#undef TEST

#define TEST "update"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParallelIdxSeq::testUpdate,
                       TEST);
#undef TEST

#define TEST "search5"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParallelIdxSeq::testSearch,  // Same test func with 
                       TEST);                        // different name! :-)
#undef TEST

    return parallelIdxer;

#undef SEQUENCE
}


    /****
     * Mixed index test cases
     */
class MixedContext : public Itk::ITestContext
{
protected:
    //
    // protected members
    //
    SmsIdxUtil   * smsUtil1_;
    SmsIdxUtil   * smsUtil2_;
    FileIdxUtil  * fileUtil1_;
    FileIdxUtil  * fileUtil2_;

    cpix_Analyzer  * analyzer_;
    cpix_Query     * smsQuery_;
    cpix_Query     * fileQuery_;
    LineTestCorpusRef   corpus_;

    cpix_QueryParser * queryParser_;
    
public:
    virtual void setup() throw (Itk::PanicExc)
    {
        SetupSentry
            ss(*this);

        cpix_Result
            result;

        cpix_IdxDb_dbgScrapAll(&result);

        using namespace std;

        smsUtil1_ = new SmsIdxUtil;
        smsUtil1_->init();

        smsUtil2_ = new SmsIdxUtil;
        smsUtil2_->init(false);

        fileUtil1_ = new FileIdxUtil;
        fileUtil1_->init();

        fileUtil2_ = new FileIdxUtil;
        fileUtil2_->init(false);
        
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

        smsQuery_ = cpix_QueryParser_parse(queryParser_,
                                           L"happy");
        if (cpix_Failed(queryParser_)
            || smsQuery_ == NULL)
            {
                ITK_PANIC("Could not parser query string");
            }

        fileQuery_ = cpix_QueryParser_parse(queryParser_,
                                            CONTENTS_FIELD L":happy");
        if (cpix_Failed(queryParser_)
            || fileQuery_ == NULL)
            {
                ITK_PANIC("Could not parser query string");
            }
        
        ss.setupComplete();
    }


    virtual void tearDown() throw ()
    {
        cleanup();
    }


    virtual ~MixedContext()
    {
        cleanup();
    }


    //
    // public operations
    //
    MixedContext()
        : smsUtil1_(NULL),
          smsUtil2_(NULL),
          fileUtil1_(NULL),
          fileUtil2_(NULL),
          analyzer_(NULL),
          smsQuery_(NULL),
          fileQuery_(NULL),
          corpus_(DEFAULT_TEST_CORPUS_PATH),
          queryParser_(NULL)
    {
        ;
    }


private:
    //
    // private methods
    //
    void cleanup()
    {
        delete smsUtil1_;
        delete smsUtil2_;
        delete fileUtil1_;
        delete fileUtil2_;
        smsUtil1_ = NULL;
        smsUtil2_ = NULL;
        fileUtil1_ = NULL;
        fileUtil2_ = NULL;

        cpix_Analyzer_destroy(analyzer_);
        analyzer_ = NULL;

        cpix_Query_destroy(smsQuery_);
        smsQuery_ = NULL;

        cpix_Query_destroy(fileQuery_);
        fileQuery_ = NULL;

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;
    }

};


    /******
     * Mixed Indexing test sequence (2x2 clients on 2 indexes).
     */
class MixedIdxSeq : public MixedContext, public Cpt::IFileVisitor
{
private:
    size_t        curId_;
    Itk::TestMgr * testMgr_;


public:

    //
    // from Cpt::IFileVisitor
    //
    virtual bool visitFile(const char * path)
    {
        bool
            goOn = true;

        fileUtil1_->indexFile(path,
                              analyzer_,
                              testMgr_);

        return goOn;
    }
    
    
    virtual DirVisitResult visitDirPre(const char * /*path*/)
    {
        return IFV_CONTINUE;
    }


    virtual bool visitDirPost(const char * /*path*/)
    {
        return true;
    }


    //
    // actual tests
    //
    virtual void setup() throw (Itk::PanicExc)
    {
        MixedContext::setup();

        curId_ = 0;
    }

    
    void testAddSomeSms(Itk::TestMgr * testMgr)
    {
        for (size_t i = 0; i < 50; ++i)
            {
                ++curId_;
                std::wstring
                    body = corpus_.item(curId_);
                smsUtil1_->indexSms(curId_,
                                    body.c_str(),
                                    analyzer_,
                                    testMgr);
            }
        smsUtil1_->flush();
    }


    void testAddFiles(Itk::TestMgr * testMgr)
    {
        testMgr_ = testMgr;
        Cpt::traverse(FILE_TEST_CORPUS_PATH "\\en",
                      this);
        fileUtil1_->flush();
    }

    
    void testSearchSms(Itk::TestMgr * testMgr)
    {
        using namespace Itk;

        cpix_Hits
            * hits = cpix_IdxDb_search(smsUtil2_->idxDb(),
                                       smsQuery_);

        if (cpix_Failed(smsUtil2_->idxDb()))
            {
                ITK_EXPECT(testMgr,
                           false,
                           "Failed to search index");
                cpix_ClearError(smsUtil2_->idxDb());
            }
        else
            {
                smsUtil2_->printHits(hits,
                                     testMgr);
                cpix_Hits_destroy(hits);
            }

    }


    void testSearchFiles(Itk::TestMgr * testMgr)
    {
        using namespace Itk;

        cpix_Hits
            * hits = cpix_IdxDb_search(fileUtil2_->idxDb(),
                                       fileQuery_);

        if (cpix_Failed(fileUtil2_->idxDb()))
            {
                ITK_EXPECT(testMgr,
                           false,
                           "Failed to search index");
                cpix_ClearError(fileUtil2_->idxDb());
            }
        else
            {
                fileUtil2_->printHits(hits,
                                      testMgr);
                cpix_Hits_destroy(hits);
            }
    }
    
};



Itk::TesterBase * CreateMixedIdxSequence()
{
    using namespace Itk;

#define SEQUENCE "mixedidx"

    MixedIdxSeq
        * mixedIdxSeq = new MixedIdxSeq;
    ContextTester
        * mixedIdxer = new ContextTester(SEQUENCE,
                                         mixedIdxSeq);

#define TEST "addSomeSms1"
    mixedIdxer->add(TEST,
                    mixedIdxSeq,
                    &MixedIdxSeq::testAddSomeSms,
                    TEST);
#undef TEST

#define TEST "searchSms1"
    mixedIdxer->add(TEST,
                    mixedIdxSeq,
                    &MixedIdxSeq::testSearchSms,
                    TEST);
#undef TEST

#define TEST "addSomeFiles1"
    mixedIdxer->add(TEST,
                    mixedIdxSeq,
                    &MixedIdxSeq::testAddFiles,
                    TEST);
#undef TEST
/*
#define TEST "searchFiles1"
    mixedIdxer->add(TEST,
                    mixedIdxSeq,
                    &MixedIdxSeq::testSearchFiles,
                    TEST);
#undef TEST

#define TEST "addSomeSms2"
    mixedIdxer->add(TEST,
                    mixedIdxSeq,
                    &MixedIdxSeq::testAddSomeSms,
                    TEST);
#undef TEST

#define TEST "searchSms2"
    mixedIdxer->add(TEST,
                    mixedIdxSeq,
                    &MixedIdxSeq::testSearchSms,
                    TEST);
#undef TEST
*/

    return mixedIdxer;

#undef SEQUENCE
}





    /****
     * ParMSearcher test cases (two clients)
     */
    // TODO perhaps move it to a common utils file
class ParMSearcherSmsContext : public Itk::ITestContext
{
protected:
    //
    // protected members
    //
    SmsIdxUtil       * util_;
    cpix_IdxSearcher * searcher_;
    cpix_Analyzer    * analyzer_;
    cpix_Query       * query_;
    LineTestCorpusRef  corpus_;
    cpix_QueryParser * queryParser_;
    

public:
    //
    // From interface Itk::ITestContext
    //
    virtual void setup() throw (Itk::PanicExc)
    {
        SetupSentry
            ss(*this);

        util_ = new SmsIdxUtil;
        util_->init();
        cpix_Result
            result;

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
        
        searcher_ = cpix_IdxSearcher_openDb(&result,
                                            SMS_QBASEAPPCLASS);
                                     
        if (searcher_ == NULL)
            {
                ITK_PANIC("Could not create idx searcher");
            }

        ss.setupComplete();
    }


    virtual void tearDown() throw()
    {
        cleanup();
    }


    virtual ~ParMSearcherSmsContext()
    {
        cleanup();
    }


    //
    // public operations
    //
    ParMSearcherSmsContext()
        : util_(NULL),
          searcher_(NULL),
          analyzer_(NULL),
          query_(NULL),          
          corpus_(DEFAULT_TEST_CORPUS_PATH),
          queryParser_(NULL)
    {
        ;
    }

    
private:
    //
    // private methods
    //
    void cleanup()
    {
        delete util_;
        util_ = NULL;

        cpix_IdxSearcher_releaseDb(searcher_);
        searcher_ = NULL;

        cpix_Analyzer_destroy(analyzer_);
        analyzer_ = NULL;

        cpix_Query_destroy(query_);
        query_ = NULL;

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;
    }
};




    /********
     * ParMSearcher Indexing Test Sequence
     *
     * One client is indexing and updating content while the other is
     * searching - and it should make no difference whatsoever.
     */
class ParMSearcherIdxSeq : public ParMSearcherSmsContext
{
private:
    size_t curId_;

public:

    virtual void setup() throw (Itk::PanicExc)
    {
        ParMSearcherSmsContext::setup();

        curId_ = 0;
    }


    void testAddSome(Itk::TestMgr * testMgr)
    {
        // the same function is invoked multiple times to add
        // more and more documents (in batches of 50)
        for (size_t i = 0; i < 50; ++i)
            {
                ++curId_;
                std::wstring
                    body = corpus_.item(curId_);
                util_->indexSms(curId_,
                                body.c_str(),
                                analyzer_,
                                testMgr);
            }
        util_->flush();
    }


    void testSearch(Itk::TestMgr * testMgr)
    {
        cpix_Hits
            * hits = cpix_IdxSearcher_search(searcher_,
                                             query_);

        if (cpix_Failed(searcher_))
            {
                ITK_EXPECT(testMgr,
                           false,
                           "Failed to search index");
                cpix_ClearError(searcher_);
            }
        else
            {
                util_->printHits(hits,
                                 testMgr);

                cpix_Hits_destroy(hits);
            }
    }

    
    void testUpdate(Itk::TestMgr * testMgr)
    {
        int32_t
            maxInsertBufSize = 32 * 1024; // 32 KB
	
        cpix_IdxDb_setMaxInsertBufSize(util_->idxDb(),
                                       maxInsertBufSize);
        ITK_EXPECT(testMgr,
                   cpix_Succeeded(util_->idxDb()),
                   "Failed to set max insert buffer size to %d",
                   maxInsertBufSize);

        util_->indexSms(23,
                        L"This UPDATED msg body does not have the h.appy word in it anymore",
                        analyzer_,
                        testMgr,
                        true); // update
        util_->indexSms(32,
                        L"This UPDATED msg body does have the happy word in it",
                        analyzer_,
                        testMgr,
                        true); // update

        for (int i = 0; i < 10; ++i)
            {
                util_->indexSms(40 + i,
                                L"Just to update couple of times, to fill up the insert buffer in update state too. Garble, gobledegook.",
                                analyzer_,
                                testMgr,
                                true); // update
            }
        util_->flush();
    }
};



Itk::TesterBase * CreateParMSearcherIdxSequence()
{
    using namespace Itk;

#define SEQUENCE "parmsearcheridx"

    ParMSearcherIdxSeq
        * parallelIdxSeq = new ParMSearcherIdxSeq;
    ContextTester
        * parallelIdxer = new ContextTester(SEQUENCE,
                                            parallelIdxSeq);

#define TEST "addSome1"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParMSearcherIdxSeq::testAddSome,
                       TEST);
#undef TEST

#define TEST "search1"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParMSearcherIdxSeq::testSearch,
                       TEST);
#undef TEST

#define TEST "addSome2"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParMSearcherIdxSeq::testAddSome,
                       TEST);
#undef TEST

#define TEST "search2"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParMSearcherIdxSeq::testSearch,
                       TEST);
#undef TEST

#define TEST "addSome3"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParMSearcherIdxSeq::testAddSome,
                       TEST);
#undef TEST

#define TEST "search3"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParMSearcherIdxSeq::testSearch,
                       TEST);
#undef TEST

#define TEST "addSome4"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParMSearcherIdxSeq::testAddSome,
                       TEST);
#undef TEST

#define TEST "search4"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParMSearcherIdxSeq::testSearch,
                       TEST);
#undef TEST

#define TEST "update"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParMSearcherIdxSeq::testUpdate,
                       TEST);
#undef TEST

#define TEST "search5"
    parallelIdxer->add(TEST,
                       parallelIdxSeq,
                       &ParMSearcherIdxSeq::testSearch,  // Same test func with 
                       TEST);                        // different name! :-)
#undef TEST

    return parallelIdxer;

#undef SEQUENCE
}




    /*******
     *
     */


Itk::TesterBase * CreatePartialSmsTests()
{
    using namespace Itk;

    SuiteTester
        * partialSmsTests = new SuiteTester(SUITE);

    partialSmsTests->add(CreateSimpleIdxSequence());
    partialSmsTests->add(CreateParallelIdxSequence());
    partialSmsTests->add(CreateMixedIdxSequence());
    partialSmsTests->add(CreateParMSearcherIdxSequence());

    // TODO add more tests to suite
        
    return partialSmsTests;
}


