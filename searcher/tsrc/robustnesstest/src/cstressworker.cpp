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

#include "CStressWorker.h"
#include "RSearchServerSession.h"
#include "CCPixIndexer.h"
#include "CCPixSearcher.h"
#include "CSearchDocument.h"
#include <e32math.h>
#include "CLog.h"
#include "MCPixSearcherObserver.h"
#include "robustnesstest.pan"

#include "CWorker.h"
#include "CIndexingWorker.h"
#include "CSearchingWorker.h"

#define KLockWaitSeconds 5 // wait for some good time 

#define KLockWaitMicroSeconds KLockWaitSeconds * 1000 * 1000 // wait for some good time 

#define KWaitAfterCancellingSeconds 10 // wait for some good time 

#define KWaitFinishSeconds 10 // wait for some good time 

#define KWaitFinishMicroSeconds KWaitFinishSeconds * 1000 * 1000 // wait for some good time 


CStressWorker::CStressWorker( MLog& aLog,
							  TInt aPreIndex, 
							  TInt aIndexAverageItems,
							  TBool aIndexingEnabled,
							  TBool aDeletesEnabled,
						      TBool aSearchersEnabled, 
						      TBool aCancellingEnabled,
						      TInt aIndexerSleep, 
						      TInt aSearcherSleep )
:   iLog( aLog ),
    iWorkers(),
    iIsActive( EFalse ),
	iPreIndex         ( aPreIndex ),		
	iIndexAverageItems( aIndexAverageItems ), 
	iIndexingEnabled  ( aIndexingEnabled ),
	iDeletesEnabled	  ( aDeletesEnabled ),
	iSearchersEnabled ( aSearchersEnabled ),
	iCancellingEnabled( aCancellingEnabled ),
	iIndexerSleep	  ( aIndexerSleep), 
	iSearcherSleep    (	aSearcherSleep )
	{
	}

CStressWorker::~CStressWorker()
	{
	iWorkers.ResetAndDestroy();
	iWorkers.Close(); 
	}

void CStressWorker::ConstructL()
	{
	}

void CStressWorker::StartWorkersL() 
	{

	// 
	// Indexers
	// 
	
	
	if ( iIndexingEnabled ) 
		{
		// Indexer working with normal index
		iWorkers.Append( CIndexingWorker::NewL( KNormalVolume, iDeletesEnabled, iIndexAverageItems, iIndexerSleep ) );
			
		// 3 indexers working with busy index
		iWorkers.Append( CIndexingWorker::NewL( KBusyVolume, iDeletesEnabled, iIndexAverageItems, iIndexerSleep ) );
		iWorkers.Append( CIndexingWorker::NewL( KBusyVolume, iDeletesEnabled, iIndexAverageItems, iIndexerSleep ) );
		iWorkers.Append( CIndexingWorker::NewL( KBusyVolume, iDeletesEnabled, iIndexAverageItems, iIndexerSleep ) );
		
		// = 4 indexers in total 
		}
	
	if ( iSearchersEnabled ) 
		{
		// 
		// Searchers
		// 
		
		// Multisearcher
		iWorkers.Append( CSearchingWorker::NewL( KTestBaseAppClass, iSearcherSleep ) );

		// Search normal index 
		iWorkers.Append( CSearchingWorker::NewL( KNormalVolume, iSearcherSleep, iCancellingEnabled ) );

		// Search big the busy index
		iWorkers.Append( CSearchingWorker::NewL( KBusyVolume, iSearcherSleep ) );
		iWorkers.Append( CSearchingWorker::NewL( KBusyVolume, iSearcherSleep, EFalse, iCancellingEnabled ) );
		iWorkers.Append( CSearchingWorker::NewL( KBusyBaseAppClass, iSearcherSleep  ) );

		// = 5 searchers in total
		}
	
	for ( TInt i = 0; i < iWorkers.Count(); i++ ) 
		{
		iWorkers[i]->StartL();
		}
	}

