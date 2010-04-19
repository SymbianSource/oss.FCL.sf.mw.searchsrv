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
#ifndef CPIXTOOLS_CPIXSYNCTOOLS_H_
#define CPIXTOOLS_CPIXSYNCTOOLS_H_

#include <pthread.h>

#include <exception>
#include <new>
#include <string>

namespace Cpt
{

    /**
     * Class to signal sync related error conditions.
     */
    class SyncExc : public std::exception
    {
        //
        // private members
        //
        std::string     what_;

    public:
        //
        // public operators
        //

        /**
         * Overriden from std::exception
         */
        virtual const char * what() const throw();


        //
        // Lifecycle management
        //

        /**
         * Constructs this exception with printf style format string
         * and potential detail arguments.
         */
        SyncExc(const char * format,
                ...);
    };



    // private namespace
    namespace Impl
    {

        /**
         * A simple structure to store information necessary for
         * recursive locking.
         */
        struct Recursive
        {
            pthread_t           lockOwner_;
            size_t              lockCount_;
        };



        /**
         * To protect regions with a mutex. Just construct it giving it a
         * valid mutex. On construction, it locks it and on destruction it
         * unlocks it. Lower level locking primitive class.
         */
        class SyncRegion_
        {
        private:
            //
            // private members
            //
            pthread_mutex_t     * mutex_;
            Recursive           * recursive_;

            // preventing C++ value semantics - declared but not defined
            SyncRegion_(const SyncRegion_ &);

            // preventing C++ value semantics - declared but not defined
            SyncRegion_ & operator=(const SyncRegion_ &);
        

        public:
            //
            // public operations
            //

            /**
             * Locks the given mutex. Not advised to use directly, as
             * unlocking must be guaranteed then by the client. Use
             * class SyncRegion RAII instead.
             *
             * @param mutex the mutex to lock.
             */
            static void lockMutex(pthread_mutex_t * mutex);


            /**
             * Recursively locks a mutex (manual implementation). Must
             * also use the recursive unlock() in pair.
             */
            static void lockMutex(pthread_mutex_t * mutex,
                                  Recursive       * recursive);


            /**
             * Unlocks the given mutex. Not advised to use directly.
             *
             * @param mutex the mutex to unlock
             */
            static void unlockMutex(pthread_mutex_t * mutex);


            /**
             * Unlocks the mutex recursively (manual
             * implementation). Must be used for a mutex that was
             * recursively locked.
             */
            static void unlockMutex(pthread_mutex_t * mutex,
                                    Recursive       * recursive);


            /**
             * Initialized the mutex.
             *
             * @param mutex - pointer to the mutex struct to
             * initialize
             */
            static void initMutex(pthread_mutex_t * mutex);

            /**
             * Destroys the given mutex.
             *
             * @param mutex the mutex to destroy.
             */
            static void destroyMutex(pthread_mutex_t * mutex);


            /**
             * Constructs this "sentry", locking the given mutex.
             */
            SyncRegion_(pthread_mutex_t * mutex);


            /**
             * Constructs this "sentry", recursively locking the given
             * mutex.
             */
            SyncRegion_(pthread_mutex_t * mutex,
                        Recursive       * recursive);


            /**
             * Destructs this "sentry", unlocking the mutex.
             */
            ~SyncRegion_();
        }; // class
       
    } // namespace 



    /**
     * A class that can be used for locking. Implemented with pthread
     * mutex. It does not have any operations on its own (beside
     * construction and destruction), one must use SyncRegion to
     * lock/unlock it.
     */
    class Mutex
    {
    protected:
        //
        // protected members
        //
        pthread_mutex_t         mutex_;


    private:
        //
        // private members
        //

        // preventing C++ value semantics - declared but not defined
        Mutex(const Mutex &);
        
        // preventing C++ value semantics - declared but not defined
        Mutex & operator=(const Mutex &);

        Impl::Recursive       * recursive_;
        

