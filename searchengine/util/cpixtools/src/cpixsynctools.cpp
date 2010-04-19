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

#include <errno.h>

#include "cpixsynctools.h"

namespace Cpt
{



    //
    //
    //  SyncException
    //
    //
    const char * SyncExc::what() const throw()
    {
        return what_.c_str();
    }
    


    SyncExc::SyncExc(const char * format,
                     ...)
    {
        char
            msg[96];

        va_list
            args;
        va_start(args,
                 format);

        vsnprintf(msg,
                  sizeof(msg),
                  format,
                  args);

        va_end(args);

        what_ = msg;
    }



    namespace Impl
    {
        //
        //
        //  SyncRegion_
        //
        //
        void SyncRegion_::lockMutex(pthread_mutex_t * mutex)
        {
            int
                result;

            do {
                result = pthread_mutex_lock(mutex);
            } while (result == ETIMEDOUT);

            if (result != 0)
                {
                    mutex = NULL;

                    throw SyncExc("Could not lock mutex (error: %d)",
                                  result);
                }
        }


        
        void SyncRegion_::lockMutex(pthread_mutex_t * mutex,
                                    Recursive       * recursive)
        {
            if (recursive == NULL)
                {
                    lockMutex(mutex);
                }
            else
                {
                    pthread_t
                        curThread = pthread_self();

                    if (pthread_equal(recursive->lockOwner_, curThread))
                        {
                            ++(recursive->lockCount_);
                        }
                    else
                        {
                            lockMutex(mutex);
                            recursive->lockOwner_ = curThread;
                            recursive->lockCount_ = 1;
                        }
                }
        }
        


        void SyncRegion_::unlockMutex(pthread_mutex_t * mutex)
        {
            if (mutex != NULL)
                {
                    int 
                        result = pthread_mutex_unlock(mutex);
                    if (result != 0)
                        {
                            throw SyncExc("Could not unlock mutex (error: %d)",
                                          result);
                        }
                    mutex = NULL;
                }
        }


        void SyncRegion_::unlockMutex(pthread_mutex_t * mutex,
                                      Recursive       * recursive)
        {
            if (recursive == NULL)
                {
                    unlockMutex(mutex);
                }
            else
                {
                    --(recursive->lockCount_);
                    
                    if (recursive->lockCount_ == 0)
                        {
                            recursive->lockOwner_ = 0;
                            unlockMutex(mutex);
                        }
                }
        }


        void SyncRegion_::initMutex(pthread_mutex_t * mutex)
        {
            int
                result = pthread_mutex_init(mutex,
                                            NULL);
            if (result != 0)
                {
                    throw SyncExc("Could not create mutex (error: %d)",
                                  result);
                }
        }


        void SyncRegion_::destroyMutex(pthread_mutex_t * mutex_)
        {
            if (mutex_ != NULL)
                {
                    int
                        result = 0;

                    do
                        {
                            result = pthread_mutex_destroy(mutex_);
                            switch (result)
                                {
                                case EAGAIN:
                                    // if the mutex is held by another
                                    // thread - we try locking (to
                                    // avoid busy polling) and then
                                    // unlock it immediately
                                    lockMutex(mutex_);
                                    unlockMutex(mutex_);
                                    break;
                                case 0:       // OK
                                    break;
                                case EBUSY:
                                    throw SyncExc("Failed to destroy mutex - still locked by this thread (error: %d, EBUSY)",
                                                  result);
                                   
                                case EINVAL:  // invalid mutex
                                    throw SyncExc("Failed to destroy mutex - invalid (error: %d, EINVAL)",
                                                  result);
                                   
                                default:
                                    throw SyncExc("Failed to destroy mutex (error: %d, unknown reason)",
                                                  result);
                                }
                        } while (result != 0);
                }
        }


        SyncRegion_::SyncRegion_(pthread_mutex_t * mutex)
            : mutex_(mutex),
              recursive_(NULL)
        {
            lockMutex(mutex_);
        }


        SyncRegion_::SyncRegion_(pthread_mutex_t * mutex,
                                 Recursive       * recursive)
            : mutex_(mutex),
              recursive_(recursive)
        {
            lockMutex(mutex_,
                      recursive_);
        }


        SyncRegion_::~SyncRegion_()
        {
            bool
                failed = true;

            try
                {
                    unlockMutex(mutex_,
                                recursive_);
                    failed = false;
                }
            catch (SyncExc & e)
                {
                    printf("PANIC Cpt::Impl::SyncRegion_::~SyncRegion_ : %s\n",
                           e.what());
                    fflush(stdout);
                }

            if (failed)
                {
                    exit(-1);
                }
        }
    }



    
    //
    //
    //  Mutex
    //
    //
    Mutex::Mutex()
        : recursive_(NULL)
    {
        init(false);
    }


