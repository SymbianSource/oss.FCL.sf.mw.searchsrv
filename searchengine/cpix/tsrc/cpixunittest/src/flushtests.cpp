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
#include "setupsentry.h"
#include "testcorpus.h"

#include "std_log_result.h"

class FlushContext : public Itk::ITestContext, public Cpt::IFileVisitor
{
private:
    SmsIdxUtil           * util_;
    cpix_Analyzer        * analyzer_;

    LineTestCorpusRef      corpus_;

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
        
        using namespace std;

        util_ = new SmsIdxUtil;
        util_->init();
        
        analyzer_ = cpix_CreateSimpleAnalyzer(&result);
        if (analyzer_ == NULL)
            {
                ITK_PANIC("Could not create analyzer");
            }

        ss.setupComplete();
    }


    virtual void tearDown() throw ()
    {
        cleanup();
    }


    FlushContext()
        : util_(NULL),
          analyzer_(NULL),
          corpus_(DEFAULT_TEST_CORPUS_PATH)
    {
        ;
    }


    virtual ~FlushContext()
    {
        cleanup();
    }
 


    //
    // from Cpt::IFileVisitor
    //
    virtual bool visitFile(const char * path)
    {
        bool
            goOn = true;

        printf("  # index file %s\n",
               path);

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



    void testEmptyIndex(Itk::TestMgr * testMgr)
    {
        char *xml_file = (char *)__FUNCTION__;
        assert_failed = 0;
        checkIndexFiles(testMgr);
        testResultXml(xml_file);
        
    }

    
    void testAddingBy512B(Itk::TestMgr * testMgr)
    {
        char *xml_file = (char *)__FUNCTION__;
        assert_failed = 0;
        setMaxInsertBufferSize(testMgr,
                         512); // 512 B buffer
        addItems(testMgr,
                 10);
        testResultXml(xml_file);
    }


    void testAddingBy10KB(Itk::TestMgr * testMgr)
    {
        char *xml_file = (char *)__FUNCTION__;
        assert_failed = 0;
        setMaxInsertBufferSize(testMgr,
                               10*1024); // 10 KB buffer
        addItems(testMgr,
                 40);
        testResultXml(xml_file);
    }


    void testFlushing(Itk::TestMgr * testMgr)
    {
        char *xml_file = (char *)__FUNCTION__;
        assert_failed = 0;
        addItems(testMgr,
                 15);
        flush(testMgr);
        checkIndexFiles(testMgr);
        testResultXml(xml_file);
    }


private:

    // utilities
    void checkIndexFiles(Itk::TestMgr * )
    {
        Cpt::traverse(DEFAULT_CPIX_DIR "indexing\\indexdb\\root\\msg\\phone\\sms",
                      this);
    }


    void addItems(Itk::TestMgr * testMgr,
                  size_t         num)
    {
        using namespace Itk;

        for (size_t i = 0; i < num; ++i)
            {
                std::wstring
                    body = corpus_.item(i);
                util_->indexSms(i,
                                body.c_str(),
                                analyzer_,
                                testMgr);
                printf("Added one more item\n");
                checkIndexFiles(testMgr);
            }
    }

    
    void setMaxInsertBufferSize(Itk::TestMgr * testMgr,
                                size_t         maxInsertBufferSize)
    {
        cpix_IdxDb_setMaxInsertBufSize(util_->idxDb(),
                                       maxInsertBufferSize);
        
        // casted to be printable with %d on device
        int trunkated = static_cast<int>(maxInsertBufferSize);
        
        ITK_ASSERT(testMgr,
                   cpix_Succeeded(util_->idxDb()),
                   "Could not set the maxInsertBufferSize to %d",
                   trunkated); 
        if(!cpix_Succeeded(util_->idxDb()))
            {
                assert_failed = 1;
            }
        printf("Set max insert buffer size to %d\n",
        	   trunkated);
    }

    
    void flush(Itk::TestMgr * testMgr)
    {
        cpix_IdxDb_flush(util_->idxDb());
        ITK_ASSERT(testMgr,
                   cpix_Succeeded(util_->idxDb()),
                   "Could not flush index database");
        if(!cpix_Succeeded(util_->idxDb()))
            {
                assert_failed = 1;
            }
        printf("Flushed index database\n");
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
    }


};



Itk::TesterBase * CreateFlushTests()
{
    using namespace Itk;

    FlushContext
        * flushContext = new FlushContext();
    ContextTester
        * flushTests = new ContextTester("flush",
                                         flushContext);

#define TEST "emptyIndex"
    flushTests->add(TEST,
                    flushContext,
                    &FlushContext::testEmptyIndex,
                    TEST);
#undef TEST


#define TEST "addingBy512B"
    flushTests->add(TEST,
                    flushContext,
                    &FlushContext::testAddingBy512B,
                    TEST);
#undef TEST

	#define TEST "addingBy10KB"
    flushTests->add(TEST,
                    flushContext,
                    &FlushContext::testAddingBy10KB,
                    TEST);
#undef TEST

#define TEST "flushing"
    flushTests->add(TEST,
                    flushContext,
                    &FlushContext::testFlushing,
                    TEST);
#undef TEST

    

    // TODO add more tests to suite

    return flushTests;
}
