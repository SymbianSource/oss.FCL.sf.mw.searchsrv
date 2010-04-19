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

#ifndef AOTESTCLASS_H
#define AOTESTCLASS_H

#include <e32base.h>	// For CActive, link against: euser.lib
#include <e32std.h>		// For RTimer, link against: euser.lib

class MAOTestObserver
	{
public:
	virtual void CallCompleted( int i ) = 0;
	};

class CAOTestClass : public CActive
	{
public:
	// Cancel and destroy
	~CAOTestClass();

	// Two-phased constructor.
	static CAOTestClass* NewL(MAOTestObserver* aObserver);

	// Two-phased constructor.
	static CAOTestClass* NewLC(MAOTestObserver* aOobserver);

public:
	// New functions
	// Function for making the initial request
	void StartL(TTimeIntervalMicroSeconds32 aDelay);

private:
	// C++ constructor
	CAOTestClass(MAOTestObserver* aObserver);

	// Second-phase constructor
	void ConstructL();

private:
	// From CActive
	// Handle completion
	void RunL();

	// How to cancel me
	void DoCancel();

	// Override to handle leaves from RunL(). Default implementation causes
	// the active scheduler to panic.
	TInt RunError(TInt aError);

private:
	enum TAOTestClassState
		{
		EUninitialized, // Uninitialized
		EInitialized, // Initalized
		EError
		// Error condition
		};

private:
	TInt iState; // State of the active object
	RTimer iTimer; // Provides async timing service
	MAOTestObserver* iObserver;

	};

#endif // AOTESTCLASS_H
