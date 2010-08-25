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

#ifndef CENTREPMONITOR_H_
#define CENTREPMONITOR_H_

#include <e32base.h>
#include "mcentrephandler.h"

class CRepository;

class CentrepMonitor : public CActive
    {
public:
    /*
     * Construction
     * @return instance of CentrepMonitor
     */
    static CentrepMonitor* NewL( MCentrepHandler* aCentrepHandler, TUint32 aKey ); //For callback
    /*
     * Construction
     * @return instance of CentrepMonitor
     */
    static CentrepMonitor* NewLC( MCentrepHandler* aCentrepHandler, TUint32 aKey );
    /*
     * Destructor
     */
    virtual ~CentrepMonitor();
public:
    /*
     * Srats the notifier to monitor the changes done for HS and SS centrep key
     */
    void StartNotifier();
    
    //From CActive
    void RunL();
    void DoCancel();
    TInt RunError( TInt aError );
private:    
    /*
     * Constructor
     */
    CentrepMonitor();
    
    /*
     * Second phase constructor
     */
    void ConstructL( MCentrepHandler* aCentrepHandler, TUint32 aKey );    

private:

    TUint32 iKey;
    //centrep handler.Not owned
    MCentrepHandler* iCentrepHandler;
    //repository db for watchdog.owned
    CRepository* aWDrepo;
    };


#endif //CENTREPMONITOR_H_
