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
class CCPixSearch;
class CSearchDocument;

class CSearchServerSubSession : public CObject, public MCPixAsyncronizerObserver
{
public:
	static CSearchServerSubSession* NewL(CSearchServerSession* aSession);
	static CSearchServerSubSession* NewLC(CSearchServerSession* aSession);
	virtual ~CSearchServerSubSession();
	
public:	
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
	CCPixSearch* iSearchDb;
	CSearchDocument* iNextDocument;
	CSearchServerSession* iSession;
};

#endif /* CSEARCHSERVERSUBSESSION_H_ */
