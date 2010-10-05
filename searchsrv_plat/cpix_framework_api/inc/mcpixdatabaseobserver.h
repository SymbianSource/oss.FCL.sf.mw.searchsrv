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

#ifndef MCPIXDATABASEOBSERVER_H_
#define MCPIXDATABASEOBSERVER_H_

#include <e32def.h>

/**
 * @file
 * @ingroup Common
 * @brief Contains MCPixOpenDatabaseRequestObserver, MCPixSetAnalyzerRequestObserver, MCPixSetQueryParserRequestObserver
 */
 
/**
 * @brief Observes completions of CPixIndexer's asynchronous requests 
 * @ingroup ClientAPI
 * 
 * Mixin class.
 * Observer to handle indexing operations completion codes.
 *  
 * Link against: CPixSearchClient.lib 
 */
class MCPixOpenDatabaseRequestObserver
    {
    public:
  
        /**
         * HandleOpenDatabaseResultL.
         * Returns completion code for OpenDatabaseL operation
         * @param aError Coompletion code of a asynchronous request. KErrNone if operation
         * was succesful, otherwise system wide error code.
         */
     	virtual void HandleOpenDatabaseResultL( TInt aError ) = 0; 
    }; 

/**
 * @brief Observes completions of CPixIndexer's asynchronous requests 
 * @ingroup ClientAPI
 * 
 * Mixin class.
 * Observer to handle indexing operations completion codes.
 *  
 * Link against: CPixSearchClient.lib 
 */
class MCPixSetAnalyzerRequestObserver
    {
    public:
  
        /**
         * HandleOpenDatabaseResultL.
         * Returns completion code for OpenDatabaseL operation
         * @param aError Coompletion code of a asynchronous request. KErrNone if operation
         * was succesful, otherwise system wide error code.
         */
     	virtual void HandleSetAnalyzerResultL( TInt aError ) = 0; 
    }; 

/**
 * @brief Observes completions of CPixIndexer's asynchronous requests 
 * @ingroup ClientAPI
 * 
 * Mixin class.
 * Observer to handle indexing operations completion codes.
 *  
 * Link against: CPixSearchClient.lib 
 */
class MCPixSetQueryParserRequestObserver
    {
    public:
  
        /**
         * TODO
         * @param aError Coompletion code of a asynchronous request. KErrNone if operation
         * was succesful, otherwise system wide error code.
         */
     	virtual void HandleSetQueryParserResultL( TInt aError ) = 0; 
    }; 

#endif /* MCPIXDATABASEOBSERVER_H_ */
