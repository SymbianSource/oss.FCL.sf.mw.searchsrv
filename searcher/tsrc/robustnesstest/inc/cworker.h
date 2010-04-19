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

#ifndef CWORKER_H_
#define CWORKER_H_

#include <e32base.h>


#define KTermDeviation 100 // The bigger this word, the more different terms
#define KPrefixDeviation 20 // Pick prefixes more moderately
#define KTermCountDeviation 10 // The bigger this word, the more terms per document
_LIT( KContentsField, "Contents" ); 
_LIT( KNormalIndexDirectory, "c:\\data\\indexing\\indexdb\\root\\test\\normal" ); 
_LIT( KBusyIndexDirectory, "c:\\data\\indexing\\indexdb\\root\\test\\busy"); 

_LIT( KTestBaseAppClass, "root test" ); 
_LIT( KNormalBaseAppClass, "root test normal"); 
_LIT( KBusyBaseAppClass, "root test busy");

_LIT( KNormalVolume, "@df:root test normal"); 
_LIT( KBusyVolume, "@df:root test busy"); 

/**
 * Returns random numbers so, that small numbers
 * are most common. Used to approximate the distribution
 * of real world worlds. In the distribution the probability that n is 
 * one of the numbers following i is 9 times as probable as that n is i. 
 */
TInt LogRand( TInt aDeviation ); 

void SleepRandom( TInt aAverageMs ); 

/**
 * Base class for workers. Workers do some kind of activity 
 * (~indexing or searching) in their own slave thread.
 */
class CWorker : public CBase 
	{
public: 
	
	CWorker();

	void ConstructL();
	
	~CWorker();
	
public: 
	
	TInt WorkerId();
	
public: // Error handling
	
	TInt Error();
	
	const TTime& ExitTime();
	
	TBool IsReported();
	
	void SetReported();
	
public: // Thread life cycle handling
	
	/**
	 * Returns true, if TryFinish() has been called successfully
	 */
	TBool IsActive();

	/**
	 * Starts
	 */
	void StartL(); 
	
	/**
	 * Signals the thread to shut down. The thread will shut down
	 * after finishing its current task.  
	 */
	void Cancel();
	
	/**
	 * Attempts to 'join' the thread. Returns error code, if
	 * joining failed. NOTE: unlike posix 'join', this method
	 * does not return the thread 'exit-value'. Instead use
	 * Error to see, if an error occurred during thread
	 * execution.
	 * 
	 * NOTE: Always call cancel before attempting to join 
	 * the thread.   
	 */
	TInt TryJoin( TInt aMicroSeconds );
	
	/**
	 * Terminates the thread forcefully
	 */
	void Terminate();

public: // Lock usage (For accessing the worker private data) 
	
	void Lock();

	TInt TryLock( TInt aMicroSeconds );

	void Unlock();

public:  

	virtual const TDesC& Name()= 0; 

protected: // For internal usage 
	
	void Wait( TReal aSeconds );
	
protected: 

	/**
	 * For doing any required preparations in the original thread. 
	 */
	virtual void DoPrepareL() = 0;
	virtual void DoCancel() = 0;
	virtual void DoRunL() = 0;

	void RunL();
	
public: 
	
	static TInt ThreadFunction(void *aThis);
	
private: 
	
	TInt iWorkerId; 	
	TInt iError;
	TTime iExitTime; 
	TBool iReported; 
	static TInt iNextFreeWorkerId;  
	TBool iActive; 
	RThread iThread; 
	
	/**
	 * This lock guards the worker thread life cycle. Before
	 * exiting, the worker thread will signal on this semaphore. 
	 * 
	 * NOTE: Logon could be used instead.  
	 */
	RSemaphore iSemaphore;
	
	/**
	 * This lock guards the internal state variables of this 
	 * worker. NOTE: The internal state variables are defined
	 * in the inherited classes. For example the statistics are
	 * guarded by this semaphore.  
	 */
	RSemaphore iLock; 
	};


#endif /* CWORKER_H_ */
