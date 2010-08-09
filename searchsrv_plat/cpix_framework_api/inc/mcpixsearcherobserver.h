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

#ifndef __MCPIXSEARCHEROBSERVER_H__
#define __MCPIXSEARCHEROBSERVER_H__

#include <e32base.h>

class CSearchDocument;

/**
 * @file
 * @ingroup ClientAPI
 * @brief Contains MCPixSearcherObserver
 */

#include "MCPixDatabaseObserver.h"

// CLASS DECLARATION

/**
 * @brief Observes completions of CPixSearcher's asynchronous requests 
 * @ingroup ClientAPI
 * 
 * Mixin class. Observer which handles search results received from the server.
 *  
 * Link against: CPixSearchClient.lib 
 */
class MCPixSearchRequestObserver 
	{
	public: 
		/**
		 * HandleSearchResultsL.
		 * Handle search results received from the server.
		 * @param aError Completion code of a asynchronous request. KErrNone if operation
		 * was succesful, otherwise system wide error code.
		 * @aMaximumResultCount The maximum amount of results (might be less).
		 */
		virtual void HandleSearchResultsL(TInt aError, TInt aEstimatedResultCount) = 0;
	};

/**
 * @brief Observes completions of CPixSearcher's asynchronous requests 
 * @ingroup ClientAPI
 * 
 * Mixin class. Observer which handles search results received from the server.
 *  
 * Link against: CPixSearchClient.lib 
 */
class MCPixNextDocumentRequestObserver 
	{
	public: 
		/**
		* HandleDocumentL.
		* Handle document received from the server. 
		* @param aError Completion code of a asynchronous request. KErrNone if operation
		* was succesful, otherwise system wide error code.
		* @aDocument The received document. Ownership of is transfered to the observer. NULL if no more documents.
		*/
		virtual void HandleDocumentL(TInt aError, CSearchDocument* aDocument) = 0;
		
			/**
		* HandleBatchDocumentL.
		* Handle list of documents received from the server. 
		* @param aError Completion code of a asynchronous request. KErrNone if operation
		* was succesful, otherwise system wide error code.
		* @param aReturnCount count of the documents returned.
		* @aDocument The received document. Ownership of is transfered to the observer. NULL if no more documents.
		*/
		virtual void HandleBatchDocumentL(TInt aError, TInt aReturnCount, CSearchDocument** aDocument) = 0;
	};


#endif // __MCPIXSEARCHEROBSERVER_H__

// End of File
