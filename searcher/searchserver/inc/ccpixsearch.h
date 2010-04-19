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

#ifndef CCPIXSEARCH_H_
#define CCPIXSEARCH_H_

#include <e32base.h>
#include <stdio.h>
#include <wchar.h>
#include <glib.h>

#include "cpixsearch.h"
#include "cpix_async.h"
#include "indevicecfg.h"

// FORWARD DECLARATIONS
class CSearchDocument;
class CCPixAsyncronizer;
class MCPixAsyncronizerObserver;

_LIT(KCpixDefaultSearchField, CPIX_DEFAULT_FIELD);

/**
 * Symbian C++ wrapper for OpenC CPixSearch interface.
 */
class CCPixSearch : public CBase
	{
public:

	/**
	 * @return Pointer to new CCPixIdxDb instance.
	 */
	static CCPixSearch* NewL();
	
	/**
	 * @return Pointer to new CCPixIdxDb instance.
	 */
	static CCPixSearch* NewLC();
	
	/**
	 * Releases databasehandle and frees resources. CPix does it own reference counting and doesn't release
	 * all resources until all handles to given database are closed.
	 */
	virtual ~CCPixSearch();
	
public:
	
	/**
	 * Cancel any incomplete asyncronous operation
	 * @param aMessage RMessage2 of CancelAll request
	 */
	void CancelAll(const RMessage2& aMessage);	
	
	/**
	 * Search given terms from the default field.
	 * @param aSearchTerms Terms to look for.
	 * @param aObserver Observing object for this asyncronous call
	 * @param aMessage The requesting message 
	 * @return ETrue if search was commited, EFalse if it was stop word for example and search was not commited
	 */
	TBool SearchL(const TDesC& aSearchTerms, MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage);
	
	/**
	 * Complete previous call to SearchL
	 * @return Count of result documents.
	 */
	TInt SearchCompleteL();
	
	/**
	 * Search given terms from the default field.
	 * @param aSearchTerms Terms to look for.
	 * @return Count of result documents.
	 */
	TInt SuggestL(const TDesC& aWildCard, const TDesC& aField, TInt aMaxHits);

	/**
	 * Gets document from the current search results.
	 * @param aObserver Observing object for this asyncronous call
	 * @param aMessage The requesting message 
	 * @parma aIndex index of the requested document 
	 */
	void GetDocumentL(TInt aIndex, MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage);
	
	/**
	 * Complete the previous GetDocumentL
	 * @return Document. Ownership is transferred to the caller of this function.
	 */
	CSearchDocument* GetDocumentCompleteL();

	/**
	 * Creates new database (and destroys existing) if path is given. Otherwise, opens existing database.
	 * @param aDefaultSearchField Default field to which query results are looked from.
	 * @param aBaseAppClass Application class of this database handle.
	 * 	      Defines which database this handle connects to.
	 */
	void OpenDatabaseL(const TDesC& aBaseAppClass, const TDesC& aDefaultSearchField = KCpixDefaultSearchField);
	
	/**
	 * IsOpen 
	 * @returns ETrue if the database is currently open
	 */
	TBool IsOpen();
		
	void SetAnalyzerL(const TDesC& aAnalyzer); 
	
	static void InitializeL();

	static void Shutdown();	

private: // Contructors

	/**
	 * Standard C++ constructor
	 */
	CCPixSearch();
	
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
     * Log cancel action code
     * @param aCancelAction Cancel action for CPix cancel
     */
    void LogCancelAction(cpix_CancelAction aCancelAction);
    
private:
	enum TPendingTask
	{
		EPendingTaskNone = 0,
		EPendingTaskSearch,
		EPendingTaskDocument
	};

private:
	/**
	 * Base application class of this database instance
	 */
	HBufC8* iBaseAppClass;

	/**
	 * Default search field
	 */
	HBufC* iDefaultSearchFieldZ;

	/**
	 * Searcher handle
	 */	
	cpix_IdxSearcher* iIdxDb;

	/**
	 * Query analyzer.
	 */
	cpix_Analyzer* iAnalyzer;
	
	/**
	 * Query parser.
	 */
	cpix_QueryParser* iQueryParser;
	
	/**
	 * Current query instance.
	 */
	cpix_Query* iQuery;
	
	/**
	 * Results of the last query.
	 */
	cpix_Hits* iHits;

	/**
	 * Count of documents returned by last query.
	 */
	TInt iDocumentCount;
	
	/**
	 * Asynchronizer object for search and getdocument 
	 */
	CCPixAsyncronizer* iAsyncronizer;
	cpix_JobId iPendingJobId;
	TPendingTask iPendingTask;
	
	/**
	 * Needed until the cpix is changed so that the docResults gives out the document 
	 */
	cpix_Document iCurrentCpixDocument;
};


#endif /* CCPIXSEARCH_H_ */
