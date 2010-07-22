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

#include <memory>
#include <string.h>

#include "itk.h"

#include "cpixsynctools.h"


void testMutex(Itk::TestMgr * mgr)
{
    using namespace Cpt;

    Mutex
        mutex;

    SyncRegion
        sr(mutex);
}


void testRecursiveMutex(Itk::TestMgr * mgr)
{
    using namespace Cpt;

    Mutex
        mutex(true);

    SyncRegion
        sr1(mutex);

    SyncRegion
        sr2(mutex);
}


void testMultiSyncRegion(Itk::TestMgr * mgr)
{
    using namespace Cpt;

    Mutex
        mutex1,
        mutex2(true),
        mutex3;

    {
        MultiSyncRegion
            msr(4);
        
        msr.lock(mutex1);
        msr.lock(mutex2);
        msr.lock(mutex3);
        msr.lock(mutex2);
    }

    SyncRegion
        sr(mutex1);

    SyncRegion
        sr1(mutex2);

    SyncRegion
        sr2(mutex3);
}


void testSyncExc(Itk::TestMgr * mgr)
{
    pthread_mutex_t
        mutex;

    // kept uninitialized on purpose!

    try
        {
            Cpt::Impl::SyncRegion_::lockMutex(&mutex);

            ITK_EXPECT(mgr,
                       false,
                       "Locking uninitalized mutex should have failed");
        }
    catch (std::exception & exc)
        {
            printf("Locking unitialized mutex have failed (as should): %s\n",
                   exc.what());
        }

    try
        {
            Cpt::Impl::SyncRegion_::unlockMutex(&mutex);

            ITK_EXPECT(mgr,
                       false,
                       "Unlocking uninitalized mutex should have failed");
        }
    catch (std::exception & exc)
        {
            printf("Unlocking unitialized mutex have failed (as should): %s\n",
                   exc.what());
        }

    Cpt::Impl::SyncRegion_::initMutex(&mutex);

    try
        {
            Cpt::Impl::SyncRegion_::unlockMutex(&mutex);

            // TODO this should be ITK_EXPECT - why does NOT it fail?
            printf("Unlocking initialized but non-locked mutex should have failed\n");
        }
    catch (std::exception & exc)
        {
            printf("Unlocking initialized but non-locked mutex have failed (as should): %s\n",
                   exc.what());
        }

    try
        {
            {
                Cpt::Impl::SyncRegion_
                    sr(&mutex);
            }

            printf("Locked and unlocked mutex");
        }
    catch (std::exception & exc)
        {
            ITK_EXPECT(mgr,
                       false,
                       "Failed to lock and unlock mutex: %s",
                       exc.what());
        }

    Cpt::Impl::SyncRegion_::destroyMutex(&mutex);
}


/**
 * Test "SyncExc2" tries to create a situation where a thread that
 * does not own the lock tries to unlock the mutex (should fail).
 */
struct StupidUnlock
{
    pthread_mutex_t    * mutex_;
    Itk::TestMgr       * mgr_;
};


void * StupidThreadFunc(void * param)
{
    printf("# SyncExc2 / StupidThread BEGIN.\n");

    StupidUnlock
        * p = reinterpret_cast<StupidUnlock*>(param);

    try
        {
            Cpt::Impl::SyncRegion_::unlockMutex(p->mutex_);

            // TODO this should be ITK_EXPECT - why does NOT it fail?
            printf("# Unlocking mutex that is locked by another thread should fail (imho) - but it succeeds on both winscw and arm.\n");
        }
    catch (std::exception & exc)
        {
            printf("# Unlocking mutex owned by another thread have failed (as should): %s\n",
                   exc.what());
        }
    
    printf("# SyncExc2 / StupidThread END.\n");

    return NULL;
}


void testSyncExc2(Itk::TestMgr * mgr)
{
    pthread_mutex_t
        mutex;

    Cpt::Impl::SyncRegion_::initMutex(&mutex);

    {
        Cpt::Impl::SyncRegion_
            sr(&mutex);

        // here we create a thread that will try to unlock the mutex
        // held by this thread - it should fail
        StupidUnlock
            stupidUnlock = {
            &mutex,
            mgr
        };
        
        pthread_t
            stupidThread;
        
        int
            result = pthread_create(&stupidThread,
                                    NULL,
                                    &StupidThreadFunc,
                                    &stupidUnlock);
        
        ITK_ASSERT(mgr,
                   result == 0,
                   "Could not start stupid thread: %d",
                   result);
        
        printf("Sleep(1) ...\n");

        sleep(1);
        
        printf("... awake(1)\n");

        result = pthread_join(stupidThread,
                              NULL);
        
        printf("has joined stupid thread, unlocking mutex\n");
    }

    Cpt::Impl::SyncRegion_::destroyMutex(&mutex);

    printf("SyncExc2 test done.\n");
}



Itk::TesterBase * CreateMutexTests()
{
    using namespace Itk;

    SuiteTester
        * mutexTests = new SuiteTester("mutex");

#define TEST "mutex"
    mutexTests->add(TEST,
                    testMutex);
#undef TEST

#define TEST "recursivemutex"
    mutexTests->add(TEST,
                    testRecursiveMutex);
#undef TEST

#define TEST "multisyncregion"
    mutexTests->add(TEST,
                    testMultiSyncRegion);
#undef TEST

#define TEST "syncexc"
    mutexTests->add(TEST,
                    testSyncExc,
                    TEST);
#undef TEST

#define TEST "syncexc2"
    mutexTests->add(TEST,
                    testSyncExc2,
                    TEST);
#undef TEST


    // ... add more tests to suite
    
    return mutexTests;
    
}
