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


#include "cgaurdtimer.h"
#include "HarvesterServerLogger.h"

#define MONITORING_DELAY 2147483647 // Max time that RTimer::After() can allow ie. 35Min.
// -----------------------------------------------------------------------------
// CGaurdTimer::NewL
// -----------------------------------------------------------------------------
//
CGaurdTimer* CGaurdTimer::NewL( MGaurdTimerHandler* aGaurdTimerHandler )
    {
    CGaurdTimer* self = CGaurdTimer::NewLC( aGaurdTimerHandler );
    CleanupStack::Pop();
    return self;
    }
// -----------------------------------------------------------------------------
// CGaurdTimer::NewLC
// -----------------------------------------------------------------------------
//
CGaurdTimer* CGaurdTimer::NewLC( MGaurdTimerHandler* aGaurdTimerHandler )
    {
    CGaurdTimer* self = new ( ELeave ) CGaurdTimer( );
    CleanupStack::PushL( self );
    self->ConstructL( aGaurdTimerHandler );
    return self;
    }

// -----------------------------------------------------------------------------
// CGaurdTimer::~CGaurdTimer()
// -----------------------------------------------------------------------------
//
CGaurdTimer::~CGaurdTimer()
    {
    Cancel();
    iTimer.Close();
    }

// -----------------------------------------------------------------------------
// CGaurdTimer::CGaurdTimer()
// -----------------------------------------------------------------------------
//
CGaurdTimer::CGaurdTimer( ): CActive( CActive::EPriorityStandard )                        
    {
    
    }

// -----------------------------------------------------------------------------
// CGaurdTimer::ConstructL()
// -----------------------------------------------------------------------------
//
void CGaurdTimer::ConstructL( MGaurdTimerHandler* aGaurdTimerHandler )
    {
    CActiveScheduler::Add( this );
    User::LeaveIfError( iTimer.CreateLocal() );
    iGaurdTimerHandler = aGaurdTimerHandler;
    }

// -----------------------------------------------------------------------------
// CGaurdTimer::StartgaurdTimer()
// -----------------------------------------------------------------------------
//
void CGaurdTimer::StartgaurdTimer()
    {
    CPIXLOGSTRING("CGaurdTimer::StartWDTimer(): Entered");    
    //start the timer
    iTimer.After( iStatus , MONITORING_DELAY ); // Wait 60 seconds before checking the servers
    SetActive();    
    }

// -----------------------------------------------------------------------------
// CGaurdTimer::RunL()
// -----------------------------------------------------------------------------
//
void CGaurdTimer::RunL()
    {
    //check for the Harvester server and the search server
    if( iStatus.Int() == KErrNone )
        {
        TInt err = KErrNone;
        TRAP ( err , iGaurdTimerHandler->HandleGaurdTimerL() );        
        }
    }

// -----------------------------------------------------------------------------
// CGaurdTimer::DoCancel()
// -----------------------------------------------------------------------------
//
void CGaurdTimer::DoCancel()
    {
    iTimer.Cancel();     
    }

// -----------------------------------------------------------------------------
// CGaurdTimer::RunError()
// -----------------------------------------------------------------------------
//
TInt CGaurdTimer::RunError( TInt )
    {
    //Cancel the timer if there are any and start the new timer
    iTimer.Cancel();
    
    //start the timer
    iTimer.After( iStatus , MONITORING_DELAY );
    SetActive();
                
    return KErrNone;
    }
//End of file
