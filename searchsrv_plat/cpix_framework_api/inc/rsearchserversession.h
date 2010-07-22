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

#ifndef __SEARCHSERVERSESSION_H__
#define __SEARCHSERVERSESSION_H__

// INCLUDE FILES
#include <e32base.h>
#include "indevicecfg.h"


/**
 * @file
 * @ingroup ClientAPI
 * @brief Contains RSearchServerSession
 */


// CONSTANTS
// Number of message slots to reserve for this client server session.
// In this example we can have one asynchronous request outstanding
// and one synchronous request in progress.
const TUint KDefaultMessageSlots = 2;

const TUid KServerUid3 = { 0x2001F6F7 }; //{ 0xE11C75A6 }; // Server UID

_LIT( KSearchServerFilename, "CPixSearchServer" );
_LIT(KCpixDefaultField, CPIX_DEFAULT_FIELD);

#ifdef __WINS__
static const TUint KServerMinHeapSize =  0x1000;  //  4K
static const TUint KServerMaxHeapSize = 0x10000;  // 64K
#endif

// FORWARD DECLARATIONS
class CSearchDocument;

// CLASS DECLARATION
/**
 * @brief Represents CPix Search Server and is responsible for the communication with the server
 * @ingroup ClientAPI
 * 
 * This class provides the client-side interface to the server session
 * 
 * Link against: CPixSearchClient.lib 
 */
class RSearchServerSession : public RSessionBase
    {
    public: // Constructors and destructors

        /**
         * RSearchServerSession.
         * Constructs the object.
         */
    	IMPORT_C RSearchServerSession();

    public: // New functions

        /**
        * Connect.
        * Connects to the server and create a session.
        * @return Error code.
        */
    	IMPORT_C TInt Connect();

        /**
        * Close.
        * Disconnects from the server
        */
    	IMPORT_C void Close();

        /**
         * Version.
         * Gets the version number.
         * @return The version.
         */
    	IMPORT_C TVersion Version() const;

        /**
         * DefineVolumeL
         * Drop an IndexDb indentified by aQualifiedBaseAppClass
         * @param aQualifiedBaseAppClass the qualified base appClass to be defined
         * @param aIndexDbPath the physical path (optional can be KDesCNull).
         */
        IMPORT_C TInt DefineVolume(const TDesC& aQualifiedBaseAppClass, 
                                   const TDesC& aIndexDbPath);

        /**
         * UnDefineVolumeL
         * Drop an IndexDb indentified by aQualifiedBaseAppClass
         * @param aQualifiedBaseAppClass the qualified base appClass to be undefined
         */
        IMPORT_C TInt UnDefineVolume(const TDesC& aQualifiedBaseAppClass);
        
        /**
         * Stop housekeeping.
         * Search server issues CPix to do housekeeping periodically but
         * when executing log player we want to disable that.
         * Note: Use only from log player. Also remember to enable
         * housekeeping with ContinueHouseKeeping.
         */
        IMPORT_C TInt StopHouseKeeping();
        
        /**
         * Continue housekeeping.
         * Continue normal housekeeping if stopped previously with StopHouseKeeping.
         * If housekeeping is already running then calling this method resets current
         * housekeeping timer.
         * Note: Use only from log player.
         */
        IMPORT_C TInt ContinueHouseKeeping();
        
        /**
         * Force housekeeping
         * This causes Search Server to issue housekeeping command to CPix.
         * Is does not matter if house keeping is stopped with StopHouseKeeping
         * or not.
         * Note: Use only from log player.
         */
        IMPORT_C TInt ForceHouseKeeping();
        
    private: // Data
      
    };

class RSearchServerSubSession : public RSubSessionBase
{
public:
	RSearchServerSubSession();
	TInt Open(RSearchServerSession& aServer);
	void Close();

    /**
     * OpenDatabaseL.
     * Creates the database
     */
	void OpenDatabaseL(const TBool aOpenForSearch, const TDesC& aBaseAppClass, const TDesC& aDefaultSearchField = KCpixDefaultField);

    /**
      * OpenDatabaseL.
      * Creates the database
      */
	void OpenDatabase(const TBool aOpenForSearch, const TDesC& aBaseAppClass, const TDesC& aDefaultSearchField, TRequestStatus& aStatus);

