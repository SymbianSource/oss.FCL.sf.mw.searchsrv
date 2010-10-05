/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description: Monitoring application for servers restarting & IAD
*
*/


#include "servermonitor.h"

_LIT(KStar , "*");
_LIT(KExtension, ".exe");
_LIT(KSemaphore, "Semaphore");

// ======== MEMBER FUNCTIONS ========
// ---------------------------------------------------------------------------
// Constructor.
// ---------------------------------------------------------------------------
//
CServerMonitor* CServerMonitor::NewL( TDesC& aServerName , TUid aServerUid )
    {   
    CServerMonitor* self = new ( ELeave ) CServerMonitor();
    CleanupStack::PushL( self );
    self->ConstructL( aServerName , aServerUid );
    CleanupStack::Pop( self );

    return self;
    }

// ---------------------------------------------------------------------------
// C++ constructor.
// ---------------------------------------------------------------------------
//
CServerMonitor::CServerMonitor() : 
    CActive( CActive::EPriorityStandard ), iState(EIdle)
    {
    // Add to active scheduler.
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------------------------
// 2nd phase constructor.
// ---------------------------------------------------------------------------
//
void CServerMonitor::ConstructL( TDesC& aServerName , TUid aServerUid )
    {
        
    TFullName name;
    //Update the member variables
    iServerName = HBufC::NewL( aServerName.Length() );
    TPtr servernameptr = iServerName->Des();
    servernameptr.Copy( aServerName );
    
    iServerUid.iUid = aServerUid.iUid;
    // find Harvester Server
    // Append * to the server name for the findprocess
    HBufC* servername = HBufC::NewL( iServerName->Length() + 1 );
    TPtr nameptr = servername->Des();
    nameptr.Copy( aServerName );
    nameptr.Append( KStar );
    
    TFindProcess findProcess( *servername );
    if ( findProcess.Next(name) == KErrNone )
        {              
        User::LeaveIfError( iProcess.Open(name) );
        iState = ERunning;
        // logon to get termination signal
        iProcess.Logon(iStatus);        
        SetActive();
        }        
    else
        { 
        // start new Harvester            
        StartL();
        }
    
    delete servername;
    
    }

void CServerMonitor::StartL()
    {  

    // Double check that harvester server is not already running
    TFullName name;
    TInt res( KErrNone );
    // Append * to the server name for the findprocess
    HBufC* servername = HBufC::NewL( iServerName->Length() + 1 );
    TPtr servernameptr = servername->Des();
    servernameptr.Copy( *iServerName );
    servernameptr.Append( KStar );
    // find really Harvester Server, using TFindServer to avoid logon a dead process
    TFindServer findServer( *servername );
    
    if ( findServer.Next(name) == KErrNone )
        {
        TFindProcess findProcess( *servername );
        if ( findProcess.Next(name) == KErrNone )
            {            
            iProcess.Close();
            res = iProcess.Open(name);
            if ( res != KErrNone )
                {
                delete servername;
                return;
                }
            iState = EWaitingRendezvous;
            SetActive();
            delete servername;
            return;
            }
        }  
    
    // close the panic process
    TFindProcess findProcess( *servername );
    if ( findProcess.Next(name) == KErrNone )
        {                
        iProcess.Close();
        }
    delete servername;
    servername = NULL;
    // Create the server process
    StartServerProcessL(); 
    }    

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CServerMonitor::~CServerMonitor()
    {
    delete iServerName;
    Cancel();
    }

// ---------------------------------------------------------------------------
// Active object's request handling.
// ---------------------------------------------------------------------------
//
void CServerMonitor::RunL()
    {    	
    	switch (iState)
    	{
          case EIdle:
              {
              break;
              }
              
          case EWaitingRendezvous:          
              {
              iState = ERunning;              
              // logon to get termination signal
              TRAPD(err,iProcess.Logon(iStatus));
              if ( err == KErrNone )
                 SetActive();
              break;
              }             
              
          case ERunning:
              {
              // server died unexpectedly, start it    
              StartL();
              break;
              }            
          default:
              break; 
          
    	}
    
    }

// ---------------------------------------------------------------------------
// Active object's request error handling.
// ---------------------------------------------------------------------------
//
TInt CServerMonitor::RunError( TInt /*aError*/ )
    {
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Cancel the request.
// ---------------------------------------------------------------------------
//
void CServerMonitor::DoCancel()
    {
    iProcess.LogonCancel(iStatus);
    iProcess.Close();
    }

// ---------------------------------------------------------------------------
// StartServerProcess
// ---------------------------------------------------------------------------
//
void CServerMonitor::StartServerProcessL()
    {
    RSemaphore semaphore;
    HBufC* semaphorename = HBufC::NewL( iServerName->Length() + 10 );
    TPtr semptr = semaphorename->Des();
    semptr.Copy( *iServerName );
    semptr.Append( KSemaphore );
    
    TInt result = semaphore.CreateGlobal( *semaphorename, 0 );
    if (result != KErrNone)
        {
        delete semaphorename;
        return ;
        }

    TRAP(result, CreateServerProcess());
    if (result != KErrNone)
        {
        return ;
        }

    semaphore.Wait();
    semaphore.Close();
    delete semaphorename;
    return ;
    }
        
// ---------------------------------------------------------------------------
// CreateServerProcess
// ---------------------------------------------------------------------------
//
void CServerMonitor::CreateServerProcess()
    {
    TInt result;

    const TUidType serverUid( KNullUid, KNullUid, iServerUid);
    
    HBufC* servernameext = HBufC::NewL( iServerName->Length() + 4 );
    TPtr ptr = servernameext->Des();
    ptr.Copy( *iServerName );
    ptr.Append( KExtension );
    //result = iProcess.Create( *iServerName, KNullDesC, serverUid);
    result = iProcess.Create( *iServerName, KNullDesC);
    if (result != KErrNone)
        {
        delete servernameext;
        return ;
        }


    // start process and wait until it is constructed                
    iProcess.Rendezvous(iStatus);
    
    if( iStatus != KRequestPending )
        {
        iProcess.Kill( 0 );   // abort startup
        }
    else
        {    
        iProcess.Resume();    // logon OK - start the server   
        iState = EWaitingRendezvous;
        SetActive();
        
        }
    delete servernameext;
    return ;
    }

// ---------------------------------------------------------------------------
// CreateServerProcess
// ---------------------------------------------------------------------------
//
void CServerMonitor::ShutdownServer()
    {
    HBufC* servername = HBufC16::NewLC( iServerName->Length() + 5 );
    TPtr servernameptr = servername->Des();
    servernameptr.Copy( *iServerName );
    servernameptr.Append( KExtension );
    servernameptr.Append( KStar );
    TFindProcess processFinder( *servername );
    TFullName result;
    RProcess processHandle;
    while ( processFinder.Next(result) == KErrNone) 
    {
       User::LeaveIfError(processHandle.Open ( processFinder, EOwnerThread));
       processHandle.Kill(KErrNone);
       processHandle.Close();
    }
    CleanupStack::PopAndDestroy( servername );
    }