void CStressWorker::PrepareIndexL()
	{
	RSearchServerSession session;
	User::LeaveIfError( session.Connect() ); 
	CleanupClosePushL( session ); 
	
	// First database has one reader and one or two searchers, what 
	// is something that can be expected in the actual device
	User::LeaveIfError
		(
		session.DefineVolume( KNormalVolume, 
							  KNormalIndexDirectory )
		); 
	
	// Second database has a number of searchers and indexers. 
	User::LeaveIfError
		(
		session.DefineVolume( KBusyVolume,
							  KBusyIndexDirectory )
		);
	
	if ( iPreIndex ) iLog.LogL( _L( "preparing index") ); 

	CCPixIndexer* indexer = CCPixIndexer::NewLC( session );
	indexer->OpenDatabaseL( KNormalVolume );
	indexer->ResetL();
	
	HBufC* buf = HBufC::NewLC( 4 * 1024 ); 
	TPtr pbuf( buf->Des() ); 
	
	for ( TInt i = 0; i < iPreIndex; i++ ) 
		{
		indexer->AddL( *GenerateDocumentLC( iIndexAverageItems, 
										    KNormalBaseAppClass, 
										    pbuf ) );
		CleanupStack::PopAndDestroy(); 
		}
	
	indexer->FlushL();
	
	indexer->OpenDatabaseL( KBusyVolume );
	indexer->ResetL(); 
	
	for ( TInt i = 0; i < iPreIndex; i++ ) 
		{
		indexer->AddL( *GenerateDocumentLC( iIndexAverageItems, 
										    KBusyBaseAppClass, 
										    pbuf ) );
		CleanupStack::PopAndDestroy(); 
		}
	
	CleanupStack::PopAndDestroy( buf );
	
	indexer->FlushL();
	CleanupStack::PopAndDestroy( indexer ); 
	CleanupStack::PopAndDestroy(); // session 
	}

void CStressWorker::StartL()
	{
	
	PrepareIndexL(); 
	
	TRAPD( err, StartWorkersL() ); 
	
	if ( err != KErrNone )
		{
		iWorkers.ResetAndDestroy();
		User::Leave( err ); 
		}
	else 
		{
		iIsActive = ETrue; 
		}
	}

void CStressWorker::DoFinishL()
	{
	for ( TInt i = 0; i < iWorkers.Count(); i++ ) 
		{
		iWorkers[i]->Cancel();
		}
	
	User::After( KWaitAfterCancellingSeconds * 1000 * 1000 );
	
	int running = iWorkers.Count();
	TInt round = 0;
	for (; round < 3 && running ; round++ ) 
		{
		switch (round) 
			{
			case 1: 
			case 2: 
				iLog.LogL( _L( "Retrying to stop workers." ) );
				break;
			}
		for ( TInt i = 0; i < iWorkers.Count(); i++ ) 
			{
			if ( iWorkers[i]->IsActive() )
				{
				if ( iWorkers[i]->TryJoin( KWaitFinishMicroSeconds ) == KErrNone )
					{
					running--;
					if ( round > 0 )
						{
						iLog.LogL( _L( "worker '%S' finished" ),
								   &iWorkers[i]->Name() );
						}
					}
				else 
					{
					iLog.LogL( _L( "worker '%S' failed to exit within %d s" ),
							   &iWorkers[i]->Name(),
							   KWaitFinishSeconds  );
					}
				}
			}
		}

	if ( running ) 
		{  
		iLog.LogL( _L( "Deadlock? %d worker(s) failed to exit." ),
				   running );
		
		iLog.LogL( _L( "Terminating workers forcefully (may cause leaks)" ) );
		
		for ( TInt i = 0; i < iWorkers.Count(); i++ ) 
			{
			if ( iWorkers[i]->IsActive() )
				{
				iWorkers[i]->Terminate();
				iWorkers[i]->Unlock(); // So that we can check the records. 
				iLog.LogL( _L( "terminated worker '%S'." ),
						   &iWorkers[i]->Name() ); 
				}
			}

		}
	else if (round > 1) 
		{
		iLog.LogL( _L( "All workers finished successfully." ) ); 
		}

	ReportL();
	
	}

TBool CStressWorker::ReportL()
	{
	TBool error = EFalse; 
	for ( TInt i = 0; i < iWorkers.Count(); i++ ) 
		{
		if ( iWorkers[i]->Error() != KErrNone && 
			 iWorkers[i]->IsReported() == EFalse )
			{
			TDateTime time = iWorkers[i]->ExitTime().DateTime();
			
			TBuf<64> buf; 
			buf.AppendNumFixedWidth( (TUint)time.Hour(), EDecimal, 2 );
			buf.Append(_L(":"));
			buf.AppendNumFixedWidth( (TUint)time.Minute(), EDecimal, 2 );
			buf.Append(_L(":"));
			buf.AppendNumFixedWidth( (TUint)time.Second(), EDecimal, 2 );
			buf.Append(_L("."));
			buf.AppendNumFixedWidth( (TUint)time.MicroSecond(), EDecimal, 3 );
			
			iLog.LogL( _L( "worker '%S' failed with %d at %S" ),
					   &iWorkers[i]->Name(),
					   iWorkers[i]->Error(),
					   &buf  );
			iWorkers[i]->SetReported();
			error = ETrue; 
			}
		}
	return error;
	}


