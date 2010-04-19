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

#ifndef CSTRESSWORKER_H_
#define CSTRESSWORKER_H_

#include <e32base.h>

class CWorker;
class MLog; 

class CStressWorker : public CBase
	{
public:
	
	CStressWorker( MLog& aLog, 	
				   TInt aPreIndex, 
				   TInt aIndexAverageItems,
				   TBool aIndexingEnabled,
				   TBool aDeletesEnabled, 
				   TBool aSearchersEnabled, 
				   TBool aCancellingEnabled, 
				   TInt aIndexerSleep, 
				   TInt aSearcherSleep ); 
	
	~CStressWorker(); 
	
	void ConstructL(); 

	void StartWorkersL(); 

	void PrepareIndexL(); 

	void StartL(); 
	
	void DoFinishL();
	
	TBool ReportL(); 

	void AppendStatsLabelsL( RPointerArray<HBufC8>& aStats );

	void AppendStatsL( RPointerArray<HBufC8>& aStats );

	TInt Finish(); 
	
	TBool IsActive(); 
	
protected: 
	
	void AppendOpStatsL( RPointerArray<HBufC8>& aStats, 
						 TInt aOperations, 
						 TInt64 aOperationsMicroSeconds ); 

	
	void AppendOpStatsL( RPointerArray<HBufC8>& aStats, 
						TInt aOperations, 
						TInt64 aOperationsMicroSeconds, 
						TInt64 aOperationsPeakMicroSeconds ); 

	/**
	 * This is called, after the workers have been locked. The information 
	 * about, which workers were successfully locked, is stored
	 * in aLocked variable. 
	 */
	void AppendStatsInsideLocksL( RPointerArray<HBufC8>& aStats, const RArray<TBool>& aLocked ); 

	/**
	 * Calculates the size of directory
	 */
	TInt DirectorySizeL( const TDesC& aDirectory );
	
	/**
	 * Calculates the size of index. 
	 */
	TInt IndexSizeL( const TDesC& aDirectory );

private: // State  
	
	MLog& iLog; 
	
    RPointerArray<CWorker> iWorkers; 

    TBool iIsActive;
    
private: // Configuration
	
	TInt iPreIndex;
    
	TInt iIndexAverageItems; 

	TBool iIndexingEnabled;

	TBool iDeletesEnabled;
	
	TBool iSearchersEnabled;

	TBool iCancellingEnabled;

	TInt iIndexerSleep; 

	TInt iSearcherSleep; 
	
	};

#endif /* CSTRESSWORKER_H_ */
