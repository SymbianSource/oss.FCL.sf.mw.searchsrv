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

#include <S32MEM.H>
#include <e32debug.H>
#include "CCPixAsyncronizer.h"

// -----------------------------------------------------------------------------
// CCPixAsyncronizer::NewL()
// -----------------------------------------------------------------------------
//
CCPixAsyncronizer* CCPixAsyncronizer::NewL()
    {
    CCPixAsyncronizer* self = CCPixAsyncronizer::NewLC();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CCPixAsyncronizer::NewLC()
// -----------------------------------------------------------------------------
//
CCPixAsyncronizer* CCPixAsyncronizer::NewLC()
    {
    CCPixAsyncronizer* self = new (ELeave) CCPixAsyncronizer;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CCPixAsyncronizer::~CCPixAsyncronizer()
// -----------------------------------------------------------------------------
//
CCPixAsyncronizer::~CCPixAsyncronizer()
	{
	Cancel();
	iMainThread.Close();
	}

// -----------------------------------------------------------------------------
// CCPixAsyncronizer::Start()
// -----------------------------------------------------------------------------
//
void CCPixAsyncronizer::Start(
        TCPixTaskType aType,
        MCPixAsyncronizerObserver* aObserver,
        const RMessage2& aMessage )
	{
	iType = aType;
	iMessage = aMessage;
	iObserver = aObserver;
	iStatus = KRequestPending;
	SetActive();
	}

// -----------------------------------------------------------------------------
// CCPixAsyncronizer::CancelWhenDone()
// -----------------------------------------------------------------------------
//
void CCPixAsyncronizer::CancelWhenDone(const RMessage2& aMessage)
    {
    if (IsActive())
        {
        // Cancelling uncancellable task. Just wait until task completes
        // and complete cancel request after that.
        iCancelMessage = aMessage;
        iCancelAllOperationPending = ETrue;
        }
    else
        {
        aMessage.Complete(KErrNone);
        }
    }

// -----------------------------------------------------------------------------
// CCPixAsyncronizer::CompletionCallback()
// -----------------------------------------------------------------------------
//
void CCPixAsyncronizer::CompletionCallback()
    {
    iWaitingForCompletion = ETrue;

	// CompletionCallback CAN BE and IS called from another thread, 
    // so care should be taken what kind of actions are done in here  
    TRequestStatus* status = &iStatus;
	
    if ( iStatus != KRequestPending )
        {
        // Cancellation already done
        return;
        }
    
    iMainThread.RequestComplete(status, KErrNone);
    }

// -----------------------------------------------------------------------------
// CCPixAsyncronizer::RunL()
// -----------------------------------------------------------------------------
//
void CCPixAsyncronizer::RunL()
	{
    iWaitingForCompletion = EFalse;

	if ( iCancelAllOperationPending )
	    {
	    iCancelAllOperationPending = EFalse;
	    iCancelMessage.Complete(KErrNone);
	    }
	
	NotifyObserver( iStatus.Int() );
	}

// -----------------------------------------------------------------------------
// CCPixAsyncronizer::DoCancel()
// -----------------------------------------------------------------------------
//
void CCPixAsyncronizer::DoCancel()
    {
	if ( iWaitingForCompletion )
		{
		return;
		}
	
    TRequestStatus* status = &iStatus;
    iMainThread.RequestComplete(status, KErrCancel);
    NotifyObserver(KErrCancel);
    }

// -----------------------------------------------------------------------------
// CCPixAsyncronizer::NotifyObserver()
// -----------------------------------------------------------------------------
//
void CCPixAsyncronizer::NotifyObserver(TInt aErrorCode)
    {
    if (iObserver)
        {
        iObserver->HandleAsyncronizerComplete(iType, aErrorCode, iMessage);
        }
    
    iObserver = NULL;
    }

// -----------------------------------------------------------------------------
// CCPixAsyncronizer::CCPixAsyncronizer()
// -----------------------------------------------------------------------------
//
CCPixAsyncronizer::CCPixAsyncronizer()
    : CActive(CActive::EPriorityHigh), iObserver(NULL)
	{
	CActiveScheduler::Add(this);
	}

// -----------------------------------------------------------------------------
// CCPixAsyncronizer::ConstructL()
// -----------------------------------------------------------------------------
//
void CCPixAsyncronizer::ConstructL()
	{
	User::LeaveIfError(iMainThread.Open(RThread().Id()));
	}
