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


#include "CProcessMonitor.h"

CProcessMonitor::CProcessMonitor()
:	CActive( EPriorityNormal ), 
    iObserver( NULL ), 
    iProcess(), 
	iProcessName()
	{
	CActiveScheduler::Add(this); 
	}
	
CProcessMonitor::~CProcessMonitor()
	{
	Cancel(); 
	Cleanup(); 
	}

void CProcessMonitor::Cleanup() 
	{
	iObserver = 0; 
	delete iProcessName; 
	iProcessName = 0; 
	iProcess.Close(); 
	}

const TDesC& CProcessMonitor::ProcessName()
	{
	if ( iProcessName ) 
		{
		return *iProcessName; 
		}
	else 
		{
		return KNullDesC;
		}
	}

void CProcessMonitor::StartL( MProcessMonitorObserver& aObserver,
							  const TDesC& aProcessName )
	{
	iObserver = &aObserver; 
	delete iProcessName; 
	iProcessName = aProcessName.AllocL();
	
	TBuf<256> match; 
	match.Append( _L("*") );
	match.Append( aProcessName );
	match.Append( _L("*") );
	
	TFindProcess find( match );
	TFullName name; 
	User::LeaveIfError( find.Next( name ) ); 
	User::LeaveIfError( iProcess.Open( find ) ); 
	
	iProcess.Logon( iStatus ); 
	SetActive(); 
	}

void CProcessMonitor::DoCancel()
	{
	iProcess.LogonCancel( iStatus ); 
	Cleanup(); 
	}

void CProcessMonitor::RunL()
	{
	if ( iStatus >= 0 ) 
		{
		TRAP_IGNORE
			(
			iObserver->ProcessFinished( *this, 
										iProcess.ExitType(), 
										iProcess.ExitCategory(), 
										iProcess.ExitReason() );
			);
		}
	else 
		{
		iObserver->Failed( *this, iStatus.Int() ); 
		}
	
	Cleanup(); 
	}

