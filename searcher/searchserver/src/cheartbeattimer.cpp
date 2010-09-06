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

#include "CHeartBeatTimer.h"
#include "MHeartBeatObserver.h"
#include "SearchServerConfiguration.h"

CHeartBeatTimer* CHeartBeatTimer::NewL(MHeartBeatObserver* aObserver)
	{
	CHeartBeatTimer* object = new ( ELeave ) CHeartBeatTimer(aObserver);
	CleanupStack::PushL(object);
	object->ConstructL();
	CleanupStack::Pop(object);
	return object;
	}

CHeartBeatTimer::CHeartBeatTimer(MHeartBeatObserver* aObserver)
	: CTimer(CActive::EPriorityStandard)
	{
	iObserver = aObserver;
	}
	
CHeartBeatTimer::~CHeartBeatTimer()
	{
	Cancel();
	}
   
void CHeartBeatTimer::StopTimer()
	{
	Cancel();
	}

void CHeartBeatTimer::ContinueTimer()
	{
	Cancel();
	After(HEARTBEAT_PERIOD_USEC);
	}
   
void CHeartBeatTimer::ConstructL()
	{
	CTimer::ConstructL();
	CActiveScheduler::Add(this);
	After(HEARTBEAT_PERIOD_USEC);
	}

void CHeartBeatTimer::RunL()
	{
	if (iObserver)
		iObserver->HandleHeartBeatL();
	//After(HEARTBEAT_PERIOD_USEC);
	}
