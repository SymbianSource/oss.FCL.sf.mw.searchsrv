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

#include "CRobustnessTest.h"
#include "SearchServerCommon.h"
#include "CStressWorker.h"
#include "CLog.h"
#include "RSearchServerSession.h"

#include "SearchServerTesting.h"

_LIT( KLogFile, 		"c:\\Data\\robustness\\log.txt" ); 
_LIT( KRecordFile, 		"c:\\Data\\robustness\\record.csv" ); 
_LIT( KTempRecordFile,	"c:\\Data\\robustness\\_record.csv" );
//_LIT( KReportFile, 		"c:\\Data\\robustness\\report.txt" ); 
_LIT( KConfigFile, 		"c:\\Data\\robustness\\config.txt" ); 

CRobustnessTest::CRobustnessTest( CConsoleBase& aConsole,
								  MRobustnessTestObserver& aObserver )
:	iConsole( aConsole ),
    iOwnedLog( 0 ),
    iObserver( aObserver  ),
	iPeriodic( 0 ), 
    iSearchServerMonitor( 0 ),
    iStressWorker( 0 ),
	iTick( 0 )
	{
	}

void CRobustnessTest::ConstructL( ) 
	{
	User::LeaveIfError( iFs.Connect() ); 
	ConfigureL(); 

	iSearchServerMonitor = 
		new ( ELeave ) CProcessMonitor();
	iStressWorker = new ( ELeave ) CStressWorker( iLog, 
												  iPreIndex, 
												  iIndexAverageItems,
												  iIndexingEnabled, 
												  iDeletesEnabled,
												  iSearchersEnabled, 
												  iCancellingEnabled,
												  iIndexerSleep, 
												  iSearcherSleep ); 
	
	iPeriodic = CPeriodic::NewL( EPriorityNormal ); 
	}

TInt CRobustnessTest::ReadNumberL( TPtrC8& left )
	{
	while ( ETrue )
		{
		// Try to find end of line 
		TInt eol = left.Find( _L8( "\n" ) );
		if ( eol < 0 ) User::Leave( KErrNotFound ); 

		TPtrC8 line = left.Left( eol ); 
		left.Set( left.Mid(eol+1) ); 
		
		TLex8 lex; 
		lex.Assign( line ); 
		lex.SkipSpace();
		if ( !lex.Eos() && lex.Peek() != '#' ) 
			{
			TInt ret; 
			lex.Val( ret );
			return ret; 
			}
		}
	}

void CRobustnessTest::ConfigureL()
	{
	iTickCount = 15;
	iTickLengthSeconds = 60; // 1 minute
	iPreIndex = 0; 
	iIndexAverageItems = 1000;  
	iIndexingEnabled = ETrue; 
	iDeletesEnabled = ETrue; 
	iSearchersEnabled = ETrue; 
	iCancellingEnabled = ETrue; 
	iIndexerSleep = 100;  
	iSearcherSleep = 20;  

	RFile file; 
	if ( file.Open( iFs, KConfigFile, EFileRead ) == KErrNone ) 
		{
		CleanupClosePushL( file );
	
		TInt size; 
		file.Size( size ); 
		
		HBufC8* config = HBufC8::NewLC( size );
		TPtr8 des( config->Des() );
		User::LeaveIfError( file.Read( 0, des, size ) );  
		
		TPtrC8 loc = *config;
	
		iTickCount = ReadNumberL( loc ); 
		iTickLengthSeconds = ReadNumberL( loc ); 
		iPreIndex = ReadNumberL( loc );
		iIndexAverageItems = ReadNumberL( loc );
		iIndexingEnabled = ReadNumberL( loc ); 
		iDeletesEnabled = ReadNumberL( loc ); 
		iSearchersEnabled = ReadNumberL( loc ); 
		iCancellingEnabled = ReadNumberL( loc ); 
		iIndexerSleep = ReadNumberL( loc ); 
		iSearcherSleep = ReadNumberL( loc ); 

		CleanupStack::PopAndDestroy( config ); 
		CleanupStack::PopAndDestroy(); // file;
		}
	}


void CRobustnessTest::StartL()
	{
	// 
	// Step 0: Start logging
	iOwnedLog = CFileConsoleLog::NewL( &iConsole, KLogFile );
	iLog.SetLog( *iOwnedLog );  
	
	// 
	// Step 1. Fire up the search server
	RSearchServerSession session; 
	User::LeaveIfError( session.Connect() ); 
	session.Close(); 
	
	// 
	// Step 2. Start monitoring the search server
	iSearchServerMonitor->StartL( *this, KSearchServerName ); 
	
	//
	// Step 3. Clear old log and request search server to record RAM usage
	iFs.Delete( KServerRecordFile ); // ignore errors
	iFs.Delete( KRecordFile ); // ignore errors
	iFs.Delete( KTempRecordFile ); // ignore errors
	AddStatsLabelsL(); // prepare our own statistics record
	
	RFile file; 
	iFs.MkDirAll( KStartRecordingSignalFile ); 
	User::LeaveIfError( file.Create( iFs, KStartRecordingSignalFile, EFileWrite ) );
	file.Close(); 
	
	//
	// Step 4. Start stressing the search searver
	iStressWorker->StartL();
	
	//
	// Step 5. Set up the clock for checking reports and ending experiment
	TTimeIntervalMicroSeconds32 tickInterval( 1000*1000*iTickLengthSeconds );
	iPeriodic->Start( tickInterval, 
					  tickInterval,
					  TCallBack( CRobustnessTest::CallTick, this ) );
	
	//
	// We are done
	iLog.LogL( _L( "started" ) ); 
	}

