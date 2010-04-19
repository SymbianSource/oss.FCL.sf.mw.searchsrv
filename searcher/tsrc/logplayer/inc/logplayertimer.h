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


#ifndef LOGPLAYERTIMER_H
#define LOGPLAYERTIMER_H

//  Include Files
#include <e32base.h>

class MLogPlayerTimeCallback
{
public:
	
	virtual void TimerCallback() = 0;
};

class CLogPlayerTimer : public CTimer
{
public : // Constructors and destructors

    /**
    * NewL.
    * Two-phased constructor.
    * @param aCallback Pointer to callback
    * @return Pointer to created CLogPlayerTimer object.
    */
    static CLogPlayerTimer* NewL(MLogPlayerTimeCallback* aCallback);

    /**
    * NewLC.
    * Two-phased constructor.
    * @param aCallback Pointer to callback
    * @return Pointer to created CLogPlayerTimer object.
    */
    static CLogPlayerTimer* NewLC(MLogPlayerTimeCallback* aCallback);

    /**
    * Destructor.
    */
    virtual ~CLogPlayerTimer();

private: // From CTimer
	
	void RunL();

public: // New functions
    
	/**
	 * Start timer.
	 * @param aTime Callback is called at given time
	 */
	void StartTimer( const TTime& aTime );
    
private: // Constructors and destructors

    /**
    * C++ default constructor.
    * @param aCallback Pointer to callback
    */
	CLogPlayerTimer(MLogPlayerTimeCallback* aCallback);

    /**
    * ConstructL.
    * 2nd phase constructor.
    */
    void ConstructL();
    
private: // Data
    
	MLogPlayerTimeCallback* iTimer;
}
;
#endif  // LOGPLAYERTIMER_H
