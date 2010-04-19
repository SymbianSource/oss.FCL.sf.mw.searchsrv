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

#ifndef DELAYEDCALLBACK_H
#define DELAYEDCALLBACK_H

// INCLUDES
#include <e32base.h>

/**
 * @file 
 * @ingroup PluginAPI
 * @brief Contains CDelayedCallback and MDelayedCallbackObserver
 */


/**
 * @brief Observer for CDelayedCallback
 * @ingroup PluginAPI
 * 
 * An interface the user of CDelaydCallback must implement if it wishes 
 * to get notified about the delayed event. Used for turning long lasting
 * and heavy operations (like harvesting) asynchronous 
 */
class MDelayedCallbackObserver
    {
    public:
        /**
         * Called by the CDelayedCallback object when it was executed by
         * the active scheduler.
         * @oaram   aCode       The same code as you passed to the Start-
         *                      method of CDelayedCallback
         */
        virtual void DelayedCallbackL(TInt aCode) = 0;

        /**
         * Called by the CDelayedCallback object when there is leave in DelayedCallbackL
         */
        virtual void DelayedError(TInt aErrorCode) = 0;
    };
    
    
/**
 * @brief Used to call MDelayedCallbackObserver's callback method perioidically
 * @ingroup PluginAPI
 * 
 * A class that provides a callback mechanism as soon as active scheduler has
 * time to execute this. Used for turning long lasting and heavy operations 
 * (like harvesting) asynchronous. 
 * 
 * Link against: CPixSearchPluginInterface.dll
 */
class CDelayedCallback : private CActive
    {
    public:
        /**
         * Creates a delayed callback object.
         * @param   aPriority       The priority on which this object runs.
         * @return  Pointer to new CDelayedCallback object.
         */
    	IMPORT_C static CDelayedCallback* NewL(TInt aPriority);
        
        /**
         * Destructor.
         */
    	IMPORT_C virtual ~CDelayedCallback();
        
    private: // from CActive
        /**
         * From CActive, RunL
         */
        virtual void RunL();
        
        /**
         * From CActive, DoCancel
         */
        virtual void DoCancel();
        
        /**
         * From CActive, RunError
         */
        virtual TInt RunError(TInt aError);
        
    public:
        /**
         * Starts timing and after the given delay a callback through
         * MDelayedCallbackObserver interface will happen.
         * @param   aCode       An integer that will be passed to the observer
         *                      callback.
         * @param   aObserver   Pointer to observer. 
         * @param   aDelay      Delay in microseconds when the callback should
         *                      happen.
         */
    	IMPORT_C void Start(TInt aCode, MDelayedCallbackObserver* aObserver,
                            TInt aDelay=0);
                            
        /**
         * Cancels the callback.
         */
    	IMPORT_C void CancelCallback();
        
        /**
         * Checks if there is a callback pending.
         * @return  ETrue if delayed callback request is pending.
         */
    	IMPORT_C TBool CallbackPending() const;
 
    private:
        /**
         * Static callback method for CPeriodic
         * @param   aObj            Pointer to object that was given to CPeriodic
         */
        static TInt StaticTimerCallback(TAny* aAny);
        
        /**
         * The timer callback function that will manage the timer callback.
         */
        void TimerCallback();
        
        /**
         * Constructr.
         * @param   aPriority       Priority of this active object.
         */
        CDelayedCallback(TInt aPriority);
        
        /**
         * 2nd phase constructor.
         */
        void ConstructL();
        
    private:
        /** The integer code being passed to the observer. */
        TInt iCode;
        
        /** Pointer to observer. */
        MDelayedCallbackObserver* iObserver;
        
        /** Timer object. */
        CPeriodic* iTimer;
    };
    
#endif // DELAYEDCALLBACK_H
