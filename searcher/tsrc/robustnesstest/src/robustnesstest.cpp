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

#include <e32base.h>
#include <e32std.h>
#include <e32cons.h>			// Console

#include "CRobustnessTest.h"

//  Constants

_LIT(KTextConsoleTitle, "Console");
_LIT(KTextFailed, " failed, leave code = %d");
_LIT(KTextPressAnyKey, " [press any key]\n");

//  Global Variables

LOCAL_D CConsoleBase* console; // write all messages to this

//  Local Functions

class CRobustnessTestDriver : public CBase, public MRobustnessTestObserver
	{
public: 
	void RunL() 
		{
		iTest = new ( ELeave ) CRobustnessTest( *console, *this ); 
		iTest->ConstructL(); 
		iTest->StartL(); 
		CActiveScheduler::Start(); 
		}
	~CRobustnessTestDriver() 
		{
		delete iTest;
		}
public: // from CRobustnessTestObserver
	
	void FinishedL(CRobustnessTest& )
		{
		CActiveScheduler::Stop(); 
		}
	
private: 
	
	CRobustnessTest* iTest; 

	};


LOCAL_C void MainL()
	{
	CRobustnessTestDriver* driver = new ( ELeave ) CRobustnessTestDriver(); 
	CleanupStack::PushL( driver ); 
	driver->RunL(); 
	CleanupStack::PopAndDestroy( driver );
	}

LOCAL_C void DoStartL()
	{
	// Create active scheduler (to run active objects)
	CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
	CleanupStack::PushL(scheduler);
	CActiveScheduler::Install(scheduler);

	MainL();
	
	// Delete active scheduler
	CleanupStack::PopAndDestroy(scheduler);
	}

//  Global Functions

GLDEF_C TInt E32Main()
	{
	// Create cleanup stack
	__UHEAP_MARK;
	CTrapCleanup* cleanup = CTrapCleanup::New();

	// Create output console
	TRAPD(createError, console = Console::NewL(KTextConsoleTitle, TSize(KConsFullScreen,KConsFullScreen)));
	if (createError)
	return createError;

	// Run application code inside TRAP harness, wait keypress when terminated
	TRAPD(mainError, DoStartL());
	if (mainError)
	console->Printf(KTextFailed, mainError);
	console->Printf(KTextPressAnyKey);
	console->Getch();

	delete console;
	delete cleanup;
	__UHEAP_MARKEND;
	return KErrNone;
	}

