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


#ifndef CWDMONITOR_H
#define CWDMONITOR_H

#include <e32base.h>
#include "MWDTimerHandler.h"
#include "mcentrephandler.h"

//Forward declaration
class CWDTimer;
class CentrepMonitor;

class CWDMonitor : public CBase , public MWDTimerHandler, public MCentrepHandler
    {
public:
    /*
     * Construction
     * @return instance of CWDMonitor
     */
    static CWDMonitor* NewL();
    /*
     * Construction
     * @return instance of CWDMonitor
     */
    static CWDMonitor* NewLC();
    /*
     * Destructor
     */
    virtual ~CWDMonitor();
public:
    
    /*
     * @description Starts the timer for monitoring the servers.
     */
    void StartMonitor();
    
    // From MWDTimerHandler
    void HandleWDTimerL();
    
    //From MCentrepHandler
    void HandlecentrepL( TUint32 aKey );
    
private:
    /*
     * @description Starts the specified server.
     * @param TDesC& aServerName Name of the server to start
     * @param TUid aServerUid uid of the server to start
     * @param TDesC& aSemaphoreName Name of the semaphore
     * @return KErrNone on succesful operation.
     */
    TInt StartServer( const TDesC& aServerName , TUid aServerUid ,
                      const TDesC& aSemaphoreName);
    
    /*
     * @description creates the specified server process.
     * @param TDesC& aServerName Name of the server to start
     * @param TUid aServerUid uid of the server to start
     * @return KErrNone on succesful operation.
     */
    TInt CreateServerProcess( const TDesC& aServerName , TUid aServerUid  );
    /*
     * Constructor
     */
    CWDMonitor();
    /*
     * Second phase constructor
     */
    void ConstructL();

private:
    
    // Timer which is used to delay server monitoring
    CWDTimer* iWDTimer;
    TBool iAllowHS;
    TBool iAllowSS;
    TUid iHSUid;
    TUid iSSUid;
    HBufC* iHSName;
    HBufC* iSSName;
    
    CentrepMonitor* aHSMonitor;
    CentrepMonitor* aSSMonitor;
    };

#endif /* CWDMONITOR_H */
