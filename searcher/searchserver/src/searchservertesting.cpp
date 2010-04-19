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

#include "SearchServerConfiguration.h"

#include "SearchServerTesting.h"
#include "SearchServerLogger.h"

#include "CSearchServer.h"

#ifdef PROVIDE_TESTING_UTILITY


#include <e32math.h>

TPerformanceRecord::TPerformanceRecord() 
: iMinimum(0x7fffffff), 
  iAverage(0),
  iVarEst(0),
  iPeak(0),  
  iSampleCount(0) 
	{

	}

void TPerformanceRecord::Record( TInt aValue )
	{
	iAverage = ( iAverage ) * ( (double)iSampleCount / (double)( iSampleCount + 1. ) ) 
					   + ( ( double ) aValue ) / ( iSampleCount + 1. );
	iVarEst = ( iVarEst ) * ( (double)iSampleCount / (double)( iSampleCount + 1. ) ) 
					   + ( ( aValue - iAverage ) * ( aValue - iAverage ) ) / ( iSampleCount + 1. );
	iPeak = Max( iPeak, aValue );
	iMinimum = Min( iMinimum, aValue );
	iSampleCount++; 
	}

void TPerformanceRecord::Record( TPerformanceRecord& aRecord )
	{
	iAverage = ((iAverage*iSampleCount) + (aRecord.iAverage*aRecord.iSampleCount)) / (iSampleCount+aRecord.iSampleCount); 
	iVarEst = ((iVarEst*iSampleCount) + (aRecord.iVarEst*aRecord.iSampleCount)) / (iSampleCount+aRecord.iSampleCount); 
	iPeak = Max( iPeak, aRecord.iPeak ); 
	iMinimum = Min( iPeak, aRecord.iPeak ); 
	iSampleCount += aRecord.iSampleCount; 
	}

void TPerformanceRecord::ToStringL( TDes& aString, const TDesC& aUnit, TInt aScale )
	{
	TReal sd;
	Math::Sqrt( sd, iVarEst );
	aString.AppendNum( iMinimum / aScale );
	aString.Append( _L( " " ) ); 
	aString.Append( aUnit );
	aString.Append( _L( " > ~") );
	aString.AppendNum( (TInt)(iAverage / aScale) );
	aString.Append( _L( "+-" ) ); 
	aString.AppendNum( (TInt)(sd / aScale) ); 
	aString.Append( _L( " " ) ); 
	aString.Append( aUnit ); 
	aString.Append( _L( " < ") );
	aString.AppendNum( iPeak / aScale );
	aString.Append( _L( " " ) ); 
	aString.Append( aUnit ); 
	aString.Append( _L( " (n=") );
	aString.AppendNum( iSampleCount );
	aString.Append( _L( ")") );
	}

int MemoryRecorder::iInstances = 0; 

MemoryRecorder* MemoryRecorder::NewL()
	{
	MemoryRecorder* self = new ( ELeave ) MemoryRecorder(); 	
	CleanupStack::PushL( self ); 
	self->ConstructL();
	CleanupStack::Pop( self ); 
	return self; 
	}

MemoryRecorder::MemoryRecorder()
:  	iSemaphore(), 
	iWorkerThread(),
 	iAbandon( EFalse  ), 
 	iRecord( NULL )
 	{
	}

void MemoryRecorder::ConstructL()
	{
	TThreadFunction function = MemoryRecorder::ThreadFunction; 
	User::LeaveIfError( iSemaphore.CreateLocal( 0 ) ); 
	TBuf<64> buf; 
	buf.Append( _L( "MemUsgRecord" ) ); 
	buf.AppendNum( iInstances++ ); 
	User::LeaveIfError( iWorkerThread.Create( buf, function, 2048, NULL, this ) ); 
	// Ownerhips is safely transferred to the worker thread
	// Go thread go. It will get stuck in the semaphore
	iWorkerThread.Resume(); 
	}

MemoryRecorder::~MemoryRecorder()
	{
	if ( iRecord ) 
		{
		// We are still recording!
		iSemaphore.Wait(); // need to do this to access the iRecord
		delete iRecord; 
		// And the thread is stuck 
		// Semaphore is in 0
		}
	
	iAbandon = ETrue; // Thread, let's go and die
	iSemaphore.Signal(); // Release thread and kill semaphore

	TRequestStatus status; 
	iWorkerThread.Logon( status );
	User::WaitForRequest( status ); 

	iWorkerThread.Close(); 
	iSemaphore.Close(); 
	}

