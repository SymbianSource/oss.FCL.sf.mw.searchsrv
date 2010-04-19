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

#ifndef CPIX_THREADLOCALDATA_H
#define CPIX_THREADLOCALDATA_H


#include <pthread.h>


namespace Cpix
{

    class ThreadLocalData
    {
    private:
        //
        // private members
        //
        bool             isSlaveThread_;


    public:
        //
        // public operators
        //
        /**
         * @returns if this thread local data belongs to a thread that
         * should treat itself a slave
         */
        bool isSlaveThread() const;



        /**
         * ctor - initializes the local thread data 
         */
        ThreadLocalData(bool isSlaveThread);



        static void Destroy(void * that);
    };



    /**
     * This key stores the pointer to a ThreadLocalData instance.
     */
    extern pthread_key_t   ThreadLocalDataKey;

}

#endif

