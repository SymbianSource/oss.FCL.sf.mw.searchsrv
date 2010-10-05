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

#include "cactivitymanager.h"
#include <f32file.h>
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cactivitymanagerTraces.h"
#endif


// ---------------------------------------------------------------------------
// CActivityManager::NewL()
// ---------------------------------------------------------------------------
//
CActivityManager* CActivityManager::NewL(MActivityManagerObserver* aObserver, TInt aTimeout)
    {    
    OstTraceFunctionEntry0( CACTIVITYMANAGER_NEWL_ENTRY );
    CActivityManager* self = new (ELeave) CActivityManager(aObserver, aTimeout);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( CACTIVITYMANAGER_NEWL_EXIT );
    return self;
    }
 
// ---------------------------------------------------------------------------
// CActivityManager::CActivityManager()
// ---------------------------------------------------------------------------
//
CActivityManager::CActivityManager(MActivityManagerObserver* aObserver, TInt aTimeout)
: CActive(CActive::EPriorityHigh), iObserver(aObserver), iTimeout(aTimeout), iPreviousStatus(KErrNotFound)
    {   
    OstTraceFunctionEntry0( CACTIVITYMANAGER_CACTIVITYMANAGER_ENTRY );
    CActiveScheduler::Add(this);
    OstTraceFunctionExit0( CACTIVITYMANAGER_CACTIVITYMANAGER_EXIT );
    }

// ---------------------------------------------------------------------------
// CActivityManager::~CActivityManager()
// ---------------------------------------------------------------------------
//
CActivityManager::~CActivityManager()
    {
    OstTraceFunctionEntry0( DUP1_CACTIVITYMANAGER_CACTIVITYMANAGER_ENTRY );
    
    iObserver = NULL;
    Cancel();
    
#ifdef MONITOR_LIGHTS
    delete iLight;
    iLight = NULL;
#endif

    iTimer.Cancel();
    iTimer.Close();
    OstTraceFunctionExit0( DUP1_CACTIVITYMANAGER_CACTIVITYMANAGER_EXIT );
    }
 
// ---------------------------------------------------------------------------
// CActivityManager::ConstructL()
// ---------------------------------------------------------------------------
//
void CActivityManager::ConstructL()
    {    
    iTimer.CreateLocal();
    iWatch = ENone;
    }

// ---------------------------------------------------------------------------
// CTMActivityManager::SetTimeout()
// ---------------------------------------------------------------------------
//
void CActivityManager::SetTimeout(TInt aTimeout)
    {    
    iTimeout = aTimeout;
    Reset();
    }

// ---------------------------------------------------------------------------
// CActivityManager::Reset()
// ---------------------------------------------------------------------------
//
void CActivityManager::Reset()
    {    
#ifdef MONITOR_LIGHTS
    delete iLight;
    iLight = NULL;
#endif
    
    Cancel();
    Start();
    }
 
// ---------------------------------------------------------------------------
// CActivityManager::DoCancel()
// ---------------------------------------------------------------------------
void CActivityManager::DoCancel()
    {
    
#ifdef MONITOR_LIGHTS
    delete iLight;
    iLight = NULL;
#endif
    
    iTimer.Cancel();
    iWatch = ENone;
    }

// ---------------------------------------------------------------------------
// CActivityManager::Start()
// ---------------------------------------------------------------------------
//
void CActivityManager::Start()
    {
    OstTraceFunctionEntry0( CACTIVITYMANAGER_START_ENTRY );
    
#ifdef MONITOR_LIGHTS
    if(!iLight)
        {
        TRAP_IGNORE(iLight = CHWRMLight::NewL(this));
        }
#endif
    
    if ( iWatch == ENone )
        {
        iFirstRound = ETrue;
        
        if( !IsActive() )
            {
            SetActive();
            TRequestStatus* statusPtr = &iStatus;
            User::RequestComplete( statusPtr, KErrNone );
            }
        }
    OstTraceFunctionExit0( CACTIVITYMANAGER_START_EXIT );
    }

