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
#ifndef CPIXTOOLS_CPIXJOBQUEUE_H_
#define CPIXTOOLS_CPIXJOBQUEUE_H_

#include <algorithm>
#include <list>
#include <set>
#include <exception>


#include "cpixsyncqueue.h"


namespace Cpt
{

    

    /**
     * TODO update documentation - this is obsolete explanation.
     * 
     * A class for job queues. The additional feature to SyncQueue is
     * that it places the jobs taken out (by get()) to another set
     * called "startedJobs".) Meaning that the object itself can be
     * observed / used even after it has been taken out of the queue
     * itself for processing.
     *
     * For each job, the jobCompleted() call MUST BE made
     * eventually. At that point, the job will cease to be stored in
     * this JobQueue in any form. If there is some deallocation work
     * to be done, it is the responsibility of the client.
     *
     * Concept JOB:
     *
     *     default constructible, assignable, copy-constructible.
     *
     *
     * NOTE: JobQueue<JOB> does not derive from SyncQueue<JOB> simply
     * because ARM compiler cannot handle calling methods of template
     * base classes from template subclasses. Aggregation is used
     * instead of derivation.
     */
    template<typename JOB>
    class JobQueue
    {
    private:
        //
        // private members
        //
        SyncQueue<JOB>               q_;
        

        std::set<JOB>                startedJobs_;

    public:
        //
        // public operators
        //



        /**
         * @returns true if this queue is empty
         */
        bool empty();

        
        /**
         * @returns true if this queue is full
         */
        bool full();


        /**
         * Looks up a started job. (Jobs that are not started yet are
         * not taken into account.)
         *
         * @param jobId an argument by which to identify the job
         *
         * @param matcher is unary function that takes JOB argument
         * and returns bool. Must return true for an identified job
         * and false otherwise.

         * @param job the found job is put here. If nothing found, its
         * value is undefined.
         *
         * @return true if a matching job was found, false otherwise
         */
        template<typename PRED>
        bool findStartedJob(PRED       matcher,
                            JOB      * job);
        


        /** 
         * TODO comment
         */
        template<typename PRED>
        bool findJob(PRED       matcher,
                     JOB      * job);

        /** 
         * Finds  non-started job using matcher predicate.
         * Does not destroy job. 
         * 
         * @param matcher the matching function.
         *
         * @return true if a matching job was found and removed from 
         * this job queue, false otherwise. 
         */
        template<typename PRED>
        bool cancelNonStartedJob(PRED       matcher);
        
        /** 
         * In addition to taking the job from the job queue it puts
         * it to the set of started jobs, from where a call to
         * jobCompleted must take out.
         */
        bool get(JOB * job);



        /**
         * Puts a jobs into this jobqueue. If the queue is full, the
         * caller thread is blocked until there is some empty slot.
         */
        bool put(JOB job);



        /**
         * Producer thread (one of the producer threads) signals the
         * consumers that no items will be placed here, they should
         * stop using this queue.
         */
        void stopProcessing();


        /**
         * Tells this job queue that the given job is completed. It
         * must be called for every job that was obtained from a call
         * to get(), and it can be called only on jobs that have been
         * got (get()).
         */
        void jobCompleted(JOB job);


        //
        // lifetime management
        //
        /**
         * Constructs this JobQueue.
         *
         * @param maxSize the maximum number of jobs in the queue
         * waiting for processing.
         */ 
        JobQueue(size_t    maxSize);

      

        /**
         * Destructor
         */
        ~JobQueue();
    };



}





/************************************************
 *
 *   Implementation of template functions
 *
 */

namespace Cpt
{
    /////////////////////////////////////////////////////////
    //
    // JobQueue
    //
    //
    template<typename JOB>
    bool JobQueue<JOB>::empty()
    {
        return q_.empty();
    }


    template<typename JOB>
    bool JobQueue<JOB>::full()
    {
        return q_.full();
    }


    template<typename JOB>
    bool JobQueue<JOB>::get(JOB * job)
    { 
        bool
            rv = false;

        SyncRegion
            sr(q_);

        rv = q_.get2(job);

        if (rv)
            {
                startedJobs_.insert(*job);
            }

        return rv;
    }



    template<typename JOB>
    bool JobQueue<JOB>::put(JOB job)
    {
        return q_.put(job);
    }


    
    template<typename JOB>
    void JobQueue<JOB>::stopProcessing()
    {
        q_.stopProcessing();
    }


    template<typename JOB>
    void JobQueue<JOB>::jobCompleted(JOB job)
    { // SYNC
        SyncRegion
            sr(q_);

        startedJobs_.erase(job);
    } // SYNC



    template<typename JOB>
    JobQueue<JOB>::JobQueue(size_t maxSize)
        : q_(maxSize)
    {
        ;
    }



    template<typename JOB>
    JobQueue<JOB>::~JobQueue()
    {
        ;
    }



    template<typename JOB>
    template<typename PRED>
    bool JobQueue<JOB>::findStartedJob(PRED       matcher,
                                       JOB      * job)
    {
        using namespace std;
        
        SyncRegion
            sr(q_);
        
        typename set<JOB>::iterator
            found = find_if(startedJobs_.begin(),
                            startedJobs_.end(),
                            matcher);
        
        bool
            rv = found != startedJobs_.end();
        
        
        if (rv)
            {
                *job = *found;
            }
        
        return rv;
    }



    template<typename JOB>
    template<typename PRED>
    bool JobQueue<JOB>::findJob(PRED       matcher,
                                JOB      * job)
    {
        using namespace std;
        
        bool
            rv = false;

        SyncRegion
            sr(q_);
            
        typename list<JOB>::iterator
            found = find_if(q_.begin(),
                            q_.end(),
                            matcher);

        if (found != q_.end())
            {
                *job = *found;
                rv = true;
            }

        if (!rv)
            {
                typename set<JOB>::iterator
                    found = find_if(startedJobs_.begin(),
                                    startedJobs_.end(),
                                    matcher);

                rv = found != startedJobs_.end();
                    
                if (rv)
                    {
                        *job = *found;
                    }
            }
            
            
        return rv;
    }

    template<typename JOB>
    template<typename PRED>
    bool JobQueue<JOB>::cancelNonStartedJob(PRED       matcher)
    {
        using namespace std;
            
        bool
            rv = false;

        SyncRegion
            sr(q_);
                
        typename list<JOB>::iterator
            found = find_if(q_.begin(),
                            q_.end(),
                            matcher);
        
        if (found != q_.end())
            {
                q_.remove(found);
                rv = true;
            }
        
        return rv;
    }

}


#endif CPIXTOOLS_CPIXJOBQUEUE_H_
