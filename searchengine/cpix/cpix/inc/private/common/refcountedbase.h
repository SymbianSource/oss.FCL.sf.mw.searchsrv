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

#ifndef CPIX_REFCOUNTEDBASE_H
#define CPIX_REFCOUNTEDBASE_H


namespace Cpix
{

    class RefCountedBase
    {
    private:
        //
        // private members
        //
        long       refCount_;

    public:
        //
        // public operations
        //
        
        /**
         * Increases the reference of this object by one.
         */
        void incRef();


        /**
         * Decreases the reference of this object by one - if it falls
         * to 0 then it also self-destructs.
         */
        void decRef();


        //
        // lifetime management
        //
        /**
         * Constructs a reference counted base instance, initalizing
         * the reference count to 1.
         */
        RefCountedBase();


        /**
         * Virtual destructor. Clients don't call it, they use
         * decRef() instead.
         */
        virtual ~RefCountedBase() = 0;
    };


}


#endif