CRobustnessTest::~CRobustnessTest()
	{
	iFs.Close();
	
	if ( iPeriodic ) 
		{
		iPeriodic->Cancel(); 
		delete iPeriodic; 
		}
	if ( iSearchServerMonitor ) 
		{
		iSearchServerMonitor->Cancel(); 
		delete iSearchServerMonitor; 
		}
	if ( iStressWorker  ) 
		{
		if ( iStressWorker->IsActive() ) iStressWorker->Finish(); 
		delete iStressWorker; 
		}
	delete iOwnedLog; 
	}

namespace 
	{
	TPtrC8 ReadLineL( TPtrC8& loc )
		{
		TInt linefeed = loc.Find( _L8("\n") );
		if ( linefeed < 0 ) 
			{
			return loc.Right( 0 );
			}
	
		TPtrC8 ret( loc.Left( linefeed ) ); 
		loc.Set( loc.Mid( linefeed + 1 ) ); 
		return ret; 
		}
	}


void CRobustnessTest::FinalizeRecordsL()
	{
	// Simply merge line by line
	RFile test;
	User::LeaveIfError( test.Open( iFs, KTempRecordFile, EFileRead ) );
	CleanupClosePushL( test ); 

	RFile server;
	User::LeaveIfError( server.Open( iFs, KServerRecordFile, EFileRead ) );
	CleanupClosePushL( server ); 

	RFile target;
	User::LeaveIfError( target.Create( iFs, KRecordFile, EFileWrite ) );
	CleanupClosePushL( target ); 
	
	HBufC8* testBuf = HBufC8::NewLC( 2048 );
	HBufC8* serverBuf = HBufC8::NewLC( 2048 );

	TPtr8 testBufDes = testBuf->Des();
	test.Read( testBufDes ); 
	TPtr8 serverBufDes = serverBuf->Des();
	server.Read( serverBufDes ); 
	
	TPtrC8 testLoc = *testBuf; 
	TPtrC8 serverLoc = *serverBuf; 
	
	for ( TInt i = 0; i < iTick; i++ )
		{
		TPtrC8 testLine = ReadLineL( testLoc ); 
		TPtrC8 serverLine = ReadLineL( serverLoc ); 
		
		target.Write( testLine ); 
		target.Write( serverLine ); 
		target.Write( _L8( "\n" ) ); 
		}
	
	target.Flush(); 
		
	CleanupStack::PopAndDestroy( serverBuf ); 
	CleanupStack::PopAndDestroy( testBuf ); 
	
	CleanupStack::PopAndDestroy( 3 ); // target, server, test
	}


void CRobustnessTest::AppendRecordL( RPointerArray<HBufC8>& aStats )
	{
	RFile file;
	TInt err = file.Open( iFs, KTempRecordFile, EFileWrite );
	if ( err == KErrNotFound ) 
		{
		err = file.Create( iFs, KTempRecordFile, EFileWrite );
		} 
	User::LeaveIfError( err ); 
	CleanupClosePushL( file ); 

	TInt end = 0;
	file.Seek( ESeekEnd, end );
	
	TInt defaultWidth = 8;
	
	for ( TInt i = 0; i < aStats.Count(); i++ ) 
		{
		file.Write( *aStats[i] );
		TInt spaces = Max( 0, defaultWidth - aStats[i]->Length() );
		for ( TInt j = 0; j < spaces; j++ )
			{
			file.Write( _L8( " " ) );
			}
		file.Write( _L8( "; " ) );
		}
	file.Write( _L8( "\n" ) );
		
	file.Flush();
	CleanupStack::PopAndDestroy(); // file
	}

void CRobustnessTest::AddStatsLabelsL() 
	{
	RPointerArray<HBufC8> stats; 
	
	TRAPD( err, stats.Append( _L8( "tick" ).AllocL() ) );
	
	if ( err == KErrNone ) 
		TRAP( err, iStressWorker->AppendStatsLabelsL( stats ) ); 
	
	if ( err == KErrNone ) 
		TRAP( err, AppendRecordL( stats ) ); 
	
	if ( err != KErrNone )
		iLog.LogL( _L( "adding labels failed") ); 
	
	stats.ResetAndDestroy();
	stats.Close(); 
	}

