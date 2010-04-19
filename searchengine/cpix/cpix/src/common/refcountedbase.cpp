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

#include <limits>

#include "common/refcountedbase.h"

#include "cpixexc.h"

namespace Cpix
{


    void RefCountedBase::incRef()
    {
        if (refCount_ == std::numeric_limits<long>::max())
            {
                THROW_CPIXEXC("Reference count grew too big");
            }

        ++refCount_;
    }


    void RefCountedBase::decRef()
    {
        if (refCount_ == 0)
            {
                THROW_CPIXEXC(PL_ERROR "Reference count already zero");
            }

        --refCount_;

        if (refCount_ == 0)
            {
                delete this;
            }
    }


    RefCountedBase::RefCountedBase()
        : refCount_(1)
    {
        ;
    }


    
    RefCountedBase::~RefCountedBase()
    {
        ;
    }


}
