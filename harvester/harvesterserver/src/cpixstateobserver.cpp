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

#include "cpixstateobserver.h"
#include <mpxcollectionutility.h>
#include <mpxmessagegeneraldefs.h>
#include <mpxcollectionmessage.h>
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cpixstateobserverTraces.h"
#endif


// -----------------------------------------------------------------------------
// CPiXStateObserver::NewL()
// -----------------------------------------------------------------------------
//
CPiXStateObserver* CPiXStateObserver::NewL( MCPiXStateObserver* aObserver, RFs* aFsSession )
    {        
    CPiXStateObserver* self = new ( ELeave ) CPiXStateObserver(aObserver);
    CleanupStack::PushL( self );
    self->ConstructL( aFsSession );
    CleanupStack::Pop(self);
    return self;
    }


CPiXStateObserver::CPiXStateObserver(MCPiXStateObserver* aObserver)
            : iObserver(aObserver)                                
                    
    {
    iPreviousState = KErrNotFound;
    iMMCHarvesting = KErrNotFound;
    iMPXHarvesting = KErrNotFound;    
    }
// -----------------------------------------------------------------------------
// CPiXStateObserver::~CPiXStateObserver()
// -----------------------------------------------------------------------------
//
CPiXStateObserver::~CPiXStateObserver()
   {
   iHarvesterClient.RemoveHarvesterEventObserver(*this);
   iHarvesterClient.Close();
   }

// -----------------------------------------------------------------------------
// CPiXStateObserver::ConstructL()
// -----------------------------------------------------------------------------
//
void CPiXStateObserver::ConstructL( RFs* aFsSession )
    {    
    iFsSession = aFsSession;
    }

void CPiXStateObserver::StartMonitoringL()
    {
    TInt err(KErrNone);
    err = iHarvesterClient.Connect();    
    
    if(err == KErrNone)
        {
        err = iHarvesterClient.AddHarvesterEventObserver( *this, EHEObserverTypeMMC, 1000 );        
        }
    iCollectionUtility = MMPXCollectionUtility::NewL( this );

    }
// -----------------------------------------------------------------------------
// CPiXStateObserver::HarvestingUpdated()
// -----------------------------------------------------------------------------
//
void CPiXStateObserver::HarvestingUpdated( HarvesterEventObserverType aHEObserverType, 
             HarvesterEventState aHarvesterEventState, TInt aItemsLeft  )
    {    
    if(aHEObserverType == EHEObserverTypeMMC)
        {
        switch(aHarvesterEventState)
            {
            case EHEStateStarted:
            case EHEStateHarvesting:            
                {
                iMMCHarvesting = ETrue;                
                break;
                }
            case EHEStateFinished:
            case EHEStateUninitialized:
                {
                iMMCHarvesting = EFalse;                
                break;
                }
            default:
                break;
            }
        OstTrace0( TRACE_FLOW, CPIXSTATEOBSERVER_HARVESTINGUPDATED, "CPiXStateObserver::HarvestingUpdated MMC Event" );        
        NotifyObserverL();
        }    
    }

void CPiXStateObserver::HandleCollectionMessage( CMPXMessage* aMessage, TInt aError )
    {
    if ( aError != KErrNone )
        {
        return;
        }
    
    TMPXMessageId generalId( *aMessage->Value<TMPXMessageId>( KMPXMessageGeneralId ) );
    
    //we are interestead of only general system events
    if ( generalId == KMPXMessageGeneral )
        {
        TInt event( *aMessage->Value<TInt>( KMPXMessageGeneralEvent ) );
        TInt op( *aMessage->Value<TInt>( KMPXMessageGeneralType ) );
        
        if ( event == TMPXCollectionMessage::EBroadcastEvent )
            {
            switch( op )
                {
                //when MTP sync or music collection is started then pause processing
                case EMcMsgRefreshStart:
                case EMcMsgUSBMTPStart:
                    {
                    iMPXHarvesting = ETrue;
                    break;
                    }
                //when MTP sync or music collection refresh is complete then resume processing
                case EMcMsgRefreshEnd:
                case EMcMsgUSBMTPEnd:
                case EMcMsgUSBMTPNotActive:
                    {
                    iMPXHarvesting = EFalse;
                    break;
                    }
                default:
                    break;
                }
            OstTrace0( TRACE_FLOW, CPIXSTATEOBSERVER_HANDLECOLLECTIONMESSAGE, "CPiXStateObserver::HandleCollectionMessage MPX Event" );            
            NotifyObserverL();
            }        
        }
    }

void CPiXStateObserver::HandleOpenL( const CMPXMedia& /*aEntries*/, TInt /*aIndex*/,
                                               TBool /*aComplete*/, TInt /*aError*/ )
     {
     // not needed here
     }

void CPiXStateObserver::HandleOpenL( const CMPXCollectionPlaylist& /*aPlaylist*/, TInt /*aError*/ )
   {
   // not needed here
   }

void CPiXStateObserver::HandleCollectionMediaL( const CMPXMedia& /*aMedia*/,
                                                       TInt /*aError*/ )
    {
    // not needed here
    }

TBool CPiXStateObserver::GetStatus()
    {
    TBool ret = EFalse;
    if( iMMCHarvesting || iMPXHarvesting )
        ret = ETrue;
    return ret;        
    }

void CPiXStateObserver::NotifyObserverL()
    {
    TBool status = GetStatus();
    if( iPreviousState != status )
            {
            iPreviousState = status; 
            if (iObserver)
                {
                iObserver->HandleStateObserverChangeL(status);
                OstTrace1( TRACE_FLOW, CPIXSTATEOBSERVER_NOTIFYOBSERVERL, "CPiXStateObserver::NotifyObserverL status=%d", status );
                
                }
            }    
    }
