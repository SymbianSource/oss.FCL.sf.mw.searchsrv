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
* Description:  This application is to monitor Harvester and Search Server
*
*/


#include "CWDTimer.h"
#include "MWDTimerHandler.h"
#include <HarvesterServerLogger.h>
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cwdtimerTraces.h"
#endif

#define MONITORING_DELAY 60000000 // Nano seconds to delay the monitored object
// -----------------------------------------------------------------------------
// CWDTimer::NewL
// -----------------------------------------------------------------------------
//
CWDTimer* CWDTimer::NewL( MWDTimerHandler* aWDTimerHandler )
    {
    CWDTimer* self = CWDTimer::NewLC( aWDTimerHandler );
    CleanupStack::Pop();
    return self;
    }
// -----------------------------------------------------------------------------
// CWDTimer::NewLC
// -----------------------------------------------------------------------------
//
CWDTimer* CWDTimer::NewLC( MWDTimerHandler* aWDTimerHandler )
    {
    CWDTimer* self = new ( ELeave ) CWDTimer( );
    CleanupStack::PushL( self );
    self->ConstructL( aWDTimerHandler );
    return self;
    }

// -----------------------------------------------------------------------------
// CWDTimer::~CWDTimer()
// -----------------------------------------------------------------------------
//
CWDTimer::~CWDTimer()
    {
    Cancel();
    iTimer.Close();
    }

// -----------------------------------------------------------------------------
// CWDTimer::CWDTimer()
// -----------------------------------------------------------------------------
//
CWDTimer::CWDTimer( ): CActive( CActive::EPriorityStandard )                        
    {
    
    }

// -----------------------------------------------------------------------------
// CWDTimer::ConstructL()
// -----------------------------------------------------------------------------
//
void CWDTimer::ConstructL( MWDTimerHandler* aWDTimerHandler )
    {
    CActiveScheduler::Add( this );
    User::LeaveIfError( iTimer.CreateLocal() );
    iWDTimerHandler = aWDTimerHandler;
    }

// -----------------------------------------------------------------------------
// CWDTimer::StartWDTimer()
// -----------------------------------------------------------------------------
//
void CWDTimer::StartWDTimer()
    {
    OstTraceFunctionEntry0( CWDTIMER_STARTWDTIMER_ENTRY );
    CPIXLOGSTRING("CWDTimer::StartWDTimer(): Entered");    
    //start the timer
    iTimer.After( iStatus , MONITORING_DELAY ); // Wait 60 seconds before checking the servers
    OstTraceFunctionExit0( CWDTIMER_STARTWDTIMER_EXIT );
    SetActive();    
    }

// -----------------------------------------------------------------------------
// CWDTimer::RunL()
// -----------------------------------------------------------------------------
//
void CWDTimer::RunL()
    {
    //check for the Harvester server and the search server
    if( iStatus.Int() == KErrNone )
        {
        TInt err = KErrNone;
        TRAP ( err , iWDTimerHandler->HandleWDTimerL() );        
        }
    // clean the timer
    Cancel();
    iTimer.Close();
    }

// -----------------------------------------------------------------------------
// CWDTimer::DoCancel()
// -----------------------------------------------------------------------------
//
void CWDTimer::DoCancel()
    {
    iTimer.Cancel();     
    }

// -----------------------------------------------------------------------------
// CWDTimer::RunError()
// -----------------------------------------------------------------------------
//
TInt CWDTimer::RunError( TInt )
    {
    //Cancel the timer if there are any and start the new timer
    iTimer.Cancel();
    
    //start the timer
    iTimer.After( iStatus , MONITORING_DELAY );
    SetActive();
                
    return KErrNone;
    }
//End of file
