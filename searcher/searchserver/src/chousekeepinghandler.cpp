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

#include "CHouseKeepingHandler.h"
#include "CHeartBeatTimer.h"
#include "CLogPlayerRecorder.h"

#include "CCPixAsyncronizer.h"

TBool CHouseKeepingHandler::iCallBackValid = EFalse;

// -----------------------------------------------------------------------------
// CHouseKeepingHandler::NewL
// -----------------------------------------------------------------------------
//
CHouseKeepingHandler* CHouseKeepingHandler::NewL()
	{
	CHouseKeepingHandler* self = new ( ELeave ) CHouseKeepingHandler();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

// -----------------------------------------------------------------------------
// CHouseKeepingHandler::~CHouseKeepingHandler
// -----------------------------------------------------------------------------
//
CHouseKeepingHandler::~CHouseKeepingHandler()
	{
	iCallBackValid = EFalse;
	
	delete iHeartBeatTimer;
	
	delete iAsyncronizer;
	}

// -----------------------------------------------------------------------------
// CHouseKeepingHandler::StartL
// -----------------------------------------------------------------------------
//
void CHouseKeepingHandler::StartL()
    {
    if ( !iHeartBeatTimer )
        {
        iHeartBeatTimer = CHeartBeatTimer::NewL(this);
        }
    }

// -----------------------------------------------------------------------------
// CHouseKeepingHandler::StopHouseKeeping()
// Stop housekeeping
// -----------------------------------------------------------------------------
//
void CHouseKeepingHandler::StopHouseKeeping()
    {
    iHeartBeatTimer->StopTimer();
    }

// -----------------------------------------------------------------------------
// CHouseKeepingHandler::ContinueHouseKeeping()
// Continue housekeeping
// -----------------------------------------------------------------------------
//
void CHouseKeepingHandler::ContinueHouseKeeping()
    {
    iHeartBeatTimer->ContinueTimer();
    }

// -----------------------------------------------------------------------------
// CHouseKeepingHandler::ForceHouseKeeping()
// Force housekeeping
// -----------------------------------------------------------------------------
//
void CHouseKeepingHandler::ForceHouseKeeping()
    {
    DoHouseKeeping();
    }

// -----------------------------------------------------------------------------
// CHouseKeepingHandler::StartL
// -----------------------------------------------------------------------------
//
void CHouseKeepingHandler::CompletionCallback(void *aCookie, cpix_JobId aJobId)
    {
    if ( !iCallBackValid )
        {
        // CPiX layer could call this callback function after CHouseKeepingHandler
        // is already deleted.
        return;
        }
    
    CHouseKeepingHandler* object = (CHouseKeepingHandler*)aCookie;

    // Sanity check
    if (!object || object->iPendingJobId != aJobId)
        {
        return;
        }
    
    // Call the asyncronizers completion code
    CCPixAsyncronizer* asyncronizer = object->iAsyncronizer;
    asyncronizer->CompletionCallback();
    }


// -----------------------------------------------------------------------------
// CSearchServer::HandleHeartBeatL()
// -----------------------------------------------------------------------------
//
void CHouseKeepingHandler::HandleHeartBeatL()
    {
    DoHouseKeeping();
    }

// -----------------------------------------------------------------------------
// CSearchServer::HandleAsyncronizerComplete()
// -----------------------------------------------------------------------------
//
void CHouseKeepingHandler::HandleAsyncronizerComplete(TCPixTaskType /*aType*/,
        TInt aError, const RMessage2& /*aMessage*/)
    {
    iIsPending = EFalse;
    if (aError == KErrNone)
        {
        cpix_Result result;
        cpix_asyncDoHousekeepingResults(&result, iPendingJobId);
        cpix_ClearError(&result);
        }
    }

// -----------------------------------------------------------------------------
// CHouseKeepingHandler::CHouseKeepingHandler
// -----------------------------------------------------------------------------
//
CHouseKeepingHandler::CHouseKeepingHandler()
	{
	iCallBackValid = ETrue;
	}
	
// -----------------------------------------------------------------------------
// CHouseKeepingHandler::ConstructL
// -----------------------------------------------------------------------------
//
void CHouseKeepingHandler::ConstructL()
	{
	iAsyncronizer = CCPixAsyncronizer::NewL();
	}

// -----------------------------------------------------------------------------
// CHouseKeepingHandler::DoHouseKeeping
// -----------------------------------------------------------------------------
//
void CHouseKeepingHandler::DoHouseKeeping()
    {
    if ( iIsPending )
        {
        // Sanity check. If previous house keeping operation is still 
        // ongoing just ignore house keeping this time.
        return;
        }
    
    cpix_Result result;
    iPendingJobId = cpix_asyncDoHousekeeping(&result, (void*)this, &CompletionCallback);
    
    TRAP_IGNORE( LOG_PLAYER_RECORD( CLogPlayerRecorder::LogHouseKeepL() ) );
    
    if ( cpix_Failed( &result ) )
        {
        // Clean error to avoid memory leaks.
        cpix_ClearError(&result);
        }
    else
        {
        iAsyncronizer->Start(ECPixTaskTypeHouseKeeping, this, iMessage);
        iIsPending = ETrue;
        }
    }
