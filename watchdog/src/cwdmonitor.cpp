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
#include "CWDTimer.h"
#include <HarvesterServerLogger.h>
#include <centralrepository.h>
#include <cpixwatchdogcommon.h>
#include "centrepmonitor.h"
#include "servermonitor.h"
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
    delete aHSStatusMonitor;
    delete aSSStatusMonitor;
    delete aHSUidMonitor;
    delete aSSUidMonitor;
    delete aHSNameMonitor;
    delete aSSNameMonitor;
    delete iHarvesterMonitor;
    delete iSearchMonitor;
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
    
    aHSStatusMonitor = CentrepMonitor::NewL( this, KHarvesterServerKey);
    aHSStatusMonitor->StartNotifier();
    aSSStatusMonitor = CentrepMonitor::NewL( this, KSearchServerKey);
    aSSStatusMonitor->StartNotifier();
    aHSUidMonitor = CentrepMonitor::NewL( this, KHarvesterServerUIDKey);
    aHSUidMonitor->StartNotifier();
    aSSUidMonitor = CentrepMonitor::NewL( this, KSearchServerUIDKey);
    aSSUidMonitor->StartNotifier();
    aHSNameMonitor = CentrepMonitor::NewL( this, KHarvesterServerNAMEKey);
    aHSNameMonitor->StartNotifier();
    aSSNameMonitor = CentrepMonitor::NewL( this, KSearchServerNAMEKey);
    aSSNameMonitor->StartNotifier();
    
    }

// -----------------------------------------------------------------------------
// CWDMonitor::StartMonitor()
// -----------------------------------------------------------------------------
//
void CWDMonitor::HandleWDTimerL()
    {
    OstTrace0( TRACE_NORMAL, CWDMONITOR_HANDLEWDTIMERL, "CWDMonitor::HandleWDTimerL(): Check the servers" );
    CPIXLOGSTRING("CWDMonitor::HandleWDTimerL(): Check the servers");

    if ( !iHarvesterMonitor )
        {
        if ( iAllowHS )
            iHarvesterMonitor = CServerMonitor::NewL( *iHSName , iHSUid );
        iWDTimer->StartWDTimer();
        return;
        }
    if ( iAllowSS )
        iSearchMonitor = CServerMonitor::NewL( *iSSName , iSSUid );
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
        
        if ( iAllowHS )
            {
            //start the Harvester server and start monitoring            
            iHarvesterMonitor = CServerMonitor::NewL( *iHSName , iHSUid );
            }
        else{
            // Shutdown the harvester server
            iHarvesterMonitor->Cancel();
            iHarvesterMonitor->ShutdownServer();                        
            delete iHarvesterMonitor;
            iHarvesterMonitor = NULL;
            }
        }
    else if ( KSearchServerKey == aKey )
        {
        // get the Search server status
        wdrepo->Get( KSearchServerKey , iAllowSS );
        
        if ( iAllowSS )
            {
            //Start the Search server and start monitoring
            iSearchMonitor = CServerMonitor::NewL( *iSSName , iSSUid );
            }
        else
            {
            //Shutdown the searchserver
            iSearchMonitor->Cancel();
            iSearchMonitor->ShutdownServer();                        
            delete iSearchMonitor;
            iSearchMonitor = NULL;
            }
        }
    else if ( (KHarvesterServerUIDKey == aKey)||( KHarvesterServerNAMEKey == aKey ) )
        {
        // Shutdown the existing server
        iHarvesterMonitor->Cancel();
        iHarvesterMonitor->ShutdownServer();                        
        delete iHarvesterMonitor;
        iHarvesterMonitor = NULL;
        //read the new values
        TBuf<KCenrepUidLength> temp;
        TInt64 value;
        TLex uidvalue;
        if ( KHarvesterServerUIDKey == aKey )
            {            
            //Read Harvester server UId value
            if ( KErrNone == wdrepo->Get( KHarvesterServerUIDKey, temp ) )
                {
                uidvalue.Assign(temp);        
                if (KErrNone == uidvalue.Val( value,EHex ))
                    iHSUid.iUid = value;
                }
            }
        else
            {
            //read harvester server name
            delete iHSName;
            iHSName = NULL;
            if ( KErrNone == wdrepo->Get( KHarvesterServerNAMEKey, temp ))
                {
                iHSName = HBufC::NewL( temp.Length() );
                TPtr hsname = iHSName->Des();
                hsname.Copy( temp );
                }
            }        
        // Start the server with new values
        iHarvesterMonitor = CServerMonitor::NewL( *iHSName , iHSUid );
        }
    else if ( (KSearchServerUIDKey == aKey)||( KSearchServerNAMEKey == aKey ) )
        {
        // Shutdown the existing server
        iSearchMonitor->Cancel();
        iSearchMonitor->ShutdownServer();                        
        delete iSearchMonitor;
        iSearchMonitor = NULL;
        //read the new values
        TBuf<KCenrepUidLength> temp;
        TInt64 value;
        TLex uidvalue;
        if ( KSearchServerUIDKey == aKey )
            {
            //read search server uid
            if ( KErrNone == wdrepo->Get( KSearchServerUIDKey, temp ) )
                {
                uidvalue.Assign(temp);        
                if (KErrNone == uidvalue.Val( value,EHex ))
                    iSSUid.iUid = value;
                }
            }
        else
            {
            //read search server name
            delete iSSName;
            iSSName = NULL;
            if ( KErrNone == wdrepo->Get( KSearchServerNAMEKey, temp ))
                {
                iSSName = HBufC::NewL( temp.Length() );
                TPtr ssname = iSSName->Des();
                ssname.Copy( temp );
                }
            }        
        // Start the server with new values
        iSearchMonitor = CServerMonitor::NewL( *iSSName , iSSUid );
        }
    delete wdrepo;
    }
//End of file