// ---------------------------------------------------------------------------
// CActivityManager::RunL()
// ---------------------------------------------------------------------------
//
void CActivityManager::RunL()
    {    
    OstTraceFunctionEntry0( CACTIVITYMANAGER_RUNL_ENTRY );
    if(iFirstRound)
        {        
        iFirstRound = EFalse;
        if (!IsActive())
             {
             iWatch = EWaitingForInactivity;
             iTimer.Inactivity(iStatus, iTimeout);
             SetActive();
             }
        NotifyObserver();
        OstTraceFunctionExit0( CACTIVITYMANAGER_RUNL_EXIT );
        return;
        }
    
    if (iStatus == KErrNone)
        {
        if (iWatch == EWaitingForInactivity)
            {
            TInt inactivity = User::InactivityTime().Int();
            if (inactivity >= iTimeout)
                {                
                NotifyObserver();

                if (!IsActive()) //observer might have called a Reset()
                    {
                    iTimer.Inactivity(iStatus,0);
                    iWatch = EWaitingForActivity;
                    }
                }
            else
                {
                iTimer.Inactivity(iStatus,iTimeout);
                }
            }
        else if (iWatch == EWaitingForActivity)
            {            
            NotifyObserver();
             
            if (!IsActive()) //observer might have called a Reset()
                {
                iTimer.Inactivity(iStatus,iTimeout);
                iWatch = EWaitingForInactivity;
                }
            }
    
        if (!IsActive()) //observer might have called a Reset()
            {
            SetActive();
            }
        }
    else
        {
        iWatch = ENone;
        }      
    OstTraceFunctionExit0( DUP1_CACTIVITYMANAGER_RUNL_EXIT );
    }

// ---------------------------------------------------------------------------
// CActivityManager::RunError()
// ---------------------------------------------------------------------------
//
TInt CActivityManager::RunError(TInt aError)
    {    
    OstTraceFunctionEntry0( CACTIVITYMANAGER_RUNERROR_ENTRY );
    if (aError != KErrNone)
        {
        Reset();
        }
    
    // nothing to do
    OstTraceFunctionExit0( CACTIVITYMANAGER_RUNERROR_EXIT );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// IsInactive()
// -----------------------------------------------------------------------------
//
TBool CActivityManager::IsInactive()
    {
    OstTraceFunctionEntry0( CACTIVITYMANAGER_ISINACTIVE_ENTRY );
    
#ifdef MONITOR_LIGHTS
#ifdef _DEBUG
TInt inactivityTime = User::InactivityTime().Int();
#endif
#else
TInt inactivityTime = User::InactivityTime().Int();
#endif

    //if lights are off or inactivity timer is less that target the device is not idle
    if( User::InactivityTime() >= TTimeIntervalSeconds(iTimeout)
#ifdef MONITOR_LIGHTS
            || !iLights
#endif
            )
      {      
      OstTraceFunctionExit0( CACTIVITYMANAGER_ISINACTIVE_EXIT );
      return ETrue;
      }    
    OstTraceFunctionExit0( DUP1_CACTIVITYMANAGER_ISINACTIVE_EXIT );
    return EFalse;
    }

#ifdef MONITOR_LIGHTS
// -----------------------------------------------------------------------------
// LightStatusChanged()
// -----------------------------------------------------------------------------
//
void CActivityManager::LightStatusChanged(TInt aTarget, CHWRMLight::TLightStatus aStatus)
    {    
    if(aTarget & CHWRMLight::EPrimaryDisplay)
        {
         if( aStatus == CHWRMLight::ELightOff )
            {
            iLights = EFalse;
            }
        else
            {
            iLights = ETrue;
            }
     
        NotifyObserver();
        }
    }
#endif

// -----------------------------------------------------------------------------
// NotifyObserver()
// -----------------------------------------------------------------------------
//
void CActivityManager::NotifyObserver()
    {    
    OstTraceFunctionEntry0( CACTIVITYMANAGER_NOTIFYOBSERVER_ENTRY );
    TBool status = IsInactive();
    OstTrace1( TRACE_NORMAL, CACTIVITYMANAGER_NOTIFYOBSERVER, "CActivityManager::NotifyObserver;status=%d", status );
    
    if( iPreviousStatus != status )
        {
        iPreviousStatus = status; 
        if (iObserver)
            {
            iObserver->ActivityChanged(status);
            }
        }    
    OstTraceFunctionExit0( CACTIVITYMANAGER_NOTIFYOBSERVER_EXIT );
    }
