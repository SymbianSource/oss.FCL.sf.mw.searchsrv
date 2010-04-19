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
#ifndef CPIXTOOLS_CPIXSYNCPOOL_H_
#define CPIXTOOLS_CPIXSYNCPOOL_H_

#include <list>
#include <set>

#include "cpixsynctools.h"


namespace Cpt
{

    /**
     * A template class for pooling items. There is always a given
     * number of items kept around (either free or used), and clients
     * can ask for more items, in which case those items are created
     * on demand.
     *
     * NOTE: clients should always release the items back to the pool
     * the moment they don't need them anymore, otherwise it will
     * behave like a memory leak (even if it is technically not a
     * memory leak).
     *
     * ITEM: must be default constructible and must have a public
     * destructor.
     */
    template<typename ITEM>
    class SyncPool
    {
    private:
        //
        // private members
        //
        /**
         * Stores the items that are free (at most minItemCount_ of
         * them).
         */
        std::list<ITEM*>     freeItems_;

        /**
         * Stores the items that are used by clients.
         */
        std::set<ITEM*>     usedItems_;

        const size_t       minItemCount_;

        /**
         * Protects freeItems_ and usedItems_ both.
         */
        Mutex              mutex_;

    public:
        //
        // public operators
        //
        /**
         * Gets an item from the pool (either an existing unused one
         * or a newly created one). Must be release()-ed
         * eventually. Thread-safe implementation.
         */
        ITEM * acquire();


        /**
         * Releases an item back to the pool. The item itself may
         * actually be destroyed, or just stored for future use, but
         * the releasing client must not use it anymore. Thread-safe
         * implementation. If the item being released has already been
         * released or is unknown to the SyncPool, an exception is
         * thrown (SyncExc).
         */
        void release(ITEM * item);


        //
        // lifetime management
        //
        /**
         * Constructs this item pool.
         *
         * @param minItemCount the number of items that will be kept
         * around. This constructor does not actually create that many
         * items up front, only on demand.
         */
        SyncPool(size_t minItemCount);


        /**
         * Destructor.
         */
        ~SyncPool();
    };

}




//////////////////////////////////////////////////
//
//
//  I M P L E M E N T A T I O N
//
//
namespace Cpt
{

    template<typename ITEM>
    ITEM * SyncPool<ITEM>::acquire()
    {
        SyncRegion
            sr(mutex_);

        ITEM
            * rv = NULL;

        if (!freeItems_.empty())
            {
                rv = freeItems_.front();
                freeItems_.pop_front();
            }
        else
            {
                rv = new ITEM;
            }

        usedItems_.insert(rv);

        return rv;
    }



    template<typename ITEM>
    void SyncPool<ITEM>::release(ITEM * item)
    {
        SyncRegion
            sr(mutex_);

        if (usedItems_.find(item) == usedItems_.end())
            {
                throw SyncExc("Releasing unknown item to SyncPool");
            }

        usedItems_.erase(item);

        if (freeItems_.size() == minItemCount_)
            {
                delete item;
            }
        else
            {
                freeItems_.push_back(item);
            }
    }



    template<typename ITEM>
    SyncPool<ITEM>::SyncPool(size_t minItemCount)
        : minItemCount_(minItemCount)
    {
        ;
    }

    

    template<typename ITEM>
    SyncPool<ITEM>::~SyncPool()
    {
        using namespace std;

        SyncRegion
            sr(mutex_);

        // AR: Used on shutdown to test, whether pool items usages have leaked
        if (!usedItems_.empty())
            {
                throw SyncExc("Destroying SyncPool that has used items still");
            }

        { // free items
            typename list<ITEM*>::iterator
                i = freeItems_.begin(),
                end = freeItems_.end();
            
            for (; i != end; ++i)
                {
                    ITEM
                        * tmp = *i;
                    delete tmp;
                }

            freeItems_.clear();
        }

 
        { // used items
            typename set<ITEM*>::iterator
                i = usedItems_.begin(),
                end = usedItems_.end();
            
            for (; i != end; ++i)
                {
                    ITEM
                        * tmp = *i;
                    delete tmp;
                }

            usedItems_.clear();
        }
           
    }


}


#endif CPIXTOOLS_CPIXSYNCPOOL_H_
