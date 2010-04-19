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
#include <vector>

#include "itk.h"

#include "cpixjobqueue.h"

#define MSG(fmt, args...)  printf(fmt, ##args); fflush(stdout);

class Job
{
    int        id_;
    int        rounds_;

    bool       cancelled_;
    
public:
    Job(int id,
        int rounds)
        : id_(id),
          rounds_(rounds),
          cancelled_(false)
    {
        ;
    }


    int id() const
    {
        return id_;
    }
    

    void cancel(int id) // TODO OBS id
    {
        if (id != id_)
            {
                ITK_PANIC("Messed up cancel id");
            }

        cancelled_ = true;
        MSG("JOB %d - cancelled\n",
            id_);
    }

    
    void doJob()
    {
        MSG("Starting JOB %d - %d rounds\n",
            id_,
            rounds_);

        for (; rounds_ > 0 && !cancelled_; --rounds_)
            {
                sleep(1);

                MSG("JOB %d : %d rounds to go\n",
                    id_,
                    rounds_);
            }

        MSG("JOB %d - %s\n",
            id_,
            cancelled_ ? "CANCELLED" : "COMPLETED");
    }
    
};


typedef Cpt::JobQueue<Job*> TestJobQueue;

class JobVector : public std::vector<Job*>, public Cpt::SyncEntity
{
public:

    ~JobVector()
    {
        std::vector<Job*>::iterator
            i = begin(),
            e = end();

        for (; i != e; ++i)
            {
                delete *i;
            }
    }
};



struct ThreadParam
{
    TestJobQueue    * q_;
    JobVector       * v_;
    Itk::TestMgr    * testMgr_;
};


static void * producer(void * param)
{
    using namespace std;

    struct ThreadParam
        * p = reinterpret_cast<ThreadParam *>(param);

    MSG("Producer thread started\n");

    try
        {
            for (int i = 0; i < 10; ++i)
                {
                    std::auto_ptr<Job>
                        job(new Job(i,
                                    i));

                    { // SYNC
                        Cpt::SyncRegion
                            sr(*p->v_);

                        p->v_->push_back(job.get());
                        p->q_->put(job.get());
                    } // SYNC

                    job.release();

                    MSG("Producer thread created job #%d\n",
                        i);
                    
                    ITK_DBGMSG(p->testMgr_, ":");

                    sleep(1);
                }

            sleep(3);

            for (int i = 0; i < 5; ++i)
                {
                    { // SYNC
                        Cpt::SyncRegion
                            sr(*p->v_);

                        int
                            id = 2*i;

                        Job
                            * job = NULL;
                        bool
                            found = p->q_->findJob(compose1(bind2nd(equal_to<int>(),
                                                                    id),
                                                            mem_fun(&Job::id)),
                                                   &job);

                        if (found)
                            {
                                job->cancel(id);
                                MSG("Producer thread cancelled job #%d\n",
                                    id);
                            }
                        else
                            {
                                MSG("Started job was not found");
                            }

                        ITK_DBGMSG(p->testMgr_, "!");

                    } // SYNC
                }

            sleep(6);

            p->q_->stopProcessing();
            
        }
    catch (...)
        {
            MSG("Producer thread: cancelled\n");
        }

    MSG("Producer thread stopped\n");

    return NULL;
}



static void * consumer(void * param)
{
    struct ThreadParam
        * p = reinterpret_cast<ThreadParam *>(param);

    MSG("Consumer thread started\n");

    try
        {
            bool
                result = true;

            while (result)
                {
                    Job
                        * job = NULL;
                    
                    MSG("Consumer thread getting ...\n");

                    result = p->q_->get(&job);

                    MSG("Consumer thread has got (%d) ...\n",
                        result);
                    
                    ITK_DBGMSG(p->testMgr_, ".");

                    if (result)
                        {
                            job->doJob();
                            p->q_->jobCompleted(job);
                        }
                }
        }
    catch (...)
        {
            MSG("Consumer thread: cancelled\n");
        }

    MSG("Consumer thread stopped\n");

    return NULL;
}



void testCancel(Itk::TestMgr * testMgr)
{
    TestJobQueue
        jobQueue(10);
    JobVector
        jobVector;
    
    struct ThreadParam threadParam = 
        {
            &jobQueue,
            &jobVector,
            testMgr
        };

    std::list<pthread_t>
        threadHndls;

    int
        result;

    MSG("Creating 1 producer and 1 consumer\n");

    pthread_t
        threadHndl;
    
    result = pthread_create(&threadHndl,
                            NULL,
                            &producer,
                            &threadParam);
    ITK_ASSERT(testMgr,
               result == 0,
               "Could not create producer thread");
    
    threadHndls.push_back(threadHndl);
    
    result = pthread_create(&threadHndl,
                            NULL,
                            &consumer,
                            &threadParam);
    
    ITK_ASSERT(testMgr,
               result == 0,
               "Could not create consumer thread");

    threadHndls.push_back(threadHndl);

    MSG("joining threads\n");

    int
        jIdx = 0;

    std::list<pthread_t>::iterator
        i = threadHndls.begin(),
        end = threadHndls.end();
    for (; i != end; ++i)
        {
            void
                * retVal;

            MSG("j:%d\n",
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

    MSG("Joined all workers\n");
    MSG("session done.\n");
}



Itk::TesterBase * CreateJobQueueTests()
{
    using namespace Itk;

    SuiteTester
        * jobQueueTests = new SuiteTester("jobqueue");

#define TEST "cancel"
    jobQueueTests->add(TEST,
                       testCancel,
                       TEST,
                       SuiteTester::REDIRECT_ONLY);
#undef TEST


    // ... add more tests to suite
    
    return jobQueueTests;
    
}
