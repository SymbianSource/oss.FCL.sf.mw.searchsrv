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


#ifndef CHOUSEKEEPINGHANDLER_H
#define CHOUSEKEEPINGHANDLER_H

#include <e32base.h>
#include "MHeartBeatObserver.h"
#include "CCPixAsyncronizer.h"

#include "cpix_async.h"

class CHeartBeatTimer;
class CCPixAsyncronizer;

/**
*  Handle automatically calling CPiX asynchronous house keeping function
*/
class CHouseKeepingHandler : public CBase, public MHeartBeatObserver, public MCPixAsyncronizerObserver
	{
public: // Constructors and destructors     
 
     /**
      * Two-phased constructor.
      * @return Pointer to created CHouseKeepingHandler object.
      */
     static CHouseKeepingHandler* NewL();
 
     /**
      * Destructor.
      */
     virtual ~CHouseKeepingHandler();

public: // New methods
    
    /**
     * Start house keeping handling
     */
    void StartL();

    /*
     * Callback from CCPixAsyncronizer, called when asynchronous operations complete
     */
    static void CompletionCallback(void *aCookie, cpix_JobId aJobId);
    
    /**
     * Stop housekeeping
     */
    void StopHouseKeeping();

    /**
     * Continue housekeeping
     */
    void ContinueHouseKeeping();

    /**
     * Force housekeeping
     */
    void ForceHouseKeeping();
    
public: // From MHeartBeatObserver
    
    void HandleHeartBeatL();
    
public: // From MCPixAsyncronizerObserver
    
    void HandleAsyncronizerComplete(TCPixTaskType aType, TInt aError, const RMessage2& aMessage);

private:
    
    /**
     * Handle house keeping
     */
    void DoHouseKeeping();
    
private: // Constructors and destructors

    /**
     * C++ default constructor.
     * @param aServer The server.
     */
    CHouseKeepingHandler();

    /**
     * 2nd phase constructor.
     */
    void ConstructL();

private:
    
    // Timer to call the heart beat
    CHeartBeatTimer* iHeartBeatTimer;
    
    // Id of currently executing function in CPix
    cpix_JobId iPendingJobId;
    
    // Asynchronizer object for search and getdocument 
    CCPixAsyncronizer* iAsyncronizer;

    // Is asynchronous request currently ongoing
    TBool iIsPending;
    
    // Dummy RMessage2
    RMessage2 iMessage;
    
    static TBool iCallBackValid;
	};

#endif // CHOUSEKEEPINGHANDLER_H
