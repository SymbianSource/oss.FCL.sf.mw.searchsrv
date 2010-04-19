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

#include "CWorker.h"

#include "robustnesstest.pan"

#include <e32math.h>

TInt LogRand( TInt aDeviation ) 
	{
	TInt rv = 0; 
	while ( Math::Random() % aDeviation ) rv++;
	return rv; 
	}

void SleepRandom( TInt aAverageMs ) 
	{
	TInt sleepMs = LogRand( 10 ) * aAverageMs / 10; 
	User::After( sleepMs * 1000 );
	}

CWorker::CWorker() 
	: iWorkerId( iNextFreeWorkerId++ ),
	  iError( KErrNone ),
	  iExitTime(),
      iReported(),
      iActive( EFalse ),
	  iThread(), 
	  iSemaphore(),
	  iLock()
	  	{
		}

void CWorker::ConstructL() 
	{
	User::LeaveIfError( iSemaphore.CreateLocal( 0 ) ); 
	User::LeaveIfError( iLock.CreateLocal( 1 ) ); 
	}

CWorker::~CWorker() 
	{
	if ( iActive ) 
		{
		Cancel(); 
		while ( TryJoin(1000) != KErrNone ){}

		}
	iSemaphore.Close(); 
	iLock.Close(); 
	}

TInt CWorker::WorkerId() 
	{
	return iWorkerId; 
	}

TInt CWorker::Error() 
	{
	return iError; 
	}

const TTime& CWorker::ExitTime()
	{
	return iExitTime; 
	}

TBool CWorker::IsReported() 
	{
	return iReported; 
	}

void CWorker::SetReported() 
	{
	iReported = ETrue; 
	}

TBool CWorker::IsActive() 
	{
	return iActive; 
	}

void CWorker::StartL() 
	{
	iError = KErrNone;
	iReported = EFalse; 
	
	TThreadFunction function = CWorker::ThreadFunction; 
	
	User::LeaveIfError( iThread.Create( Name(), function, 2048, 0, this ) ); 
	 
	iActive = ETrue;
	iThread.Resume(); 
	// Go thread go
	
}

void CWorker::Cancel() 
	{
	DoCancel(); 
	}

TInt CWorker::TryJoin( TInt aMicroSeconds ) 
	{
	TInt err = iSemaphore.Wait( aMicroSeconds );
	if ( err == KErrNone ) 
		{
		iActive = EFalse;
		iThread.Close();
		}
	return err;
	}

void CWorker::Terminate() 
	{
	iActive = EFalse;
	iThread.Panic( KRobustnessTestPanicCategory, KRobustnessTestTerminate ); 
	iThread.Close();
	}

void CWorker::Wait( TReal aSeconds ) 
	{
	RTimer timer; 
	TRequestStatus timerStatus; 
	timer.CreateLocal();        
	CleanupClosePushL( timer ); 
	
	timer.After( timerStatus, aSeconds * 1000 * 1000 ); 
	User::WaitForRequest(timerStatus); 
	
	CleanupStack::PopAndDestroy(); // timer 
	}

void CWorker::Lock() {
	iLock.Wait(); 
}

TInt CWorker::TryLock( TInt aMicroSeconds ) {
	return iLock.Wait( aMicroSeconds ); 
}

void CWorker::Unlock() {
	iLock.Signal(); 
}

void CWorker::RunL() 
	{
	CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
	CleanupStack::PushL(scheduler);
	CActiveScheduler::Install(scheduler);
	
	Lock(); 
	TRAP(iError, DoRunL()); 
	iExitTime.HomeTime(); 
	Unlock(); 
	
	iSemaphore.Signal(); 
	
	CleanupStack::PopAndDestroy(scheduler);
	}

TInt CWorker::ThreadFunction(void *aThis) 
	{
	CTrapCleanup* cleanup = CTrapCleanup::New();

	TRAP_IGNORE
		( 
		(reinterpret_cast<CWorker*>(aThis))->RunL();
		); 

	delete cleanup;
	return KErrNone;
	}

TInt CWorker::iNextFreeWorkerId = 0;  
