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
* Description:  This component is to start gaurd timer
*
*/


#ifndef CGAURDTIMER_H
#define CGAURDTIMER_H

#include <e32base.h>

class MGaurdTimerHandler
    {
public :
    /*
    * HandleGaurdTimerL signal when the gaurd timer expires 
    */
    virtual void HandleGaurdTimerL() = 0;
    };

class CGaurdTimer : public CActive
    {
public:
    /*
     * Construction
     * @return instance of CGaurdTimer
     */
    static CGaurdTimer* NewL( MGaurdTimerHandler* aGaurdTimerHandler ); //For callback
    /*
     * Construction
     * @return instance of CGaurdTimer
     */
    static CGaurdTimer* NewLC( MGaurdTimerHandler* aGaurdTimerHandler );
    /*
     * Destructor
     */
    virtual ~CGaurdTimer();
public:
    /*
     * Srats the gaurd timer to find the idle time
     */
    void StartgaurdTimer();
    
    //From CActive
    void RunL();
    void DoCancel();
    TInt RunError( TInt aError );
private:    
    /*
     * Constructor
     */
    CGaurdTimer();
    /*
     * Second phase constructor
     */
    void ConstructL( MGaurdTimerHandler* aGaurdTimerHandler );

private:
    
    //Timer for self activation of AO
    RTimer iTimer; 
   
    MGaurdTimerHandler* iGaurdTimerHandler;
    };

#endif /* CGAURDTIMER_H */
