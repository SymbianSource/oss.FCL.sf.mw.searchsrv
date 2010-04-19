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

#ifndef __CCPIXINDEXER_H__
#define __CCPIXINDEXER_H__

// INCLUDE FILES
#include <e32base.h>
#include "RSearchServerSession.h"

/**
 * @file
 * @ingroup ClientAPI
 * @brief Contains CCPixIndexer used for indexing
 */


// FORWARD DECLARATIONS
class MCPixOpenDatabaseRequestObserver; 
class MCPixIndexingRequestObserver; 
class MCPixSetAnalyzerRequestObserver;

class CSearchDocument;

// CLASS DECLARATION
/**
 * @brief Used for adding documents into an index database
 * @ingroup ClientAPI
 * @todo Enable defining the database as a construction parameter for the indexer
 * 
 * An instance of the CCPixIndexer for the indexing operations of RSearchServerSession
 * 
 * Link against: CPixSearchClient.lib 
 */
class CCPixIndexer : public CActive
	{
	public: // Constructors and destructors
		
	    /**
	    * NewL.
	    * Two-phased constructor.
	    * Creates a CCPixIndexer object using two phase construction,
	    * and return a pointer to the created object.
	    * @return A pointer to the created instance of CCPixIndexer.
	    */
	    IMPORT_C static CCPixIndexer* NewL(RSearchServerSession& aSearchServerSession);
	
	    /**
	    * NewLC.
	    * Two-phased constructor.
	    * Creates a CCPixIndexer object using two phase construction,
	    * and return a pointer to the created object.
	    * @return A pointer to the created instance of CCPixIndexer.
	    */
	    IMPORT_C static CCPixIndexer* NewLC(RSearchServerSession& aSearchServerSession);
	
	    /**
	    * ~CCPixIndexer.
	    * Destructor.
	    * Destroys the object and release memory.
	    */
	    IMPORT_C virtual ~CCPixIndexer();
	
	public: // New functions

		/**
		 * Returns true, if OpenDatabaseL method has been called succesfully. 
		 */
		IMPORT_C TBool IsDatabaseOpen() const; 
		
		/**
		 * Returns KNullDesC if database not open, otherwise the base app class for the opened database 
		 */		
		IMPORT_C const TDesC& GetBaseAppClass() const;

	    /**
	     * OpenDatabaseL
	     * Opens a database to be modified by this indexer. No call to the modifying methods can 
	     * be done before a database has been successfully opened. Opening a database, when 
	     * database has already been opened will close connection to the old database. 
	     * 
	     * Leaves with KErrInUse, if asynchronous request is pending. 
	     * 
	     * @param aBaseAppClass The base application class identifying the database to be modified
	     */
		IMPORT_C void OpenDatabaseL( const TDesC& aBaseAppClass );

	    /**
	     * OpenDatabaseL
	     * Opens a database to be modified by this indexer. No call to the modifying methods can 
	     * be done before a database has been successfully opened. Opening a database, when 
	     * database has already been opened will close connection to the old database. 
	     * The result of this asycnhronous request is later returned to the observer's  	
	     * HandleOpenDatabaseResultL method. 
	     * 
	     * Leaves with KErrInUse, if asynchronous request is already pending. 
	     * 
	     * @param aObserver The observer, which is notified on the asynchronous request's completion
	     * @param aBaseAppClass The base application class identifying the database to be modified
	     */
		IMPORT_C void OpenDatabaseL(MCPixOpenDatabaseRequestObserver& aObserver, const TDesC& aBaseAppClass );

	    /**
	     * SetAnalyzer. Synchronous version
	     */
		IMPORT_C void SetAnalyzerL( const TDesC& aAnalyzer ); 

	    /**
	     * SetAnalyzer. Asynchronous version
	     */
		IMPORT_C void SetAnalyzerL( MCPixSetAnalyzerRequestObserver& aObserver, const TDesC& aAnalyzer ); 

		/**
	     * AddL. Synchronous version.
	     * Adds a document to the index. 
	     * 
	     * Leaves with KErrInUse if asynchronous request is pending, or with KErrNotReady
	     * if no database has been opened.  
	     * 
	     * @param aDocument A document to be added.
	     */
		IMPORT_C void AddL(const CSearchDocument& aDocument);

	    /**
	     * AddL. Asynchronous version. 
	     * Adds a document to the index. The result of this asynchronous request is later returned
	     * to the observer's HandleAddResultL method. 
	     * 
	     * Leaves with KErrInUse if asynchronous request is already pending, or with KErrNotReady
	     * if no database has been opened.  
	     * 
	     * @param aObserver The observer, which is notified on the request's completion
	     * @param aDocument A document to be added.
	     */
		IMPORT_C void AddL(MCPixIndexingRequestObserver& aObserver, const CSearchDocument& aDocument);

	    /**
	     * UpdateL. Synchronous version
	     * Updates a document in the index. 
	     * 
	     * Leaves with KErrInUse if asynchronous request is already pending, or with KErrNotReady
	     * if no database has been opened.  
	     * 
	     * @param aDocument A document to be updated.
	     */
		IMPORT_C void UpdateL(const CSearchDocument& aDocument);

	    /**
	     * UpdateL. Asynchronous version
	     * Updates a document in the index. The result of this asynchronous request is later returned
	     * to the observer's HandleUpdateResultL method. 
	     * 
	     * Leaves with KErrInUse if asynchronous request is already pending, or with KErrNotReady
	     * if no database has been opened.  
	     * 
	     * @param aObserver The observer, which is notified on the request's completion
	     * @param aDocument A document to be updated.
	     */
		IMPORT_C void UpdateL(MCPixIndexingRequestObserver& aObserver, const CSearchDocument& aDocument);

	    /**
	     * DeleteL
	     * Deletes a document from the index. 
	     * 
	     * Leaves with KErrInUse if asynchronous request is pending, or with KErrNotReady
	     * if no database has been opened.  
	     * 
	     * @param aDocUid A unique identifier of the document(s) to be deleted.
	     */
		IMPORT_C void DeleteL(const TDesC& aDocUid);
		
	    /**
	     * DeleteL
	     * Deletes a document from the index. The result of this asynchronous request is later returned
	     * to the observer's HandleDeleteResultL method. 
	     * 
	     * Leaves with KErrInUse if asynchronous request is already pending, or with KErrNotReady
	     * if no database has been opened.  
	     * 
	     * @param aObserver The observer, which is notified on the request's completion
	     * @param aDocUid A unique identifier of the document(s) to be deleted.
	     */
		IMPORT_C void DeleteL(MCPixIndexingRequestObserver& aObserver, const TDesC& aDocUid);

	    /**
	     * ResetL
	     * Deletes all documents from the index.
	     * 
	     * Leaves with KErrInUse if asynchronous request is already pending, or with KErrNotReady
	     * if no database has been opened.  
	     */
		IMPORT_C void ResetL();
		
	    /**
	     * ResetL
	     * Deletes all documents from the index. The result of this asynchronous request 
	     * is later returned to the observer's HandleResetResultL method. 
	     * 
         * Leaves with KErrInUse if asynchronous request is already pending, or with KErrNotReady
	     * if no database has been opened.  
	     * 
	     * @param aObserver The observer, which is notified on the request's completion
	     */
		IMPORT_C void ResetL(MCPixIndexingRequestObserver& aObserver);
		
		/**
		 * FlushL. Synchronous version.
		 * Issues CPix flush
		 * 
		 * Leaves with KErrInUse if asynchronous request is pending, or with KErrNotReady
		 * if no database has been opened.  
		 */
		IMPORT_C void FlushL();

		/**
		 * FlushL. Asynchronous version. 
		 * Issues CPix flush
		 * 
		 * Leaves with KErrInUse if asynchronous request is already pending, or with KErrNotReady
		 * if no database has been opened.  
		 * 
		 * @param aObserver The observer, which is notified on the request's completion
		 */
		IMPORT_C void FlushL(MCPixIndexingRequestObserver& aObserver);


	protected: // Functions from base classes
	
	    /**
	     * From CActive, RunL.
	     * Callback function.
	     * Invoked to handle responses from the server.
	     */
	    void RunL();
	
	    /**
	    * From CActive, DoCancel.
	    * Cancels any outstanding operation.
	    */
	    void DoCancel();

	    /**
	    * From CActive, RunError.
	    * Handle leaves from CCPixIndexer::RunL
	    */
	    TInt RunError(TInt aError);

	private: // Constructors and destructors
	
	    /**
	    * CCPixIndexer.
	    * Performs the first phase construction.
	    */
		CCPixIndexer(RSearchServerSession& aSearchServerSession);
	
	    /**
	     * ConstructL.
	     */
	    void ConstructL();
	    
	    /**
	     * Helper function to serialize a document
	     */
	    void SerializeDocumentL(const CSearchDocument& aDocument);

	private:
		
		enum TState 
			{ 
			EStateNone = 0, 
			EStateOpenDatabase, 
			EStateAdd, 
			EStateUpdate, 
			EStateDelete, 
			EStateReset,
			EStateFlush,
			EStateSetAnalyzer
			}; 
		
		union TRequestObserver
			{
			void* iAny; 
			MCPixOpenDatabaseRequestObserver* iOpenDatabase; 
			MCPixIndexingRequestObserver* iIndexing; 
			MCPixSetAnalyzerRequestObserver* iSetAnalyzer; 
			};
	
	private: // Data
	
	    /**
	    * iSession, the search server session.
	    */
	    RSearchServerSession& iSearchServerSession;
	    RSearchServerSubSession iSubSession;
	    
	    /**
	     * iSerializedDocuments, serialized CSearchDocument. 
	     * Serialized form is stored for indexing until server has completed the request. 
	     */
	    HBufC8* iSerializedDocument;
	
	    /**
	    * iObserver, observer which receives the completion value of a indexing request.
	    */
	    TRequestObserver iRequestObserver;
	    
	    /**
	     * iState, indexer state
	     */
	    TState iState;
	    
	    /**
	     * Informs, whether a database has been successfully opened.
	     */
	    TBool iIsDatabaseOpen;
	    
	    /**
	     * Used to store the base application class
	     */
	    HBufC* iBaseAppClass; 

	    /**
	     * Used to store the analyzer definition
	     */
	    HBufC* iAnalyzer;

	    /**
	     * Used to store the docUid to delete
	     */
	    HBufC* iDocUidToDelete;
};

#endif // __CCPIXINDEXER_H__

// End of File
