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

#ifndef CCPIXASYNCRONIZER_H
#define CCPIXASYNCRONIZER_H

enum TCPixTaskType
	{
	ECPixTaskTypeSearch = 0,
	ECPixTaskTypeGetDocument,
	ECPixTaskTypeAdd,
	ECPixTaskTypeUpdate,	
	ECPixTaskTypeDelete,
	ECPixTaskTypeFlush,
	ECPixTaskTypeHouseKeeping	
	};

// CLASS DECLARATION

/**
*  Observer for CCPixAsyncronizer
*/
class MCPixAsyncronizerObserver
	{
public:
	virtual void HandleAsyncronizerComplete(TCPixTaskType aType, TInt aError, const RMessage2& aMessage) = 0;
	};

/**
 * Asynchronizer responsible for communicating with CPix asynchronous functions
 */
class CCPixAsyncronizer : public CActive
	{
public: // Constructors and destructors
    
    /**
    * NewL.
    * Two-phased constructor.
    * @return Pointer to created CHarvesterServer object.
    */
    static CCPixAsyncronizer* NewL();

    /**
    * NewLC.
    * Two-phased constructor.
    * @return Pointer to created CHarvesterServer object.
    */
    static CCPixAsyncronizer* NewLC();

    /**
    * Destructor.
    */
    virtual ~CCPixAsyncronizer();

public: // New methods

    /**
     * Start waiting asynchronous command completion
     * @param aType Type of CPix task
     * @param aObserver Pointer to callback observer
     * @param aMessage client-server message responsible for this request
     */
	void Start(TCPixTaskType aType, MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage);
	
	/**
	 * Cancel asynchronous commands that CPix does not provice cancellation function
     * Only way to cancel these messages is to wait until commands are completed.
     * @param aMessage RMessage2 of client cancellation request. Must be completed
     *                 when cancellation is done.
    */
	void CancelWhenDone(const RMessage2& aMessage);
	
	/**
	 * Callback for CPix requests.
	 * CompletionCallback CAN BE and IS called from another thread, 
	 * so care should be taken what kind of actions are done in there  
    */
	void CompletionCallback();
	
protected: // From CActive
    
	void RunL();

	void DoCancel();
	
private:
    
    /**
     * Notify observer about
     * @param aErrorCode Error code of request
     */
    void NotifyObserver(TInt aErrorCode);

private: // Constructors and destructors

    /**
    * C++ default constructor.
    */
    CCPixAsyncronizer();

    /**
    * 2nd phase constructor.
    */
    void ConstructL();
    
private:
    
    // Type of currently executing task
	TCPixTaskType iType;
	
	// Callback observer
	MCPixAsyncronizerObserver* iObserver;
	
	// Thread where this object was created. Callback from CPiX comes from
	// different thread and needs reference to original thread
	RThread iMainThread;

	// Original client-server message for this request
	RMessage2 iMessage;

	/**
	 * Is CancellAll operation currently ongoing
	 */
	TBool iCancelAllOperationPending;
	
	/**
	 * RMessage2 related to cancellation request
	 */
	RMessage2 iCancelMessage;
	
	
	TBool iWaitingForCompletion;
	};

#endif // CCPIXASYNCRONIZER_H