void CStressWorker::AppendOpStatsL( RPointerArray<HBufC8>& aStats, TInt aOperations, TInt64 aOperationsMicroSeconds ) 
	{
	HBufC8* buf = HBufC8::NewL( 64 ); 
	buf->Des().AppendNum( aOperations ); 
	aStats.Append( buf );
	buf = HBufC8::NewL( 64 ); 
	if ( aOperations > 0 ) 
		{
		buf->Des().AppendNum( (aOperationsMicroSeconds / 1000) / aOperations ); 
		}
	else 
		{
		buf->Des().Append( _L8( "NaN") ); 
		}
	aStats.Append( buf );
	}

void CStressWorker::AppendOpStatsL( RPointerArray<HBufC8>& aStats, TInt aOperations, TInt64 aOperationsMicroSeconds, TInt64 aOperationsPeakMicroSeconds ) 
	{
	HBufC8* buf = HBufC8::NewL( 64 ); 
	AppendOpStatsL( aStats, aOperations, aOperationsMicroSeconds );
	
	if ( aOperations > 0 )
		{
		buf->Des().AppendNum( aOperationsPeakMicroSeconds / 1000 ); 
		}
	else 
		{
		buf->Des().Append( _L8( "NaN") ); 
		}
	aStats.Append( buf );
	}

TInt CStressWorker::DirectorySizeL( const TDesC& aDirectory )
	{

	RFs fs; 
	User::LeaveIfError( fs.Connect() ); 
	CleanupClosePushL( fs ); 
	
	TBuf<256> buf; 
	buf.Append( aDirectory ); 
	buf.Append( _L( "\\*" ) );  
	
	// Make sure, that the index is not under merging or optimization operations
	
	CDir* dir; 
	TInt err = fs.GetDir( buf,        
				          KEntryAttMaskSupported,
						  ESortByName,
						  dir );
	TInt size = 0; 
	if ( err == KErrNone ) 
		{
		for (TInt i=0;i<dir->Count();i++)
			{
			size += (*dir)[i].iSize;
			}
		
		delete dir;
		}
	else 
		{
		// It is ok, if directory was not found.
		}
	
	CleanupStack::PopAndDestroy(); // fs
	
	return size;
	}

_LIT( KIndexZero, "\\_0" ); 
_LIT( KIndexOne, "\\_1" ); 

TInt CStressWorker::IndexSizeL( const TDesC& aDirectory )
	{
	TInt size = 0; 
	TBuf<256> buf;
	buf.Append( aDirectory ); 
	buf.Append( KIndexZero ); 
	size += DirectorySizeL( buf ); 
	buf.Zero();
	buf.Append( aDirectory ); 
	buf.Append( KIndexOne ); 
	size += DirectorySizeL( buf ); 
	return size;
	}

void CStressWorker::AppendStatsLabelsL( RPointerArray<HBufC8>& aStats )
	{
	aStats.Append( _L8( "norm sz").AllocL() ); 
	aStats.Append( _L8( "busy sz").AllocL() );
	
	if ( iIndexingEnabled )
		{
		aStats.Append( _L8( "adds").AllocL() ); 
		aStats.Append( _L8( "ms/op").AllocL() ); 
		aStats.Append( _L8( "deletes").AllocL() ); 
		aStats.Append( _L8( "ms/op").AllocL() );
		}
	if ( iSearchersEnabled ) 
		{
		aStats.Append( _L8( "searches").AllocL() ); 
		aStats.Append( _L8( "ms/op").AllocL() ); 
		aStats.Append( _L8( "peak").AllocL() ); 

		aStats.Append( _L8( "getdocs").AllocL() ); 
		aStats.Append( _L8( "ms/op").AllocL() ); 
		aStats.Append( _L8( "peak").AllocL() ); 

		aStats.Append( _L8( "terms").AllocL() ); 
		aStats.Append( _L8( "ms/op").AllocL() ); 
		aStats.Append( _L8( "peak").AllocL() ); 

		aStats.Append( _L8( "getterms").AllocL() ); 
		aStats.Append( _L8( "ms/op").AllocL() ); 
		aStats.Append( _L8( "peak").AllocL() ); 
		}
	}


