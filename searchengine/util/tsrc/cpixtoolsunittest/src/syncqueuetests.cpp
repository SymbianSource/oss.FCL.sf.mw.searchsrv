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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <list>

#include "itk.h"

#include "cpixsynctools.h"
#include "cpixsyncqueue.h"


#define LENIENT_STR "#IGNORE "

#define MSG(fmt, args...)  printf(fmt, ##args); fflush(stdout);


typedef Cpt::SyncQueue<int>    IntQueue;


struct ProducerParam
{
    IntQueue                 * queue_;
    int                        maxCount_;
    int                        maxSleepMs_;
    Itk::TestMgr             * testMgr_;
};


void * producer(void * param)
{
    struct ProducerParam
        const * p = reinterpret_cast<ProducerParam const*>(param);

    int
        ms = rand();
    ms = ms % p->maxSleepMs_;

    enum 
    { 
        MSEC_PER_SEC = 1000,
        NSEC_PER_MSEC = 1000000
    };

    struct timespec 
        tim;
    tim.tv_sec = ms / MSEC_PER_SEC;
    tim.tv_nsec = ms % MSEC_PER_SEC * NSEC_PER_MSEC;
            
    MSG("[B:%d] ",
        ms);

    try
        {
            bool
                goOn = true;

            for (int i = 0; 
                 i <= p->maxCount_ && goOn; 
                 ++i)
                {
                    
                    nanosleep(&tim,
                              NULL);
                    goOn = p->queue_->put(i);
                    MSG(">%d ",
                           i);
                    ITK_DBGMSG(p->testMgr_, ":");
                }
        }
    catch (Cpt::SyncExc & exc)
        {
            // we should not get exceptions
            MSG("\nSync exception in producer ");
        }

    MSG("\n" LENIENT_STR "[E] ");

    return NULL;
}


/**
 * A simple integer threads can add to in a synchronized way.
 */
class SyncSum : public Cpt::SyncEntity
{
    int        sum_;
public:
    SyncSum()
        : sum_(0)
    {
        ;
    }

    
    int sum()
    {
        { // SYNC
            Cpt::SyncRegion
                sr(*this);
            
            return sum_;

        } // SYNC
    }


    void inc(int delta)
    {
        { // SYNC
            Cpt::SyncRegion
                sr(*this);
            
            sum_ += delta;

        } // SYNC
    }
};


struct ConsumerParam
{
    IntQueue                 * queue_;
    SyncSum                  * sum_;
    Itk::TestMgr             * testMgr_;
};


void * consumer(void * param)
{
    struct ConsumerParam
        const * p = reinterpret_cast<ConsumerParam const *>(param);

    MSG("(B) ");

    bool
        x = false;

    try
        {
            bool
                goOn = true;

            while (goOn)
                {
                    MSG(x ? "xB " : ". ");
                    int
                        i;
                    goOn = p->queue_->get(&i);
                    if (goOn)
                        {
                            p->sum_->inc(i);
                            MSG("<%d ",
                                   i);
                        }
                    else
                        {
                            MSG("\n#IGNORE !X ");
                            x = true;
                        }
                    MSG(x ? "xA " : ". ");
                    ITK_DBGMSG(p->testMgr_, ".");
                }
        }
    catch (Cpt::SyncExc & exc)
        {
            // we should not get exceptions
            MSG("\nSync exception in consumer ");
        }

    MSG("\n" LENIENT_STR "(E) ");

    return NULL;
}