    public:
        //
        // public operators
        //

        //
        // lifetime management
        //
        /**
         * Constructs this mutex (initializes pthread mutex) - non
         * recursive.
         */
        Mutex();


        /**
         * Constructs this mutex (initializes pthread mutex).
         *
         * @param whether to allow recursive locking.
         */
        Mutex(bool recursive);


        /**
         * Destroys this mutex (destroys pthread mutex). The pthread
         * mutex is acquired before destroying.
         */
        ~Mutex();


        /**
         * One uses SyncRegion to lock this mutex.
         */
        friend class SyncRegion;


    private:
        //
        // private
        //
        void init(bool recursive);

        friend class SyncRegion;
    };



    /**
     * A base class for any class that needs synchronization
     * primitives.
     */
    class SyncEntity : public Mutex
    {
    private:
        //
        // private members
        //
        pthread_cond_t       cond_;

        // preventing C++ value semantics - declared but not defined
        SyncEntity(const SyncEntity &);

        // preventing C++ value semantics - declared but not defined
        SyncEntity & operator=(const SyncEntity &);

    public:
        //
        // public operators
        //

        /**
         * Wait on this object (cond_ and mutex_).
         */
        void wait();


        /**
         * Signal a thread waiting on this object.
         */
        void signal();


        /**
         * Broadcasts (signals) to all waiting threads on this object.
         */
        void broadcast();


        //
        // lifetime management
        //
        /**
         * Constructor
         */
        SyncEntity();


        /**
         * Destructor. The pthread mutex of this synchronized entity is
         * acquired before destroying it.
         */
        virtual ~SyncEntity();


        /**
         * One uses SyncRegion to lock this synchronized entity.
         */
        friend class SyncRegion;
    };
    


    /**
     * A class to use confortably to lock on SyncEntities.
     */
    class SyncRegion : private Impl::SyncRegion_
    {
    private:
        //
        // private members
        //

    public:
        //
        // public operations
        //

        /**
         * Constructor - locks the mutex.
         */
        SyncRegion(Mutex & mutex);


        /**
         * Constructor - locks the mutex of the target entity.
         */
        SyncRegion(SyncEntity & entity);


        /**
         * Destructor - unlocks the mutex of the target entity.
         */
        ~SyncRegion();
    };
    


    /**
     * A class that can lock multiple mutexes and release them all at
     * its destruction (in the reverse order of their locking).
     */
    class MultiSyncRegion
    {
    private:
        //
        // private members
        //
        /**
         * Maximum number of SyncRegion_ instances managed here
         */
        size_t              maxSize_;

        /**
         * Array of SyncRegion_ size bytes of maxSize.
         */
        SyncRegion        * syncers_;

        /**
         * Pointer to the next empty (unconstructed) SyncRegion_ slot.
         */
        SyncRegion        * next_;



    public:
        //
        // public operators
        //
        /**
         * Locks the given mutex (adds it to the number of
         * mutexes/sync entities already locked by this).
         */
        void lock(Mutex & mutex);


        /**
         * Locks the given syncEntity (adds it to the number of
         * mutexes/sync entities already locked by this).
         */
        void lock(SyncEntity & syncEntity);


        //
        // lifetime management
        //
        /**
         * Constructor - creates a MultiSyncRegion that can lock up to
         * a certain number of mutexes.
         *
         * @param maxSize maximal number of mutexes this
         * MultiSyncRegion will be able to lock.
         */
        MultiSyncRegion(size_t maxSize);


        /**
         * Destructor - releases all the locks (if any) on any mutexes
         * that have been locked by this instance.
         */
        ~MultiSyncRegion();


    private:
        /**
         * Checks if this MultiSyncRegion instance has filled up its
         * array pointed by syncers_, and if so, throws an exception.
         */
        void throwIfFull();
    };




} // namespace


#endif CPIXTOOLS_CPIXSYNCTOOLS_H_