void CStressWorker::AppendStatsInsideLocksL( RPointerArray<HBufC8>& aStats, const RArray<TBool>& aLocked ) 
	{
	TInt additions = 0; 
	TInt64 additionsMicroSeconds = 0;
		
	TInt deletes = 0; 
	TInt64 deletesMicroSeconds = 0; 

	TInt searches = 0; 
	TInt64 searchesMicroSeconds = 0; 
	TInt64 searchesPeakMicroSeconds = 0; 
	
	TInt termSearches = 0; 
	TInt64 termSearchesMicroSeconds = 0; 
	TInt64 termSearchesPeakMicroSeconds = 0; 

	TInt docs = 0; 
	TInt64 docsMicroSeconds = 0; 
	TInt64 docsPeakMicroSeconds = 0; 

	TInt terms = 0; 
	TInt64 termsMicroSeconds = 0; 
	TInt64 termsPeakMicroSeconds = 0; 

	for ( TInt i = 0; i < iWorkers.Count(); i++ ) 
		{
		if ( aLocked[i] )
			{
			// Extract statistics only, if worker was successfully locked. 
			if ( dynamic_cast<CIndexingWorker*>( iWorkers[i] ) ) 
				{
				CIndexingWorker* worker = dynamic_cast<CIndexingWorker*>( iWorkers[i] ); 
				additions 				+= worker->ConsumeAdditions(); 
				additionsMicroSeconds 	+= worker->ConsumeAdditionsMicroSeconds(); 
				deletes					+= worker->ConsumeDeletes(); 
				deletesMicroSeconds 	+= worker->ConsumeDeletesMicroSecond(); 
				}
			if ( dynamic_cast<CSearchingWorker*>( iWorkers[i] ) ) 
				{
				CSearchingWorker* worker = dynamic_cast<CSearchingWorker*>( iWorkers[i] );
				searches 					+= worker->ConsumeSearches(); 
				searchesMicroSeconds 		+= worker->ConsumeSearchesMicroSeconds(); 
				searchesPeakMicroSeconds 	+= worker->ConsumeSearchesPeakMicroSeconds();
				
				termSearches 				+= worker->ConsumeTermSearches(); 
				termSearchesMicroSeconds	+= worker->ConsumeTermSearchesMicroSeconds(); 
				termSearchesPeakMicroSeconds+= worker->ConsumeTermSearchesPeakMicroSeconds(); 

				docs 						+= worker->ConsumeDocs(); 
				docsMicroSeconds			+= worker->ConsumeDocsMicroSeconds(); 
				docsPeakMicroSeconds		+= worker->ConsumeDocsPeakMicroSeconds(); 

				terms 						+= worker->ConsumeTerms(); 
				termsMicroSeconds			+= worker->ConsumeTermsMicroSeconds(); 
				termsPeakMicroSeconds		+= worker->ConsumeTermsPeakMicroSeconds(); 
				}
			}
		}
	HBufC8* buf = HBufC8::NewL( 64 ); 
	buf->Des().AppendNum( IndexSizeL( KNormalIndexDirectory ) ); 
	aStats.Append( buf );

	buf = HBufC8::NewL( 64 ); 
	buf->Des().AppendNum( IndexSizeL( KBusyIndexDirectory ) ); 
	aStats.Append( buf );
	
	if ( iIndexingEnabled )
		{
		AppendOpStatsL( aStats, additions, additionsMicroSeconds ); 
		AppendOpStatsL( aStats, deletes, deletesMicroSeconds );
		}
	
	if ( iSearchersEnabled ) 
		{
		AppendOpStatsL( aStats, searches, searchesMicroSeconds, searchesPeakMicroSeconds ); 
		AppendOpStatsL( aStats, docs, docsMicroSeconds, docsPeakMicroSeconds ); 
		AppendOpStatsL( aStats, termSearches, termSearchesMicroSeconds, termSearchesPeakMicroSeconds );
		AppendOpStatsL( aStats, terms, termsMicroSeconds, termsPeakMicroSeconds ); 
		}
	
	}

void CStressWorker::AppendStatsL( RPointerArray<HBufC8>& aStats )
	{
	RArray<TBool> locked;
	CleanupClosePushL( locked ); 
	
	// First of first: lock all workers that can be locked and request
	// their entries
	
	for ( TInt i = 0; i < iWorkers.Count(); i++ ) 
		{
		if ( iWorkers[i]->TryLock( KLockWaitMicroSeconds ) == KErrNone )
			{
			locked.Append( ETrue ); 
			}
		else 
			{
			TRAP_IGNORE
				( 
				iLog.LogL( _L( "worker '%S' failed to respond within %d s" ),
						   &iWorkers[i]->Name(),
						   KLockWaitSeconds );
				)
			locked.Append( EFalse ); 
			}
		}
	
	TRAPD( err, AppendStatsInsideLocksL( aStats, locked ) ); 
	
	for ( TInt i = 0; i < iWorkers.Count(); i++ ) 
		{
		if ( locked[i] ) 
			{
			iWorkers[i]->Unlock();
			}
		}
	
	CleanupStack::PopAndDestroy(); // locked
	
	User::LeaveIfError( err ); 
}

TInt CStressWorker::Finish()
	{
	if ( iIsActive ) 
		{
		TRAPD( err, DoFinishL() ); 
		if ( err != KErrNone ) 
			{
			iWorkers.ResetAndDestroy();
			}
		iIsActive = EFalse; 
		return err; 
		}
	return KErrNone; 
	}

TBool CStressWorker::IsActive() 
	{
	return iIsActive; 
	}
