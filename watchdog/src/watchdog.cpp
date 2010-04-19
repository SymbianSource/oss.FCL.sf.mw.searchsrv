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
* Description:  This application is to monitor Harvester and Search Server
 *
*/


//  Include Files  
#include <e32base.h>
#include <e32std.h>
#include "CWDMonitor.h"
#include "WatchDog.h"

//  Constants
//  Global Variables
//  Local Functions

LOCAL_C void DoStartL()
    {
    // Create active scheduler (to run active objects)
    CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
    CleanupStack::PushL(scheduler);
    CActiveScheduler::Install(scheduler);
    //create the watch dog monitor
    CWDMonitor* monitor = CWDMonitor::NewLC();
    //Start monitoring the Harvester and Search server
    monitor->StartMonitor();
    
    // Start handling requests
    CActiveScheduler::Start();
    //Delete the watch dog monitor
    CleanupStack::PopAndDestroy(monitor);
    // Delete active scheduler
    CleanupStack::PopAndDestroy(scheduler);
    }

//  Global Functions

GLDEF_C TInt E32Main()
    {
    // Create cleanup stack
    __UHEAP_MARK;
    CTrapCleanup* cleanup = CTrapCleanup::New();

    // Run application code inside TRAP harness
    TRAPD(mainError, DoStartL());
    
    if( mainError != KErrNone)
        {
        //Handle the error    
        }
    
    delete cleanup;
    __UHEAP_MARKEND;
    return KErrNone;
    }

