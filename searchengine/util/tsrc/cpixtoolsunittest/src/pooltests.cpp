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

#include <pthread.h>
#include <unistd.h>

#include <string>
#include <iostream>

#include "itk.h"

#include "cpixsyncpool.h"


namespace
{


    class TestPoolItemCounter
    {
    private:

        static TestPoolItemCounter  * instance_;


        size_t ctorInvocationCount_;
        size_t dtorInvocationCount_;
        size_t instanceCount_;

    public:

        static TestPoolItemCounter * instance()
        {
            if (instance_ == NULL)
                {
                    instance_ = new TestPoolItemCounter;
                }

            return instance_;
        }


        void ctor()
        {
            ++ctorInvocationCount_;
            ++instanceCount_;
        }

        void dtor()
        {
            ++dtorInvocationCount_;
            --instanceCount_;
        }


        void printStatus()
        {
            printf("TestPoolItem ctor %ld, dtor %ld, count %ld\n",
                   ctorInvocationCount_,
                   dtorInvocationCount_,
                   instanceCount_);
        }

        
        void reset()
        {
            ctorInvocationCount_ = 0;
            dtorInvocationCount_ = 0;
            instanceCount_ = 0;
        }


    private:
        TestPoolItemCounter()
            : ctorInvocationCount_(0),
              dtorInvocationCount_(0),
              instanceCount_(0)
        {
            ;
        }
    };



    TestPoolItemCounter * TestPoolItemCounter::instance_ = NULL;


    class TestPoolItem
    {
    public:
        TestPoolItem()
        {
            TestPoolItemCounter::instance()->ctor();
        }


        ~TestPoolItem()
        {
            TestPoolItemCounter::instance()->dtor();
        }
    };


    enum { ITEMCOUNT = 4 };


    void PrintStatus(TestPoolItem   ** items,
                     const char      * name = NULL)
    {
        TestPoolItemCounter::instance()->printStatus();

        int
            count = 0;

        for (int i = 0; i < ITEMCOUNT; ++i)
            {
                if (*(items + i) != NULL)
                    {
                        ++count;
                    }
            }
        

        if (name == NULL)
            {
                printf("Items in use: %d\n",
                       count);
            }
        else
            {
                printf("Thread %s: items in use: %d\n",
                       name,
                       count);
            }
    }


    struct ThreadParam
    {
        const char                        * name_;
        Itk::TestMgr                      * testMgr_;
        Cpt::SyncPool<TestPoolItem>       * pool_;
        TestPoolItem                     ** items_;
    };



    void ItemsArrayDeleter(void * p)
    {
        TestPoolItem
            ** items = reinterpret_cast<TestPoolItem**>(p);

        delete[] p;
    }
    

    pthread_key_t     NameKey;
    pthread_key_t     ItemsKey;


    void PrintStatus()
    {
        void
            * p = pthread_getspecific(ItemsKey);

        if (p == NULL)
            {
                printf("Cannot get thread specific key: ItemsKey\n");
                ITK_PANIC("ItemsKey");
            }

        TestPoolItem
            ** items = reinterpret_cast<TestPoolItem **>(p);

        p = pthread_getspecific(NameKey);

        const char
            * name = reinterpret_cast<const char*>(p);
        
        PrintStatus(items,
                    name);

        sleep(1);
    }



} // namespace





void testSingleThreadedUse(Itk::TestMgr * testMgr)
{
    using namespace Cpt;

    TestPoolItemCounter::instance()->reset();

    static int 
        minPoolItemCount = 2;

    SyncPool<TestPoolItem>
        pool(minPoolItemCount);

    TestPoolItem
        * items[ITEMCOUNT];
    for (int i = 0; i < ITEMCOUNT; ++i)
        {
            items[i] = NULL;
        }

    items[0] = pool.acquire();
    PrintStatus(items);

    items[1] = pool.acquire();
    PrintStatus(items);

    pool.release(items[0]);
    items[0] = NULL;
    PrintStatus(items);

    items[2] = pool.acquire();
    PrintStatus(items);

    items[3] = pool.acquire();
    PrintStatus(items);
    
    items[0] = pool.acquire();
    PrintStatus(items);

    pool.release(items[1]);
    items[1] = NULL;
    PrintStatus(items);
    
    pool.release(items[0]);
    items[0] = NULL;
    PrintStatus(items);

    pool.release(items[3]);
    items[3] = NULL;
    PrintStatus(items);

    pool.release(items[2]);
    items[2] = NULL;
    PrintStatus(items);
}



