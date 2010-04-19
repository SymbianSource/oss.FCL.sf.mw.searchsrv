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

// INCLUDES
#include "DelayedCallback.h"


// ---------------------------------------------------------------------------
// CDelayedCallback::NewL
// Creates new instance of CDelayedCallback
// ---------------------------------------------------------------------------    
EXPORT_C CDelayedCallback* CDelayedCallback::NewL(TInt aPriority)
    {
    CDelayedCallback* self = new(ELeave) CDelayedCallback(aPriority);
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }
    
// ---------------------------------------------------------------------------
// CDelayedCallback::~CDelayedCallback
// Destructor.
// ---------------------------------------------------------------------------
EXPORT_C CDelayedCallback::~CDelayedCallback()
    {
    if ( iTimer )
        {
        iTimer->Cancel();
        delete iTimer;
        iTimer = NULL;
        }
	iObserver = NULL;
    Cancel();
    }
    
// ---------------------------------------------------------------------------
// CDelayedCallback::RunL
// From CActive, RunL
// ---------------------------------------------------------------------------
void CDelayedCallback::RunL()
    {
    User::LeaveIfError( iStatus.Int() );
    if ( iObserver )
        {
        iObserver->DelayedCallbackL(iCode);
        }
    }
    
// ---------------------------------------------------------------------------
// CDelayedCallback::DoCancel
// From CActive, DoCancel
// ---------------------------------------------------------------------------
void CDelayedCallback::DoCancel()
    {
    if ( iTimer )
        {
        if ( iTimer->IsActive() )
            {
            iTimer->Cancel();
            }
        delete iTimer;
        iTimer = NULL;
        }
    }
    
// ---------------------------------------------------------------------------
// CDelayedCallback::RunError
// From CActive, RunError
// ---------------------------------------------------------------------------
TInt CDelayedCallback::RunError(TInt aError)
    {
    if (iObserver)
        iObserver->DelayedError(aError);
    return KErrNone;
    }
    
// ---------------------------------------------------------------------------
// CDelayedCallback::Start
// Activates this callback object.
// ---------------------------------------------------------------------------
EXPORT_C void CDelayedCallback::Start(TInt aCode, MDelayedCallbackObserver* aObserver,
                                      TInt aDelay)
    {
    iCode = aCode;
    iObserver = aObserver;
    if ( CallbackPending() )
        {
        return;
        }
        
    if ( aDelay <= 0 )
        {
        if ( !IsAdded() )
            {
            CActiveScheduler::Add( this );
            }
        SetActive();
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        }
    else
        {
        if ( iTimer )
            {
            iTimer->Start( aDelay, aDelay, TCallBack(StaticTimerCallback, this) );
            }
        }
    }
    
// ---------------------------------------------------------------------------
// CDelayedCallback::CancelCallback
// Cancels the pending callback request.
// ---------------------------------------------------------------------------
EXPORT_C void CDelayedCallback::CancelCallback()
    {
    Cancel();
    if ( iTimer )
        {
        iTimer->Cancel();
        }
    }
    
// ---------------------------------------------------------------------------
// CDelayedCallback::CallbackPending
// Tells if delayed callback request is pending.
// ---------------------------------------------------------------------------
EXPORT_C TBool CDelayedCallback::CallbackPending() const
    {
    TBool pending = IsActive();
    if ( iTimer )
        {
        pending |= iTimer->IsActive();
        }
    return pending;
    }

// ---------------------------------------------------------------------------
// CDelayedCallback::StaticTimerCallback
// The actual callback function of CPeriodic
// ---------------------------------------------------------------------------
TInt CDelayedCallback::StaticTimerCallback(TAny* aObject)
    {
    reinterpret_cast<CDelayedCallback*>(aObject)->TimerCallback();
    return 1;
    }
    
// ---------------------------------------------------------------------------
// CDelayedCallback::TimerCallback
// The CPeriodic callback method that just calls the observer.
// ---------------------------------------------------------------------------
void CDelayedCallback::TimerCallback()
    {
    if ( iTimer )
        {
        iTimer->Cancel();
        }
    
    if ( iObserver )
        {    
        TInt err = KErrNone;
        TRAP(err, iObserver->DelayedCallbackL(iCode));
        if (err != KErrNone)
        	iObserver->DelayedError(err);
        }
    }
    
// ---------------------------------------------------------------------------
// CDelayedCallback::CDelayedCallback
// Constructor.
// ---------------------------------------------------------------------------
CDelayedCallback::CDelayedCallback(TInt aPriority) :
    CActive( aPriority )
    {
    }
    
// ---------------------------------------------------------------------------
// CDelayedCallback::ConstructL
// 2nd phase constructor.
// ---------------------------------------------------------------------------
void CDelayedCallback::ConstructL()
    {
    iTimer = CPeriodic::NewL( Priority() );
    }
    
// End of file
