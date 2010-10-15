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

#ifndef __CCPIXIDXDB_H__
#define __CCPIXIDXDB_H__

#include <e32base.h>
#include <stdio.h>
#include <wchar.h>
#include <glib.h>

#include "cpixsynctools.h"
#include "cpixidxdb.h"
#include "cpix_async.h"
//#include "MIndexingPluginObserver.h" //TODO: Bhuvi Not required 
#include "indevicecfg.h"

// FORWARD DECLARATIONS
class CSearchDocument;
class CCPixAsyncronizer;
class MCPixAsyncronizerObserver;

_LIT(KCpixDefaultField, CPIX_DEFAULT_FIELD);

/**
 * Symbian C++ wrapper for OpenC CPixIdxDb interface.
 */
class CCPixIdxDb : public CBase
	{
public:

	/**
	 * @return Pointer to new CCPixIdxDb instance.
	 */
	static CCPixIdxDb* NewL();
	
	/**
	 * @return Pointer to new CCPixIdxDb instance.
	 */
	static CCPixIdxDb* NewLC();
	
	/**
	 * Releases databasehandle and frees resources. CPix does it own reference counting and doesn't release
	 * all resources until all handles to given database are closed.
	 */
	virtual ~CCPixIdxDb();
	
	   static Cpt::Mutex idxMutex_;

public:
	
    /**
     * Cancel any incomplete asyncronous operation
     * @param aMessage RMessage2 of CancelAll request
     */
	void CancelAll(const RMessage2& aMessage);	
	

	void AddL(const CSearchDocument& aDocument, MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage); // should we expose also analyzer?
	void AddCompleteL();
	
	void UpdateL(const CSearchDocument& aDocument, MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage);	
	void UpdateCompleteL();

	void DeleteDocumentsL(const TDesC& aDocUid, MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage);
	void DeleteDocumentsCompleteL();
	
	void SetAnalyzerL(const TDesC& aAnalyzer); 

	/**
	 * Resets opened index and deletes all documents from it.
	 */
	void ResetL();

	void FlushL(MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage);
	void FlushCompleteL();
	
	/**
	 * InitializeL():
	 * Initializes the cpix layer. Should be only called once at startup of the server. 
	 */
	static void InitializeL();

	/**
	 * Shutdown():
	 * Un-initializes the cpix layer. Should be only called once at the shutdown of the server. 
	 */
	static void Shutdown();
	
	/**
	 * Creates new database (and destroys existing) if path is given. Otherwise, opens existing database.
	 * @param aBaseAppClass Application class of this database handle.
	 * 	      Defines which database this handle connects to.
	 */
	void OpenDatabaseL(const TDesC& aBaseAppClass);

	/**
	 * IsOpen 
	 * @returns ETrue if the database is currently open
	 */
	TBool IsOpen();
	
	/**
	 * Get BaseAppClass
	 */
	TPtrC8 BaseAppClass();
	
private: // New functions
		
	/**
	 * Makes a C-style zero terminated string of the field's content
	 * and returns it, if and only if the field is exist. Independent
	 * of whether C-string is returned or not, method pushes a cleanup item
	 * into the cleanup stack. Returned object is not the item in the
	 * cleanup stack.
	 */
	const char* GetFieldCStrLC(const CSearchDocument& aDocument, const TDesC& aFieldName );
	const wchar_t* GetFieldWideCStrLC(const CSearchDocument& aDocument, const TDesC& aFieldName );

	/**
	 * Converts a CSearchDocument to a cpix_Document
	 * @param aDocument Document to be converted.
	 * @return returns the cpix_document which is left on the cleanup stack.
	 */
	cpix_Document* ConvertToCpixDocumentLC(const CSearchDocument& aDocument);
	
private: // Contructors

	/**
	 * Standard C++ constructor
	 */
	CCPixIdxDb();
	
	/**
	 * Symbian 2nd phase constructor
	 */
	void ConstructL();
	
	/*
	 * Callback from CCPixAsyncronizer, called when asynchronous operations complete
	 */
    static void CompletionCallback(void *aCookie, cpix_JobId aJobId);
	
private:
	
	/**
	 * Base application class of this database instance
	 */
	HBufC8* iBaseAppClass;

	/**
	 * Index db handle
	 */	
	cpix_IdxDb* iIdxDb;
	
	/**
	 * Query analyzer.
	 */
	cpix_Analyzer* iAnalyzer;
	
	/**
	 * Asynchronizer object for search and getdocument 
	 */
	CCPixAsyncronizer* iAsyncronizer;
	cpix_JobId iPendingJobId;
	TBool iIsPending;	
	};

#endif /*__CCPIXIDXDB_H__*/