void CRobustnessTest::AddStatsEntryL() 
	{
	RPointerArray<HBufC8> stats; 

	TRAPD( err, 
		HBufC8* buf = HBufC8::NewL( 32 );
		buf->Des().AppendNum( iTick ); 
		stats.Append( buf ) 
		);
	
	if ( err == KErrNone ) 
		TRAP( err, iStressWorker->AppendStatsL( stats ) ); 
	
	if ( err == KErrNone ) 
		TRAP( err, AppendRecordL( stats ) ); 
		
	if ( err != KErrNone )
		iLog.LogL( _L( "adding stats failed") ); 
		
	stats.ResetAndDestroy();
	stats.Close(); 
	}

void CRobustnessTest::Tick() 
	{
	if ( iTick++ < iTickCount ) 
		{
		TRAP_IGNORE( iLog.LogL( _L( "tick") ) );

		TInt err;
		// 
		// Request server to dump performance record
		TRAP( err, AddStatsEntryL() );
		if ( err != KErrNone ) 
			{
			TRAP_IGNORE( iLog.LogL( _L( "adding record failed" ) ) );
			}
		
		RFile file;
		err = file.Create( iFs, KDumpRecordSignalFile, EFileWrite );
		if ( err == KErrNone ) 
			{
			file.Close();
			}
		else 
			{
			TRAP_IGNORE( iLog.LogL( _L( "requesting server record failed" ) ) );
			}
		
		TRAP( err, iStressWorker->ReportL() );  
		if ( err != KErrNone ) 
			{
			TRAP_IGNORE( iLog.LogL( _L( "adding record failed" ) ) );
			}
		}
	else 
		{
		TRAPD( err, 
			iPeriodic->Cancel();
			iLog.LogL( _L( "shutting down" ) );
			User::LeaveIfError( iStressWorker->Finish() ); 
			User::LeaveIfError( iFs.Connect() );
			RFile file; 
			User::LeaveIfError( file.Create( iFs, KDumpRecordSignalFile, EFileWrite ) );
			file.Close(); 
			User::LeaveIfError( file.Create( iFs, KStopRecordingSignalFile, EFileWrite ) );
			file.Close(); 
			User::LeaveIfError( file.Create( iFs, KShutdownSignalFile, EFileWrite ) );
			file.Close(); 
		);
		if ( err != KErrNone ) 
			{
			TRAP_IGNORE( iLog.LogL( _L( "failed with %d" ), err ) ); 
						
			TRAP_IGNORE( FinishL() ); 
			}
		}
	}

TInt CRobustnessTest::CallTick(TAny* aThis)
	{
	reinterpret_cast<CRobustnessTest*>(aThis)->Tick(); 
	return KErrNone; 
	}

void CRobustnessTest::FinishL() 
	{
	if ( iPeriodic )  			iPeriodic->Cancel(); 
	if ( iStressWorker ) 		iStressWorker->Finish(); 
	if ( iSearchServerMonitor ) iSearchServerMonitor->Cancel(); 

	TRAPD(err,  
		// Create a brand new record file by merging the server record
		// with the robustness test record
		iFs.Delete( KRecordFile ); 
		FinalizeRecordsL(); 
		iFs.Delete( KTempRecordFile ); 
	);
	if ( err != KErrNone ) 
		TRAP_IGNORE( iLog.LogL( _L( "finalizing record failed (%d)" ), err ); ); 
	
	if ( iTick == ( iTickCount+1 ) )
		{
		TRAP_IGNORE( iLog.LogL( _L( "finished") ) ); 
		}
	else 
		{
		TRAP_IGNORE( iLog.LogL( _L( "interrupted by server termination") ) ); 
		TRAP_IGNORE( iLog.LogL( _L( "failed") ) ); 
		}


	iObserver.FinishedL( *this );
	}
	
void CRobustnessTest::ProcessFinished( CProcessMonitor& aMonitor, 
									   TExitType aExitType ,
									   TExitCategoryName aCategory,
									   TInt aExitReason ) 
	{
	switch ( aExitType ) 
		{
		case EExitKill:
			iLog.LogL( _L( "%S shut down." ), &aMonitor.ProcessName() ); 
			break;
		case EExitTerminate: 
			iLog.LogL( _L( "%S terminated." ), &aMonitor.ProcessName() ); 
			break;
		case EExitPanic: 
			iLog.LogL( _L( "%S paniced %S %d." ), 
						&aMonitor.ProcessName(), 
						&aCategory,
						aExitReason ); 
			break;
		}
	FinishL(); 
	}

void CRobustnessTest::Failed( CProcessMonitor& aMonitor,
							  TInt aError ) 
	{
	iLog.LogL( _L( "Monitoring %S failed with %d." ), 
			   &aMonitor.ProcessName(),
			   aError ); 
	FinishL(); 
	}

