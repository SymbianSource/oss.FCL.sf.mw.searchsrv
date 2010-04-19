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

#ifndef CHEARTBEATTIMER_H_
#define CHEARTBEATTIMER_H_

#include <e32base.h>

class MHeartBeatObserver;

class CHeartBeatTimer : public CTimer
	{
public:  // Constructors and destructors
    
    /**
     * Two-phased constructor.
     * @param aObserver Observer
     * @return Pointer to created CHeartBeatTimer object.
     */
    static CHeartBeatTimer* NewL(MHeartBeatObserver* aObserver);

    /**
     * Destructor.
     */
    virtual ~CHeartBeatTimer();

public: // New methods
    
    /**
     * Stop timer
     */
    void StopTimer();
    
    /**
     * Continue timer.
     * Timer starts from initial value
     */
    void ContinueTimer();
    
private:  // Constructors and destructors

    /**
     * C++ default constructor.
     * @param aObserver Observer
     */
    CHeartBeatTimer(MHeartBeatObserver* aObserver);

    /**
     * 2nd phase constructor.
     */
    void ConstructL();

private: // From baseclass
    
    void RunL();
	
private: // Data
    
    // Observer
	MHeartBeatObserver* iObserver;
	};

#endif /* CHEARTBEATTIMER_H_ */
