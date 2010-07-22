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

#ifndef CSEARCHSERVERSUBSESSION_H_
#define CSEARCHSERVERSUBSESSION_H_

#include "CCPixAsyncronizer.h"

// FORWARD DECLARATIONS
class CSearchServerSession;
class CCPixIdxDb;
class CCPixAbstractSearcher;
class CSearchDocument;

class CSearchServerSubSession : public CObject, public MCPixAsyncronizerObserver
{
public:
	static CSearchServerSubSession* NewL(CSearchServerSession* aSession);
	static CSearchServerSubSession* NewLC(CSearchServerSession* aSession);
	virtual ~CSearchServerSubSession();
	
public:	
	/**
	 * OpenSearchableL 
	 * Opens CPixSearcher, if aSearchableId identifies search domain,
	 * or CPixDiscoverer, if aSearchableId identifies discovery 
	 * service.
	 * 
	 * @param aSearchableId either search domain or discovery service
	 * @param aDefaultfield in case aSearchableId defines search domain, 
	 *                      this field is used as searchers default id. 
	 *                      In case discoverer is opened, this parameter is 
	 *                      ignored   
	 */
	void OpenSearcherL(const TDesC& aSearchableId, const TDesC& aDefaultField); 

	/**
	 * OpenDatabaseL.
	 * Opens database 
	 * @param aMessage Message from client.
	 */
	void OpenDatabaseL(const RMessage2& aMessage);
	
	/**
	 * SetAnalyzerL.
	 * Sets the analyzer to be used with indexing and searchign 
	 * @param aMessage Message from client.
	 */
	void SetAnalyzerL(const RMessage2& aMessage);
	
	/**
	 * SetQueryParserL.
	 * TODO
	 */
	void SetQueryParserL(const RMessage2& aMessage);
		
	/**
	 * SearchL.
	 * Issues an OpenC search.
	 * @param aMessage Message from client.
	 */
	void SearchL(const RMessage2& aMessage);
	
	/**
	 * SearchCompleteL.
	 * Completes an OpenC search
	 */
	void SearchCompleteL(const RMessage2& aMessage);

	/**
	 * GetDocumentL.
	 * Client gets the next documents (result) when SearchL has completed
	 * @param aMessage Message from client.
	 */
	void GetDocumentL(const RMessage2& aMessage);
	
	/**
	 * GetDocumentCompleteL.
	 * Completes an previous call to GetDocumentL 
	 */
	void GetDocumentCompleteL(const RMessage2& aMessage);

	/**
	 * GetDocumentObjectL.
	 * Client gets the object after GetDocumentL() has completed 
	 * @param aMessage Message from client.
	 */
	void GetDocumentObjectL(const RMessage2& aMessage);

	/**
	 * AddL.
	 * Adds document to index
	 * @param aMessage Message from client.
	 */
	void AddL(const RMessage2& aMessage);

	/*
	 * AddCompleteL.
	 * Completes an previous call to AddL 
	 */
	void AddCompleteL(const RMessage2& aMessage);
	
	/**
	 * UpdateL.
	 * Updates document to index
	 * @param aMessage Message from client.
	 */
	void UpdateL(const RMessage2& aMessage);
	
	/*
	 * UpdateCompleteL.
	 * Completes an previous call to UpdateL 
	 */
	void UpdateCompleteL(const RMessage2& aMessage);
	
	
	/**
	 * DeleteL.
	 * Deletes document from index
	 * @param aMessage Message from client.
	 */
	void DeleteL(const RMessage2& aMessage);

	/*
	 * DeleteCompleteL.
	 * Completes an previous call to DeleteL 
	 */
	void DeleteCompleteL(const RMessage2& aMessage);
	
	/*
	 * FlushCompleteL.
	 * Completes an previous call to FlushL 
	 */
	void FlushCompleteL(const RMessage2& aMessage);
	
	/**
	 * ResetL.
	 * Deletes all documents from 
	 * @param aMessage Message from client.
	 */
	void ResetL(const RMessage2& aMessage);
	
	/**
	 * FlushL.
	 * Issues CPix flush
	 * @param aMessage Message from client.
	 */
	void FlushL(const RMessage2& aMessage);

	/**
	 * CancelAll.
	 * Client calls this to cancel any asyncronous call.
	 */
	void CancelAll(const RMessage2& aMessage);
	
protected:
	void HandleAsyncronizerComplete(TCPixTaskType aType, TInt aError, const RMessage2& aMessage);
	
private:
	void ConstructL();
	CSearchServerSubSession(CSearchServerSession* aSession);	
	void LimitExcerptToMaxLengthL(CSearchDocument* aSearchDocument);
		
private:
	CCPixIdxDb* iIndexDb;
	CCPixAbstractSearcher* iSearchDb;
	CSearchDocument* iNextDocument;
	CSearchServerSession* iSession;
};

#endif /* CSEARCHSERVERSUBSESSION_H_ */