void MemoryRecorder::StartL( TInt aRecordingIntervalMs ) 
	{
	if ( iRecord ) 
		{
		User::Leave( KErrNotReady ); 
		}
	
	iIntervalMs = aRecordingIntervalMs;
	iRecord = new TPerformanceRecord(); 
	Record(); 
	iSemaphore.Signal(); // Go thread go 
	
	// Must not touch record from this thread anymore
	}

TBool MemoryRecorder::IsActive() 
	{
	return iRecord != 0; 
	}

TPerformanceRecord* MemoryRecorder::Finish()
	{
	if ( !iRecord ) 
		{
		User::Panic( _L( "MemUsageRecord" ), 1 ); 
		}
	iSemaphore.Wait(); // Thread, stop
	// Can touch record from this thread now 
	Record(); 
	TPerformanceRecord* ret = iRecord;
	iRecord = NULL; 
	return ret; 
	}

void MemoryRecorder::Record()
	{
    TInt 
        largestBlock;
    TInt 
        mem = User::Heap().Size() - User::Heap().Available(largestBlock);
    
    iRecord->Record( mem ); 
	}

void MemoryRecorder::Work()
	{
	while (true) 
		{
		iSemaphore.Wait();
		// Condition: iAbandon == ETrue || iRecord != NULL 
		if ( iAbandon ) {
			break; // Quit execution
		}
		Record(); 
		iSemaphore.Signal();

		User::After( iIntervalMs * 1000 );
		}
	}

	
TInt MemoryRecorder::ThreadFunction(void *aThis)
	{	
	(reinterpret_cast<MemoryRecorder*>(aThis))->Work();
	return KErrNone;
	}

#endif // 


#ifdef LOG_PERIODIC_MEMORY_USAGE

CMemoryLogger::CMemoryLogger() 
	{
	}

CMemoryLogger::~CMemoryLogger() 
	{
	//iPeriodic->Cancel(); 
	delete iPeriodic;
	if ( iRecorder ) 
		{
		iRecorder->Close();
		}
	}

	
void CMemoryLogger::StartL()
	{
	TCallBack callback(CMemoryLogger::RunThisL, this);
	iPeriodic = CPeriodic::NewL( CActive::EPriorityNormal ); 
	iPeriodic->Start( TTimeIntervalMicroSeconds32( KMemoryLoggingIntervalMs*1000 ), 
					  TTimeIntervalMicroSeconds32( KMemoryLoggingIntervalMs*1000 ), 
					  callback ); 

	iRecorder = RMemoryRecorder::NewL(); 
	iRecorder->StartL( KMemoryRecordingIntervalMs );
	}
	
void CMemoryLogger::Stop()
	{
	iPeriodic->Cancel(); 
	delete iPeriodic; 
	iPeriodic = NULL; 
	iRecorder->Close(); 
	iRecorder = NULL; 
	}
	
void CMemoryLogger::RunL()
	{
	TPerformanceRecord* record = iRecorder->Finish(); 

	CleanupStack::PushL( record ); 
	
	TBuf<256> buf; 
	buf.Append( _L("SearchServer ") );
	record->ToStringL( buf ); 
	PERFORMANCE_LOG( buf );
	
	CleanupStack::PopAndDestroy( record ); 				
	
	iRecorder->StartL( KMemoryRecordingIntervalMs );
	}
	
TInt CMemoryLogger::RunThisL(void *aThis)
	{
	(reinterpret_cast<CMemoryLogger*>(aThis))->RunL(); 
	return KErrNone; 
	}

#endif

#ifdef PROVIDE_TESTING_UTILITY


TFunctionPerformanceEntry::TFunctionPerformanceEntry( const TDesC& aId )
	{
	iId = aId; 
	}

CSearchServerTesting* CSearchServerTesting::iInstance = NULL;

CSearchServerTesting* CSearchServerTesting::Instance()
	{
	return iInstance; 
	}
	
CSearchServerTesting::CSearchServerTesting()
:	iRecorder( 0 ),
	iServer( 0 ),
	iPeriodic( 0 ),
	iEntries()
	{
	iInstance = this;
	}

void CSearchServerTesting::ConstructL()
	{
	User::LeaveIfError( iFs.Connect() ); 
	iPeriodic = CPeriodic::NewL( CActive::EPriorityLow ); 
	iRecorder = MemoryRecorder::NewL();
	}

CSearchServerTesting::~CSearchServerTesting()
	{
	Stop(); 
	
	delete iRecorder;
	
	iEntries.Reset(); 
	iEntries.Close(); 
	
	iInstance = NULL;
	delete iPeriodic;
	
	iPeriodic = NULL; 
	iFs.Close(); 
	}
	
void CSearchServerTesting::SetServer( CSearchServer* aServer )
	{
	iServer = aServer; 
	}

