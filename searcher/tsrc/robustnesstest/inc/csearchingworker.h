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

#ifndef CSEARCHINGWORKER_H_
#define CSEARCHINGWORKER_H_

#include "CWorker.h"
#include "MCPixSearcherObserver.h"

class CSearchDocument; 

class CSearchingWorker : public CWorker, public MCPixSearchRequestObserver, public MCPixNextDocumentRequestObserver
	{
public: 
	CSearchingWorker( TInt aSleep, TBool aCancelSearch, TBool aCancelDocumentFetching );
	static CSearchingWorker* NewL( 
	        const TDesC& aAppClass,
	        TInt aSleep,
	        TBool aCancelSearch=EFalse,
	        TBool aCancelDocumentFetching=EFalse );

	void ConstructL( const TDesC& aAppClass );
	
	~CSearchingWorker();

public: // From Cworker

	virtual void DoPrepareL();

	virtual void DoCancel();
	
	virtual void DoRunL();
	
	virtual const TDesC& Name();

public: // From MCPixSearchRequestObserver 

    virtual void HandleSearchResultsL(TInt /*aError*/, TInt /*aEstimatedResultCount*/);
	
public: // From MCPixNextDocumentRequestObserver
    
    virtual void HandleDocumentL(TInt /*aError*/, CSearchDocument* /*aDocument*/);
    virtual void HandleBatchDocumentL(TInt aError, TInt aReturnCount, CSearchDocument** aDocument) ;

public: // Statistics 
	
	TInt ConsumeSearches();
	TInt ConsumeSearchesPeakMicroSeconds();
	TInt ConsumeSearchesMicroSeconds();
	TInt ConsumeTermSearches();
	TInt ConsumeTermSearchesMicroSeconds();
	TInt ConsumeTermSearchesPeakMicroSeconds();
	TInt ConsumeDocs();
	TInt ConsumeDocsMicroSeconds();
	TInt ConsumeDocsPeakMicroSeconds();
	TInt ConsumeTerms();
	TInt ConsumeTermsMicroSeconds();
	TInt ConsumeTermsPeakMicroSeconds();

private: // Statistics

	TInt iSearches; 
	TInt iTermSearches; 
	TInt64 iSearchesMicroSeconds; 
	
	TInt64 iTermSearchesMicroSeconds; 
	TInt64 iSearchesPeakMicroSeconds; 
	TInt64 iTermSearchesPeakMicroSeconds;

	TInt iDocs; 
	TInt iDocsMicroSeconds; 
	TInt iDocsPeakMicroSeconds; 

	TInt iTerms; 
	TInt iTermsMicroSeconds; 
	TInt iTermsPeakMicroSeconds; 

private:
	
	TBool iStop; 
	TInt iSleep;
	TBool iCancelSearch;
	TBool iCancelDocumentFetching;
	HBufC* iAppClass; 
	HBufC* iName; 
	
	};


#endif /* CSEARCHINGWORKER_H_ */