void testMultiThreadSession(Itk::TestMgr * testMgr,
                            int            count,
                            int            mainSleepSec)
{
    IntQueue
        queue(5);
    SyncSum
        sum;
    
    struct ProducerParam
        producerParam = {
        &queue,
        10,  // max count
        1100,    // max sleep millisecs
        testMgr
    };

    struct ConsumerParam
        consumerParam = {
        &queue,
        &sum,
        testMgr
    };

    std::list<pthread_t>
        threadHndls;

    int
        result;

    MSG("Creating %d producers and %d consumers\n",
        count,
        count);

    MSG(LENIENT_STR);

    for (int i = 0; i < count; ++i)
        {
            pthread_t
                threadHndl;

            result = pthread_create(&threadHndl,
                                    NULL,
                                    &producer,
                                    &producerParam);
            ITK_ASSERT(testMgr,
                       result == 0,
                       "Could not create producer thread %d",
                       2*i);
            
            threadHndls.push_back(threadHndl);

            result = pthread_create(&threadHndl,
                                    NULL,
                                    &consumer,
                                    &consumerParam);

            ITK_ASSERT(testMgr,
                       result == 0,
                       "Could not create consumer thread %d",
                       2*i + 1);

            threadHndls.push_back(threadHndl);
        }

    result = sleep(mainSleepSec);

    MSG("forcing stop ");
            
    queue.stopProcessing();
            
    MSG("forced stop ");

    MSG("joining threads ");

    int
        jIdx = 0;

    std::list<pthread_t>::iterator
        i = threadHndls.begin(),
        end = threadHndls.end();
    for (; i != end; ++i)
        {
            void
                * retVal;

            MSG("j:%d ",
                jIdx);

            result = pthread_join(*i,
                                  &retVal);
            ITK_EXPECT(testMgr,
                       result == 0,
                       "Failed to join %s thread %d",
                       ((jIdx % 2) == 0 ? "producer" : "consumer"),
                       jIdx);

            ++jIdx;
        }

    MSG("\nJoined all workers\n");
    MSG("\n" LENIENT_STR "SUM: %d\n",
        sum.sum());
    MSG("\n session done.");
}



void testMTEarlyStop1(Itk::TestMgr * testMgr)
{
    testMultiThreadSession(testMgr,
                           1,     // count
                           2);     // sleep
}


void testMTEarlyStop2(Itk::TestMgr * testMgr)
{
    testMultiThreadSession(testMgr,
                           2,     // count
                           2);     // sleep
}


void testMTEarlyStop5(Itk::TestMgr * testMgr)
{
    testMultiThreadSession(testMgr,
                           5,     // count
                           2);     // sleep
}


void testMTLateStop1(Itk::TestMgr * testMgr)
{
    testMultiThreadSession(testMgr,
                           1,     // count
                           12);     // sleep
}


void testMTLateStop2(Itk::TestMgr * testMgr)
{
    testMultiThreadSession(testMgr,
                           2,     // count
                           12);    // sleep
}


void testMTLateStop5(Itk::TestMgr * testMgr)
{
    testMultiThreadSession(testMgr,
                           5,     // count
                           12);     // sleep
}





Itk::TesterBase * CreateSyncQueueTests()
{
    using namespace Itk;

    SuiteTester
        * syncQueueTests = new SuiteTester("syncqueue");

#define TEST "earlyStop1"
    syncQueueTests->add(TEST,
                        testMTEarlyStop1,
                        TEST,
                        LENIENT_STR);
#undef TEST

#define TEST "earlyStop2"
    syncQueueTests->add(TEST,
                        testMTEarlyStop2,
                        TEST,
                        LENIENT_STR);
#undef TEST

#define TEST "earlyStop5"
    syncQueueTests->add(TEST,
                        testMTEarlyStop5,
                        TEST,
                        LENIENT_STR);
#undef TEST

#define TEST "lateStop1"
    syncQueueTests->add(TEST,
                        testMTLateStop1,
                        TEST,
                        LENIENT_STR);
#undef TEST

#define TEST "lateStop2"
    syncQueueTests->add(TEST,
                        testMTLateStop2,
                        TEST,
                        LENIENT_STR);
#undef TEST

#define TEST "lateStop5"
    syncQueueTests->add(TEST,
                        testMTLateStop5,
                        TEST,
                        LENIENT_STR);
#undef TEST

    // ... add more tests to suite
    
    return syncQueueTests;
    
}
