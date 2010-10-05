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
class CServerMonitor;

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
    /* Monitor to watch the centrep values of Harvester server status.owned*/
    CentrepMonitor* aHSStatusMonitor;
    /* Monitor to watch the centrep values of Search server status.owned*/
    CentrepMonitor* aSSStatusMonitor;
    /* Monitor to watch the centrep values of Harvester server Uid.owned*/
    CentrepMonitor* aHSUidMonitor;
    /* Monitor to watch the centrep values of Search server Uid.owned*/
    CentrepMonitor* aSSUidMonitor;
    /* Monitor to watch the centrep values of Harvester server Name.owned*/
    CentrepMonitor* aHSNameMonitor;
    /* Monitor to watch the centrep values of Search server Name.owned*/
    CentrepMonitor* aSSNameMonitor;
    /* Monitor to watch the existance of the Harvester server.owned*/
    CServerMonitor* iHarvesterMonitor;
    /* Monitor to watch the existance of the Search server.owned*/
    CServerMonitor* iSearchMonitor;
    };

#endif /* CWDMONITOR_H */