    /**
      * SetAnalyzerL
      * Replaces the used analyzer with an analyzer that has been constructed
      * based on given analyzer definition 
      * @param aAnalyzer Analyzer definition string. View the documentation for format. 
      */
	IMPORT_C void SetAnalyzerL(const TDesC& aAnalyzer);
	IMPORT_C void SetAnalyzer(const TDesC& aAnalyzer, TRequestStatus& aStatus);

	
	IMPORT_C void SetQueryParserL(TInt aQueryParser);
	IMPORT_C void SetQueryParser(TInt aQueryParser, TRequestStatus& aStatus);

    /**
     * Search.
     * Issues a request for a search
     * @param aStatus The TRequestStatus object to be used for async comms.
     */
    IMPORT_C void SearchL(const TDesC& aSearchTerms);
    IMPORT_C void Search(const TDesC& aSearchTerms, TRequestStatus& aStatus);

    /**
     * GetEstimatedDocumentCount.
     * @return Maximum number of documents to be returned. Actual number can be less.
     */
    TInt GetEstimatedDocumentCount();

    /**
     * GetDocument. Synchronous version.
     * Gets a document from the search results. 
     */
    IMPORT_C CSearchDocument* GetDocumentL(TInt aIndex);
    
    /**
      * GetDocument. Asynchronous version.
      * Prepares a document from the search results. After this request
      * complets, document can be read by using GetDocumentObject. 
      * @param aStatus The TRequestStatus object to be used for async comms.
      */
    IMPORT_C void GetDocument(TInt aIndex, TRequestStatus& aStatus);
    
    /**
	 * GetDocumentObjectL.
	 * Get the actual document object prepared by the asynchronous GetDocument request.
	 * @return The document object. Ownership is transferred to the caller 
	 */
	IMPORT_C CSearchDocument* GetDocumentObjectL();

    /**
	 * AddL. Synchronous version
	 * Adds the serialized document to the index
	 */
    IMPORT_C void AddL(const TDesC8& aSerializedDocument);

	/**
	 * Add. Asynchronous version
	 * Adds the serialized document to the index
	 */
    IMPORT_C void Add(const TDesC8& aSerializedDocument, TRequestStatus& aStatus);

    /**
	 * UpdateL. Synchronous version
	 * Updates the serialized document in the index
	 */
    IMPORT_C void UpdateL(const TDesC8& aSerializedDocument);

    /**
	 * Update. Asynchronous version
	 * Updates the serialized document in the index
	 */
    IMPORT_C void Update(const TDesC8& aSerializedDocument, TRequestStatus& aStatus);

    /**
     * DeleteL. Synchronous version
     * @param aDocUid Delete document with this docuUid
     */
    IMPORT_C void DeleteL(const TDesC& aDocUid);
    
    /**
     * Delete. Asynchronous version
     * @param aDocUid Delete document with this docuUid
     * @param aStatus The TRequestStatus object to be used for async comms.
     */
    IMPORT_C void Delete(const TDesC& aDocUid, TRequestStatus& aStatus);

    /**
	 * Reset. Synchronous verions
	 * Deletes all of the documents in the index
     */
    IMPORT_C void ResetL();

	/**
	 * Reset. Asynchronous verions
	 * Deletes all of the documents in the index
	 */
    IMPORT_C void Reset(TRequestStatus& aStatus);
    
    /**
     * FlushL. Synchronous version
     * Issues CPix flush
     */
    IMPORT_C void FlushL();

    /**
     * Flush. Asynchronous version
     * Issues CPix flush
     */
    IMPORT_C void Flush(TRequestStatus& aStatus);
    
    /**
    * CancelAll.
    * Cancels any outstanding request to the server.
    */
    IMPORT_C void CancelAll() const;

private:
    /**
    * iEstimatedResultsCount, maximum amount of results received from SearchL
    */
    TInt iEstimatedResultsCount;

    /**
     * Package for results count.
     */
    TPckg<TInt> iEstimatedResultsCountPckg;

    /**
    * iDocumentSize, bytes needed for the next document
    */
    TInt iDocumentSize;
    
    /**
     * Package for document size.
     */
    TPckg<TInt> iDocumentSizePckg;

};


#endif // __SEARCHSERVERSESSION_H__


// End of File
