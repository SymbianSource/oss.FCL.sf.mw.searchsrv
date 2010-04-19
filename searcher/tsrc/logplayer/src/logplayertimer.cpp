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


//  Include Files
#include "LogPlayerTimer.h"

// Contants
// None

// ========================= MEMBER FUNCTIONS ==================================

// -----------------------------------------------------------------------------
// CLogPlayerTimer::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CLogPlayerTimer* CLogPlayerTimer::NewL(MLogPlayerTimeCallback* aCallback)
    {
	CLogPlayerTimer* logPlayerTimer = CLogPlayerTimer::NewLC(aCallback);
    CleanupStack::Pop(logPlayerTimer);
    return logPlayerTimer;
    }

// -----------------------------------------------------------------------------
// CLogPlayerTimer::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CLogPlayerTimer* CLogPlayerTimer::NewLC(MLogPlayerTimeCallback* aCallback)
    {
	CLogPlayerTimer* logPlayerTimer = new (ELeave) CLogPlayerTimer(aCallback);
    CleanupStack::PushL(logPlayerTimer);
    logPlayerTimer->ConstructL();
    return logPlayerTimer;
    }

// -----------------------------------------------------------------------------
// CLogPlayerTimer::~CLogPlayerTimer()
// Destructor.
// -----------------------------------------------------------------------------
//
CLogPlayerTimer::~CLogPlayerTimer()
    {
    }

void CLogPlayerTimer::RunL()
    {
	iTimer->TimerCallback();
    }

void CLogPlayerTimer::StartTimer(const TTime& aTime)
    {
	At(aTime);
    }
// -----------------------------------------------------------------------------
// CLogPlayerTimer::CLogPlayerTimer()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CLogPlayerTimer::CLogPlayerTimer(MLogPlayerTimeCallback* aTimer)
	: CTimer(EPriorityStandard), iTimer(aTimer)
    {
    }

// -----------------------------------------------------------------------------
// CLogPlayerTimer::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CLogPlayerTimer::ConstructL()
    {
	CTimer::ConstructL();
	CActiveScheduler::Add(this);
    }
