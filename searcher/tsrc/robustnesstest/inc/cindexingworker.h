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


#ifndef CINDEXINGWORKER_H_
#define CINDEXINGWORKER_H_

#include "CWorker.h"

class CSearchDocument; 

CSearchDocument* GenerateDocumentLC( TInt aIndexAverageItems,  
									 const TDesC& aAppClass,
								     TDes& buf); 

class CIndexingWorker : public CWorker
	{
public: // Constructors & destructors
	
	CIndexingWorker( TBool aDeletesEnabled, TInt aIndexAverageItems, TInt aSleep );
	
	void ConstructL( const TDesC& aQualAppClass );
		
	static CIndexingWorker* NewL( const TDesC& aAppClass, TBool aDeletesEnabled, TInt aIndexAverageItems, TInt aSleep );
	
	~CIndexingWorker();
	
public: // From CWorker
 
	virtual void DoPrepareL();

	virtual void DoCancel();
	
	virtual void DoRunL();

	virtual const TDesC& Name();

private: 
	
	void GetId(TDes& aId);

public: 
	
	TInt ConsumeAdditions();

	TInt ConsumeAdditionsMicroSeconds();

	TInt ConsumeDeletes();

	TInt ConsumeDeletesMicroSecond ();
	
private: 
	
	TBool iStop; 
	
	TInt iNextFreeUid; 

    HBufC* iQualAppClass;

    HBufC* iAppClass;

    HBufC* iName;

	TBool iDeletesEnabled; 
    
    TInt iIndexAverageItems;

    TInt iSleep; 

private: // statistics

	TInt iAdditions; 
	
	TInt iDeletes;
	
	TInt64 iAdditionsMicroSeconds; 

	TInt64 iDeletesMicroSeconds; 

	};

#endif /* CINDEXINGWORKER_H_ */
