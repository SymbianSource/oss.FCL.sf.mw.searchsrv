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

#ifndef CPIX_CPIXFWDTYPES_H
#define CPIX_CPIXFWDTYPES_H

namespace Cpix
{
    class IIdxDb;
    class IdxDb;
    class ICLuceneHitsQuery; 
    class LuceneHits; 
    class IHits; 
    class Document; 
    class StateAssertionSentry;
    class DocumentConsumer;

    class InitParams;

    /**
     * Typedef to track version / changes of IdxDb and MultiIdxDb
     * instances.
     */
    typedef size_t    Version;

}


#endif // CPIX_CPIXFWDTYPES_H
