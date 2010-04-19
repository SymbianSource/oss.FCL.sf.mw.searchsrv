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

#include "AOTestClass.h"

CAOTestClass::CAOTestClass(MAOTestObserver* aObserver) :
	CActive(EPriorityStandard) // Standard priority
	{
	iObserver = aObserver;
	}

CAOTestClass* CAOTestClass::NewLC(MAOTestObserver* aObserver)
	{
	CAOTestClass* self = new (ELeave) CAOTestClass(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CAOTestClass* CAOTestClass::NewL(MAOTestObserver* aObserver)
	{
	CAOTestClass* self = CAOTestClass::NewLC(aObserver);
	CleanupStack::Pop(); // self;
	return self;
	}

void CAOTestClass::ConstructL()
	{
	User::LeaveIfError(iTimer.CreateLocal()); // Initialize timer
	CActiveScheduler::Add(this); // Add to scheduler
	}

CAOTestClass::~CAOTestClass()
	{
	Cancel(); // Cancel any request, if outstanding
	iTimer.Close(); // Destroy the RTimer object
	// Delete instance variables if any
	}

void CAOTestClass::DoCancel()
	{
	iTimer.Cancel();
	}

void CAOTestClass::StartL(TTimeIntervalMicroSeconds32 aDelay)
	{
	Cancel(); // Cancel any request, just to be sure
	iState = EUninitialized;
	iTimer.After(iStatus, aDelay); // Set for later
	SetActive(); // Tell scheduler a request is active
	}

void CAOTestClass::RunL()
	{
	if (iState == EUninitialized)
		{
		// Do something the first time RunL() is called
		iState = EInitialized;
		}
	else if (iState != EError)
		{
		// Do something
		}
	iObserver->CallCompleted( 2 );
	}

TInt CAOTestClass::RunError(TInt aError)
	{
	return aError;
	}