    Mutex::Mutex(bool recursive)
        : recursive_(NULL)
    {
        init(recursive);
    }


    Mutex::~Mutex()
    {

        if (recursive_ != NULL)
            {
                delete recursive_;
            }

        bool
            failed = true;

        try
            {
                Impl::SyncRegion_::destroyMutex(&mutex_);
                failed = false;
            }
        catch (SyncExc & e)
            {
                printf("PANIC Cpt::Mutex::~Mutex : %s\n",
                       e.what());
                fflush(stdout);
            }

        if (failed)
            {
                exit(-1);
            }
    }


    // TODO it is unclear if the OpenC layer has bug wrt recursive
    // mutexes - may be better to reimplement them manually here
    void Mutex::init(bool recursive)
    {
        if (recursive)
            {
                recursive_ = new Impl::Recursive;
            }

        Impl::SyncRegion_::initMutex(&mutex_);
    }


    //
    //
    //  SyncEntity
    //
    //
    void SyncEntity::wait()
    {
        int
            result;

        do {
            result = pthread_cond_wait(&cond_, 
                                       &mutex_);
        } while (result == ETIMEDOUT);

        if (result != 0)
            {
                throw SyncExc("Could not wait on cond (error: %d)",
                              result);
            }
    }


    void SyncEntity::signal()
    {
        int
            result = pthread_cond_signal(&cond_);
        if (result != 0)
            {
                throw SyncExc("Could not signal on cond (error: %d)",
                              result);
            }
    }


    void SyncEntity::broadcast()
    {
        int
            result = pthread_cond_broadcast(&cond_);
        if (result != 0)
            {
                throw SyncExc("Could not broadcast on cond (error: %d)",
                              result);
            }
    }


    SyncEntity::SyncEntity()
    {
        int
            result = pthread_cond_init(&cond_,
                                       NULL);
        if (result != 0)
            {
                throw SyncExc("Could not create cond (error: %d)",
                              result);
            }
    }


    SyncEntity::~SyncEntity()
    {
        int
            result = 0;

        { // SYNC
            Impl::SyncRegion_
                syncedRegion(&mutex_);
            
            // we need to wake up all threads waiting on our cond
            // variable cond_
            broadcast();
            
            // all threads waiting on cond_ have been
            // waken and they can't possibly re-enter
            // waiting because they would need the lock on
            // mutex_ which we are holding
            result = pthread_cond_destroy(&cond_);

            if (result != 0)
                {
                    printf("PANIC Cpt::SyncEntity::~SyncEntity (cond): %d\n",
                           result);
                    fflush(stdout);
                    exit(-1);
                }
        } // SYNC
    }


    //
    //
    //  SyncRegion
    //
    //
    SyncRegion::SyncRegion(Mutex & mutex)
        : SyncRegion_(&mutex.mutex_,
                      mutex.recursive_)
    {
        ;
    }


    SyncRegion::SyncRegion(SyncEntity & entity)
        : SyncRegion_(&entity.mutex_)
    {
        ;
    }


    SyncRegion::~SyncRegion()
    {
        ;
    }



    //
    //
    //  MultiSyncRegion
    //
    //
    void MultiSyncRegion::lock(Mutex & mutex)
    {
        throwIfFull();

        new (next_) SyncRegion(mutex);

        ++next_;
    }


    void MultiSyncRegion::lock(SyncEntity & syncEntity)
    {
        throwIfFull();

        new (next_) SyncRegion(syncEntity);

        ++next_;
    }


    MultiSyncRegion::MultiSyncRegion(size_t maxSize)
        : maxSize_(maxSize),
          syncers_(NULL),
          next_(NULL)
          
    {
        void
            * p = malloc(maxSize_ * sizeof(SyncRegion));

        if (p == NULL)
            {
                throw std::bad_alloc();
            }

        syncers_ = reinterpret_cast<SyncRegion*>(p);
        next_ = syncers_;
    }


    MultiSyncRegion::~MultiSyncRegion()
    {
        while (next_ > syncers_)
            {
                --next_;

                next_->~SyncRegion();
            }
        free(syncers_);
    }


    void MultiSyncRegion::throwIfFull()
    {
        if ((next_ - syncers_) == maxSize_)
            {
                throw SyncExc("MultiSyncRegion instance is full (max size: %d)",
                              maxSize_);
            }
    }

}
