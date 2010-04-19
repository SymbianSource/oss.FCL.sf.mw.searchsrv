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

#ifndef __MCPIXINDEXEROBSERVER_H__
#define __MCPIXINDEXEROBSERVER_H__

/**
 * @file
 * @ingroup ClientAPI
 * @brief Contains MCPixIndexerObserver
 */

#include "MCPixDatabaseObserver.h"

// CLASS DECLARATION

/**
 * @brief Observes completions of CPixIndexer's asynchronous requests 
 * @ingroup ClientAPI
 * 
 * Mixin class.
 * Observer to handle indexing operations completion codes.
 *  
 * Link against: CPixSearchClient.lib 
 */
class MCPixIndexingRequestObserver
    {
    public:
   	
        /**
        * HandleIndexingResultL.
        * Returns completion code for indexing operation.
        * @param aError Coompletion code of a asynchronous request. KErrNone if operation
        * was succesful, otherwise system wide error code.
        */
        virtual void HandleIndexingResultL( TInt aError ) = 0;
    }; 

#endif // __MCPIXINDEXEROBSERVER_H__

// End of File
