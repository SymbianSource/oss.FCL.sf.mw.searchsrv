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
#ifndef CPIXTOOLS_CPIXSYNCQUEUE_H_
#define CPIXTOOLS_CPIXSYNCQUEUE_H_

#include <list>
#include <set>
#include <exception>


#include "cpixsynctools.h"


namespace Cpt
{


    /**
     * A synchronized queue that can be a rendez-vous point
     * between multiple producer and multiple consumer threads.
     */
    template<typename ITEM>
    class SyncQueue : public SyncEntity
    {
    public:
        //
        // public definitions
        //
        typedef ITEM            Item;


    protected:
        //
        // protected type definitions
        //
        typedef std::list<Item> ItemList;

    private:
        //
        // private members
        //
        size_t               maxSize_;
        ItemList             items_;
        bool                 keepProcessing_;

        // preventing C++ value semantics - declared but not defined
        SyncQueue(const SyncQueue &);

        // preventing C++ value semantics - declared but not defined
        SyncQueue & operator=(const SyncQueue &);


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
         * Gets the next item from the queue, if the queue is empty,
         * the caller thread is blocked (waiting) until there is item.
         *
         * @param item to copy the item taken out from the queue -
         * must not be NULL
         *
         * @return true if getting an item was successful, false if
         * this get operation was woken up from its blocking due to
         * forceful completion of operations (keepProcessing() ==
         * false). If the return value if false, do not attemp to use
         * the item pointed by param 'item', its value is undefined.
         */
        bool get(ITEM * item);


        /**
         * Puts an item into the queue. If the queue is full, the
         * caller thread is blocked (waiting) until there is free space.
         *
         * @param item the item to put in the queue.
         *
         * @return true if putting the item there was successful,
         * false if it was not, because the current thread was woken
         * up from its blocking operation with forceful completion of
         * operations (keepProcessing() == false).
         */
        bool put(ITEM item);


        /**
         * Producer thread (one of the producer threads) signals the
         * consumers that no items will be placed here, they should
         * stop using this queue.
         */
        void stopProcessing();


        //
        // lifetime management
        //

        /**
         * Constructs a synced queue.
         */
        SyncQueue(size_t maxSize);


        //
        // These methods here are protected conceptually, except ARM
        // compiler can not handle derivation with template classes
        // well
        //


        /**
         * Subclasses can iterate through items. WARNING: this
         * operation is not synchronized, it's the client that MUST
         * synchronize on this for the entire duration of iteration.
         */
        typename ItemList::iterator begin();


        /**
         * Subclasses can iterate through items. WARNING: this
         * operation is not synchronized, it's the client that MUST
         * synchronize on this for the entire duration of iteration.
         */
        typename ItemList::iterator end();

        /**
         * Removes the item pointed to by the iterator iter.  WARNING:
         * this operation is not synchronized, it's the client that
         * MUST synchronize on this for the entire duration of
         * iteration.
         */
        void remove(typename ItemList::iterator iter);


        /**
         * Same as get() above, except the client MUST lock on this
         * before calling this method (get() does it by itself).
         */
        bool get2(ITEM * item);


    private:
        //
        // private methods
        //
        /**
         * Client MUST lock on this before calling this method
         * 
         * @returns true if this queue is empty
         */
        bool empty2();

        
        /**
         * Client MUST lock on this before calling this method
         * 
         * @returns true if this queue is full
         */
        bool full2();
        
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
    // SyncQueue
    //
    //
    template<typename ITEM>
    bool SyncQueue<ITEM>::empty()
    {
        { // SYNC
            SyncRegion
                sr(*this);
        
            return empty2();

        } // SYNC
    }


    template<typename ITEM>
    bool SyncQueue<ITEM>::full()
    {
        { // SYNC
            SyncRegion
                sr(*this);
        
            return full2();

        } // SYNC
    }


    template<typename ITEM>
    bool SyncQueue<ITEM>::get(ITEM * item)
    {
        bool
            rv = false;

        { // SYNC
            SyncRegion
                syncedRegion(*this);

            rv = get2(item);

        } // SYNC
        
        return rv;
    }


    template<typename ITEM>
    bool SyncQueue<ITEM>::put(ITEM item)
    {
        bool
            rv = false;

        { // SYNC
            SyncRegion
                syncedRegion(*this);
        
            while (full2() && keepProcessing_) {
                wait();
            }

            if (keepProcessing_)
                {
                    items_.push_back(item);

                    signal();

                    rv = true;
                }

        } // SYNC

        return rv;
    }


    template<typename ITEM>
    void SyncQueue<ITEM>::stopProcessing()
    {
        { // SYNC
            SyncRegion
                syncedRegion(*this);
        
            keepProcessing_ = false;
            broadcast();
        } // SYNC
    }


    template<typename ITEM>
    SyncQueue<ITEM>::SyncQueue(size_t maxSize)
        : maxSize_(maxSize),
          keepProcessing_(true)
    {
        ;
    }


    template<typename ITEM>
    typename SyncQueue<ITEM>::ItemList::iterator SyncQueue<ITEM>::begin()
    {
        return items_.begin();
    }


    template<typename ITEM>
    typename SyncQueue<ITEM>::ItemList::iterator SyncQueue<ITEM>::end()
    {
        return items_.end();
    }

    template<typename ITEM>
    void SyncQueue<ITEM>::remove(typename ItemList::iterator iter)
    {
        items_.erase(iter);
    }


    template<typename ITEM>
    bool SyncQueue<ITEM>::get2(ITEM * item)
    {
        bool
            rv = false;

        while (empty2() && keepProcessing_) {
            // "this" must be locked at this point
            wait();
        }

        if (keepProcessing_)
            {
                *item = items_.front();
                items_.pop_front();

                signal();
                
                rv = true;
            }

        return rv;
    }


    template<typename ITEM>
    bool SyncQueue<ITEM>::empty2()
    {
        return items_.empty();
    }


    template<typename ITEM>
    bool SyncQueue<ITEM>::full2()
    {
        return items_.size() == maxSize_;
    }


}


#endif CPIXTOOLS_CPIXSYNCQUEUE_H_
