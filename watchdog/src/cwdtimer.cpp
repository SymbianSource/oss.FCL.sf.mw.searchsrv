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
#include "WatchDogCommon.h"
#include "MWDTimerHandler.h"
#include <HarvesterServerLogger.h>
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
    CPIXLOGSTRING("CWDTimer::StartWDTimer(): Entered");    
    //start the timer
    iTimer.After( iStatus , MONITORING_DELAY ); // Wait 60 seconds before checking the servers
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
        if ( err == KErrNone)
            {
            //start the timer
            iTimer.After( iStatus , MONITORING_DELAY ); // Wait 60 seconds before checking the servers
            SetActive();
            }
        }
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