void * ThreadFunc(void * param)
{
    ThreadParam
        * p = reinterpret_cast<ThreadParam*>(param);

    int
        result = pthread_setspecific(NameKey,
                                     p->name_);
    if (result != 0)
        {
            printf("Could not set thread specific NameKey\n");
            ITK_PANIC("NameKey setting");
        }

    result = pthread_setspecific(ItemsKey,
                                 p->items_);

    if (result != 0)
        {
            printf("Could not set thread specific ItemsKey\n");
            ITK_PANIC("ItemsKey setting");
        }

    Cpt::SyncPool<TestPoolItem>
        & pool = *p->pool_;;

    TestPoolItem
        ** items = p->items_;
    for (int i = 0; i < ITEMCOUNT; ++i)
        {
            items[i] = NULL;
        }

    items[0] = pool.acquire();
    PrintStatus();

    items[1] = pool.acquire();
    PrintStatus();

    pool.release(items[0]);
    items[0] = NULL;
    PrintStatus();

    items[2] = pool.acquire();
    PrintStatus();

    items[3] = pool.acquire();
    PrintStatus();
    
    items[0] = pool.acquire();
    PrintStatus();

    pool.release(items[1]);
    items[1] = NULL;
    PrintStatus();
    
    pool.release(items[0]);
    items[0] = NULL;
    PrintStatus();

    pool.release(items[3]);
    items[3] = NULL;
    PrintStatus();

    pool.release(items[2]);
    items[2] = NULL;
    PrintStatus();

    printf("Thread %s is DONE.\n",
           p->name_);

    return NULL;
}




void testMultiThreadedUse(Itk::TestMgr * testMgr)
{
    TestPoolItemCounter::instance()->reset();

    int
        result = pthread_key_create(&NameKey,
                                    NULL);
    ITK_ASSERT(testMgr,
               result == 0,
               "Could not create thread specific key: NameKey");

    result = pthread_key_create(&ItemsKey,
                                ItemsArrayDeleter);
    ITK_ASSERT(testMgr,
               result == 0,
               "Could not create thread specific key: ItemsKey");
    
    static int 
        minPoolItemCount = 2;

    Cpt::SyncPool<TestPoolItem>
        pool(minPoolItemCount);

    TestPoolItem
        ** tpi1 = new TestPoolItem*[ITEMCOUNT],
        ** tpi2 = new TestPoolItem*[ITEMCOUNT];

    ThreadParam threadParams[2] = {
        {
            "main",
            testMgr,
            &pool,
            tpi1
        },

        {
            "extra",
            testMgr,
            &pool,
            tpi2
        }
    };

    pthread_t
        extra;

    result = pthread_create(&extra,
                            NULL,
                            &ThreadFunc,
                            threadParams + 1);

    ITK_ASSERT(testMgr,
               result == 0,
               "Could not create extra thread");
    
    ThreadFunc(threadParams + 0);

    void
        * retVal = NULL;

    printf("main: Joining extra\n");

    result = pthread_join(extra,
                          &retVal);

    ITK_ASSERT(testMgr,
               result == 0,
               "Could not join extra thread");

    printf("main: joined extra\n");
}




Itk::TesterBase * CreatePoolTests()
{
    using namespace Itk;

    SuiteTester
        * poolTests = new SuiteTester("pool");

#define TEST "singleThreadedUse"
    poolTests->add(TEST,
                   testSingleThreadedUse,
                   TEST);
#undef TEST


#define TEST "multiThreadedUse"
    poolTests->add(TEST,
                   testMultiThreadedUse,
                   TEST);
#undef TEST


    // ... add more tests to suite
    
    return poolTests;
}
