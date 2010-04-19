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


#ifndef CWDTIMER_H
#define CWDTIMER_H

#include <e32base.h>

class MWDTimerHandler;

class CWDTimer : public CActive
    {
public:
    /*
     * Construction
     * @return instance of CWDTimer
     */
    static CWDTimer* NewL( MWDTimerHandler* aWDTimerHandler ); //For callback
    /*
     * Construction
     * @return instance of CWDTimer
     */
    static CWDTimer* NewLC( MWDTimerHandler* aWDTimerHandler );
    /*
     * Destructor
     */
    virtual ~CWDTimer();
public:
    /*
     * Srats the watch dog timer to monitor the CPix servers
     */
    void StartWDTimer();
    
    //From CActive
    void RunL();
    void DoCancel();
    TInt RunError( TInt aError );
private:    
    /*
     * Constructor
     */
    CWDTimer();
    /*
     * Second phase constructor
     */
    void ConstructL( MWDTimerHandler* aWDTimerHandler );

private:
    
    // Timer which is used to delay indexing messages
    RTimer iTimer; //Timer for self activation of AO
    //Watch dog timer handler.Not owned
    MWDTimerHandler* iWDTimerHandler;
    };

#endif /* CWDTIMER_H */