void CSearchServerTesting::Reset()
	{
	iEntries.Reset(); 
	}

TFunctionPerformanceEntry& CSearchServerTesting::FunctionEntry( const TDesC& aFunctionId )
	{
	for ( TInt i = 0; i < iEntries.Count(); i++ ) 
		{
		if ( iEntries[i].iId == aFunctionId )
			{
			return iEntries[i]; 
			}
		}
	
	TFunctionPerformanceEntry entry( aFunctionId );
	TRAP_IGNORE(iEntries.AppendL( entry )); 
	
	return iEntries[iEntries.Count()-1]; 
	}

void CSearchServerTesting::StartL()
	{
	User::LeaveIfError( iFs.Connect() ); 
	TCallBack callback( CSearchServerTesting::RunThisL, this );
	iPeriodic->Start( TTimeIntervalMicroSeconds32( TSignalCheckIntervalMs*1000 ), 
					  TTimeIntervalMicroSeconds32( TSignalCheckIntervalMs*1000 ), 
					  callback ); 
}

void CSearchServerTesting::Stop()
	{
	iPeriodic->Cancel(); 
	}

void CSearchServerTesting::RunL()
	{
	TEntry entry; 

	if ( iFs.Entry( KStartRecordingSignalFile, entry ) == KErrNone  ) 
		{
		iFs.Delete( KStartRecordingSignalFile );
		StartRecordingL(); 
		}

	if ( iFs.Entry( KStopRecordingSignalFile, entry ) == KErrNone  ) 
		{
		iFs.Delete( KStopRecordingSignalFile );
		StopRecording(); 
		}

	if ( iFs.Entry( KDumpRecordSignalFile, entry ) == KErrNone  ) 
		{
		iFs.Delete( KDumpRecordSignalFile );
		DumpRecordL(); 
		}

	if ( iFs.Entry( KShutdownSignalFile, entry ) == KErrNone ) 
		{
		iFs.Delete( KShutdownSignalFile );
		ShutdownL(); 
		}
	}

TInt CSearchServerTesting::RunThisL(void *aThis)
	{
	(reinterpret_cast<CSearchServerTesting*>(aThis))->RunL(); 
	return KErrNone; 
	}

void CSearchServerTesting::ShutdownL()
	{
	if ( iServer ) 
		{
		iServer->ShutDown();
		}
	}
	
void CSearchServerTesting::StartRecordingL()
	{
	if ( !iRecorder->IsActive() )
		{
		iRecorder->StartL( KMemoryRecordingIntervalMs ); 
		}
	}

void CSearchServerTesting::StopRecording()
	{
	if ( iRecorder->IsActive() )
		{
        TPerformanceRecord* record;
        record = iRecorder->Finish();
		delete record; 
		}
	}
	

void CSearchServerTesting::DumpRecordL()
	{
	if ( !iRecorder->IsActive() ) return; // not active

	TPerformanceRecord* record = iRecorder->Finish();
	CleanupStack::PushL( record ); 

	HBufC8* buf = HBufC8::NewLC( 2048 ); 
	
	RFile file;
	TBool created = EFalse; 
	if ( file.Open( iFs, KServerRecordFile, EFileWrite ) == KErrNotFound ) 
		{
		User::LeaveIfError( file.Create( iFs, KServerRecordFile, EFileWrite ) );
		created = ETrue; 
		}
	CleanupClosePushL( file ); 

	TInt end = 0;
	file.Seek( ESeekEnd, end );
	
	if ( created ) 
		{
		buf->Des().Append( _L8("heap min ; heap aver; heap peak\n" ) );
		file.Write( *buf ); 
		buf->Des().Zero();
		}
	
	buf->Des().AppendNum( record->iMinimum ); 
	
	TInt spaces = Max( 0, 9 - buf->Length() );
	if ( spaces ) buf->Des().AppendFill( ' ', spaces );
	buf->Des().Append( _L8("; " ) );
	buf->Des().AppendNum( record->iAverage ); 
	
	spaces = Max( 0, 20 - buf->Length() );
	if ( spaces ) buf->Des().AppendFill( ' ', spaces );
	buf->Des().Append( _L8("; " ) );
	buf->Des().AppendNum( record->iPeak ); 
	
	buf->Des().Append( _L8("\n" ) );
	file.Write( *buf ); 
	file.Flush();
	
	CleanupStack::PopAndDestroy(); // file
	CleanupStack::PopAndDestroy( buf ); 
	CleanupStack::PopAndDestroy( record ); 
	
	Reset(); 
	StartRecordingL(); 
	}


	
#endif  // PROVIDE_TESTING_UTILITY


