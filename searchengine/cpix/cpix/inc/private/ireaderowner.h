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

#ifndef CPIX_IREADEROWNER_H
#define CPIX_IREADEROWNER_H

#include "CLucene.h"

#include "cpixsynctools.h"

#include "fwdtypes.h"


namespace Cpix
{

    /**
     * A temporary interface to abstract the aspect of IdxDb classe
     * needed by MultiIdxDb. Namely: to provide its version number and
     * to provide its reader - while being locked.
     *
     * Both implementations of this interface:
     *
     * (a) own an IndexReader instance
     *
     * (b) have a version that always reflect the IndexReader instance
     *     state / entity changes - this is to be able to track when
     *     cliens to the reader have been invalidated by reader
     *     changes
     *
     * (c) owns a mutex protecting the reader instance
     *
     * Below, the expression "locking this" means "locking the mutex
     * owned by this instance relevant to the reader state
     * protection".
     */
    class IReaderOwner
    {
    public:
        //
        // public operators
        //
        
        /**
         * Provides the version number of the current reader state,
         * after it has "locked itself" using msr.
         *
         * @param msr the multi sync region to use for "locking this".
         *
         * The version number (and hence the reader) is guaranteed not
         * to change while this (the mutex of this) is held locked.
         *
         * Make sure that msr instance dies as soon as possible - to
         * quick release the lock on this.
         */
        virtual Version getVersion(Cpt::MultiSyncRegion & msr) = 0;


        /**
         * Provides the reader and its version this instance owns. If
         * msr is not NULL, it is used for locking this. If msr is
         * NULL, then this instance must already be locked (by a
         * previous call to getVersion(), for instance).
         *
         * @param msr the multi sync region tool to lock the mutex of
         * this instance with. May be NULL, in which case no
         * re-locking occurs, but then this instance should be already
         * locked.
         *
         * @param version to put the current version to, must not be
         * NULL
         *
         * @return the reader
         *
         * The version number and the reader is guaranteed not to
         * change while this (the mutex of this) is held locked.
         *
         * Make sure that msr instance dies as soon as possible - to
         * quick release the lock on this.
         */
        virtual lucene::index::IndexReader * 
        getReader(Cpt::MultiSyncRegion * msr,
                  Version              * version) = 0;


        //
        // lifetime mgmt
        //
        virtual ~IReaderOwner() = 0;
    };

}


#endif
