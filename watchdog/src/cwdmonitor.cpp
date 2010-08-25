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
#include <centralrepository.h>
#include <cpixwatchdogcommon.h>
#include "centrepmonitor.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cwdmonitorTraces.h"
#endif

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
    delete iHSName;
    delete iSSName;
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
    CRepository* wdrepo = CRepository::NewL( KWDrepoUidMenu );
    wdrepo->Get( KHarvesterServerKey , iAllowHS );
    wdrepo->Get( KSearchServerKey , iAllowSS );
    
    //get the UID's of the servers
    TBuf<KCenrepUidLength> temp;
    TInt64 value;
    TLex uidvalue;
    //Read Harvester server UId value
    if ( KErrNone == wdrepo->Get( KHarvesterServerUIDKey, temp ) )
        {
        uidvalue.Assign(temp);        
        if (KErrNone == uidvalue.Val( value,EHex ))
            iHSUid.iUid = value;
        }
    //Read Search server Uid value
    if ( KErrNone == wdrepo->Get( KSearchServerUIDKey, temp ))
        {
        uidvalue.Assign(temp);
        if (KErrNone == uidvalue.Val( value,EHex ))
          iSSUid.iUid = value;
        }
    
    //Read Harvester server Name
    if ( KErrNone == wdrepo->Get( KHarvesterServerNAMEKey, temp ))
        {
        iHSName = HBufC::NewL( temp.Length() );
        TPtr hsname = iHSName->Des();
        hsname.Copy( temp );
        }
    //Read Search server Name
    if ( KErrNone == wdrepo->Get( KSearchServerNAMEKey, temp ))
        {
        iSSName = HBufC::NewL( temp.Length() );
        TPtr ssname = iSSName->Des(); 
        ssname.Copy( temp );
        }
    delete wdrepo;
    if ( iAllowHS || iAllowSS )
        iWDTimer = CWDTimer::NewL( this );
    
    aHSMonitor = CentrepMonitor::NewL( this, KHarvesterServerKey);
    aHSMonitor->StartNotifier();
    aSSMonitor = CentrepMonitor::NewL( this, KSearchServerKey);
    aSSMonitor->StartNotifier();
    }

// -----------------------------------------------------------------------------
// CWDMonitor::StartMonitor()
// -----------------------------------------------------------------------------
//
void CWDMonitor::HandleWDTimerL()
    {
    OstTrace0( TRACE_NORMAL, CWDMONITOR_HANDLEWDTIMERL, "CWDMonitor::HandleWDTimerL(): Check the servers" );
    CPIXLOGSTRING("CWDMonitor::HandleWDTimerL(): Check the servers");
    TFindServer harvesterServer(*iHSName);
    TFindServer searchServer(*iSSName);
    
    TFullName name;
    
    if ( iAllowHS && (harvesterServer.Next(name) != KErrNone) )
        {
        OstTrace0( TRACE_NORMAL, DUP1_CWDMONITOR_HANDLEWDTIMERL, "Harvester Server is down, Starting Harvester Server" );
        CPIXLOGSTRING("Harvester Server is down, Starting Harvester Server");
        //Harvester server is not running. 
        //Start Harvester server
        StartServer( *iHSName , iHSUid ,KHarvesterServerSemaphoreName);        
        }
    else if ( iAllowSS && (searchServer.Next( name ) != KErrNone) )
        {
        OstTrace0( TRACE_NORMAL, DUP2_CWDMONITOR_HANDLEWDTIMERL, "Search Server is down, Starting Search Server" );
        CPIXLOGSTRING("Search Server is down, Starting Search Server");
        //Search server is not running.
        //Start search server
        StartServer( *iSSName , iSSUid ,KSearchServerSemaphoreName);
        }
    return;
    }
// -----------------------------------------------------------------------------
// CWDMonitor::StartMonitor()
// -----------------------------------------------------------------------------
//
void CWDMonitor::StartMonitor()
    {
    OstTraceFunctionEntry0( CWDMONITOR_STARTMONITOR_ENTRY );
    CPIXLOGSTRING("CWDMonitor::StartMonitor(): Entered");
    iWDTimer->StartWDTimer();
    OstTraceFunctionExit0( CWDMONITOR_STARTMONITOR_EXIT );
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

// -----------------------------------------------------------------------------
// CWDMonitor::HandlecentrepL()
// -----------------------------------------------------------------------------
//
void CWDMonitor::HandlecentrepL( TUint32 aKey )
    {
    CRepository* wdrepo = CRepository::NewL( KWDrepoUidMenu );        
    if ( KHarvesterServerKey == aKey )
        {
        // get the harvester server status
        wdrepo->Get( KHarvesterServerKey , iAllowHS );        
        }
    else if ( KSearchServerKey == aKey )
        {
        // get the Search server status
        wdrepo->Get( KSearchServerKey , iAllowSS );        
        }
    delete wdrepo;
    }
//End of file
