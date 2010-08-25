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


//  Include Files  
#include "WatchDogCommon.h"
#include "centrepmonitor.h"
#include <centralrepository.h>
#include <cpixwatchdogcommon.h>

// -----------------------------------------------------------------------------
// CentrepMonitor::NewL
// -----------------------------------------------------------------------------
//
CentrepMonitor* CentrepMonitor::NewL( MCentrepHandler* aCentrepHandler, TUint32 aKey )
    {
    CentrepMonitor* self = CentrepMonitor::NewLC( aCentrepHandler, aKey );
    CleanupStack::Pop();
    return self;
    }
	
// -----------------------------------------------------------------------------
// CentrepMonitor::NewLC
// -----------------------------------------------------------------------------
//
CentrepMonitor* CentrepMonitor::NewLC( MCentrepHandler* aCentrepHandler, TUint32 aKey )
    {
    CentrepMonitor* self = new ( ELeave ) CentrepMonitor( );
    CleanupStack::PushL( self );
    self->ConstructL( aCentrepHandler, aKey );
    return self;
    }
	
// -----------------------------------------------------------------------------
// CentrepMonitor::~CentrepMonitor()
// -----------------------------------------------------------------------------
//
CentrepMonitor::~CentrepMonitor()
    {
    Cancel();
    delete aWDrepo;
    }
	
// -----------------------------------------------------------------------------
// CentrepMonitor::CentrepMonitor()
// -----------------------------------------------------------------------------
//
CentrepMonitor::CentrepMonitor( ): CActive( CActive::EPriorityStandard )                        
    {
    
    }
	
// -----------------------------------------------------------------------------
// CentrepMonitor::ConstructL()
// -----------------------------------------------------------------------------
//
void CentrepMonitor::ConstructL( MCentrepHandler* aCentrepHandler, TUint32 aKey )
    {
    CActiveScheduler::Add( this );
    iCentrepHandler = aCentrepHandler;
    iKey = aKey;
    aWDrepo = CRepository::NewL( KWDrepoUidMenu );
    }
	
// -----------------------------------------------------------------------------
// CentrepMonitor::RunL()
// -----------------------------------------------------------------------------
//
void CentrepMonitor::RunL()
    {   
    if ( iCentrepHandler )
       iCentrepHandler->HandlecentrepL( iKey );
    aWDrepo->NotifyRequest( iKey , iStatus );
    SetActive();
    }
	
// -----------------------------------------------------------------------------
// CentrepMonitor::DoCancel()
// -----------------------------------------------------------------------------
//
void CentrepMonitor::DoCancel()
    {  
    }

// -----------------------------------------------------------------------------
// CentrepMonitor::RunError()
// -----------------------------------------------------------------------------
//
TInt CentrepMonitor::RunError( TInt )
    {                   
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CentrepMonitor::RunError()
// -----------------------------------------------------------------------------
//
void CentrepMonitor::StartNotifier()
    {
    aWDrepo->NotifyRequest( iKey , iStatus );
       SetActive();
    }
