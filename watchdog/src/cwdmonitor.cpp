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

#include "CWDmonitor.h"
#include "WatchDogCommon.h"
#include "CWDTimer.h"
#include <HarvesterServerLogger.h>
// -----------------------------------------------------------------------------
// CWDMonitor::NewL
// -----------------------------------------------------------------------------
//
CWDMonitor* CWDMonitor::NewL(  )
    {
    CWDMonitor* self = CWDMonitor::NewLC( );
    CleanupStack::Pop();
    return self;
    }
// -----------------------------------------------------------------------------
// CWDMonitor::NewLC
// -----------------------------------------------------------------------------
//
CWDMonitor* CWDMonitor::NewLC( )
    {
    CWDMonitor* self = new ( ELeave ) CWDMonitor( );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CWDMonitor::~CWDMonitor()
// -----------------------------------------------------------------------------
//
CWDMonitor::~CWDMonitor()
    {
    delete iWDTimer;
    }

// -----------------------------------------------------------------------------
// CWDMonitor::CWDMonitor()
// -----------------------------------------------------------------------------
//
CWDMonitor::CWDMonitor( )                    
    {    
    }

// -----------------------------------------------------------------------------
// CWDMonitor::ConstructL()
// -----------------------------------------------------------------------------
//
void CWDMonitor::ConstructL()
    {
    iWDTimer = CWDTimer::NewL( this );
    }

// -----------------------------------------------------------------------------
// CWDMonitor::StartMonitor()
// -----------------------------------------------------------------------------
//
void CWDMonitor::HandleWDTimerL()
    {
    CPIXLOGSTRING("CWDMonitor::HandleWDTimerL(): Check the servers");
    TFindServer harvesterServer(KHarvesterServer);
    TFindServer searchServer(KSearchServer);
    
    TFullName name;
    
    if ( harvesterServer.Next(name) != KErrNone)
        {
        CPIXLOGSTRING("Harvester Server is down, Starting Harvester Server");
        //Harvester server is not running. 
        //Start Harvester server
        StartServer( KHarvesterServer , KHServerUid3 ,KHarvesterServerSemaphoreName);        
        }
    else if ( searchServer.Next( name ) != KErrNone)
        {
        CPIXLOGSTRING("Search Server is down, Starting Search Server");
        //Search server is not running.
        //Start search server
        StartServer( KSearchServer , KSServerUid3 ,KSearchServerSemaphoreName);
        }
    return;
    }
// -----------------------------------------------------------------------------
// CWDMonitor::StartMonitor()
// -----------------------------------------------------------------------------
//
void CWDMonitor::StartMonitor()
    {
    CPIXLOGSTRING("CWDMonitor::StartMonitor(): Entered");
    iWDTimer->StartWDTimer();
    }

// -----------------------------------------------------------------------------
// CWDMonitor::StartServer()
// -----------------------------------------------------------------------------
//
TInt CWDMonitor::StartServer( const TDesC& aServerName , TUid aServerUid ,
                              const TDesC& aSemaphoreName)
    {
    RSemaphore semaphore;
    TInt result = semaphore.CreateGlobal(aSemaphoreName, 0);
    if (result != KErrNone)
        {
        return result;
        }

    result = CreateServerProcess( aServerName , aServerUid);
    if (result != KErrNone)
        {
        return result;
        }

    semaphore.Wait();
    semaphore.Close();
    return result;
    }

// -----------------------------------------------------------------------------
// CWDMonitor::CreateServerProcess()
// -----------------------------------------------------------------------------
//
TInt CWDMonitor::CreateServerProcess( const TDesC& aServerName , TUid aServerUid  )
    {
    TInt result;

    const TUidType serverUid( KNullUid, KNullUid, aServerUid);

    RProcess server;

    result = server.Create(aServerName, KNullDesC, serverUid);
    if (result != KErrNone)
        {
        return result;
        }

    server.Resume();
    server.Close();
    return KErrNone;
    }

//End of file
