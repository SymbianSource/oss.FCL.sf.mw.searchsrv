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


#ifndef CROBUSTNESSTEST_H_
#define CROBUSTNESSTEST_H_

#define PROVIDE_TESTING_UTILITY

#include <e32base.h>
#include "CProcessMonitor.h"
#include "CLog.h"

class CStressWorker; 
class CRobustnessTest; 
class CTicker;

class MRobustnessTestObserver 
	{
public: 
	virtual void FinishedL( CRobustnessTest& aTest ) = 0; 
	};

/**
 * Robustness test is designed to set heavy stress on cpix 
 * services over long period of time and record the results. 
 * 
 * To stress the search server, two indexers are provided for
 * usage, where the other is the 'normal' index and the other
 * is the 'busy' index. There is one thread doing indexing
 * and another performing searching on the normal index. 
 * There are three threads for both indexing and searching 
 * for the busy index. As an addition to these, there is one
 * multisearcher, which searchers over both normal and busy 
 * indeces. 
 * 
 * The time period of robustness test is divided into 'ticks',
 * where during each tick one entry to the test record is added. 
 * The record will contain information around average and peak 
 * memory usages in the server, about the index sizes and 
 * operation counts and times. If the search server closes or 
 * panics the test is terminated and the server exit/panic code 
 * is added to the test report.
 */
class CRobustnessTest : public CBase, 
						public MProcessMonitorObserver
	{
public: 
	
	CRobustnessTest( CConsoleBase& iConsole, MRobustnessTestObserver& aObserver ); 

	void ConstructL(); 

	void StartL( ); 

	~CRobustnessTest(); 
	
protected: 
	
	TInt ReadNumberL( TPtrC8& left );
	
	void ConfigureL(); 
	
public: // From process monitor observer
	
	virtual void ProcessFinished( CProcessMonitor& aMonitor, 
							      TExitType aExitType ,
							      TExitCategoryName aCategory,
							      TInt aExitReason ); 

	virtual void Failed( CProcessMonitor& aMonitor,
					     TInt aError ); 
	
public: // For Periodic
	
	/**
	 * Updates robustness test record and requests the server
	 * to update its record. If this is the last tick; it will finalize
	 * the records and terminate the test. 
	 */
	virtual void Tick();
	
	/**
	 * Used as a parameter for CPeriodic. Calls CPeriodic::Tick
	 */
	static TInt CallTick(TAny* ptr); 

protected: 

	/**
	 * Combines the temporary robustness record and the search 
	 * server record to form the final robustness test record.
	 */
	void FinalizeRecordsL(); 

	/**
	 * Formats a record row so that each member of the given 
	 * aStats array is formatted nicely on its own column.
	 */
	void AppendRecordL( RPointerArray<HBufC8>& aStats ); 

	/**
	 * Requests the record labels from the stress worker object,
	 * and forms a row having the labels in nicely formatted form. 
	 */
	void AddStatsLabelsL(); 
	
	/**
	 * Requests the record values from the stress worker object,
	 * and forms a row having the labels in nicely formatted form. 
	 */
	void AddStatsEntryL(); 
	
	/**
	 * Terminates the test and notifies the test observer that the 
	 * test has finished. At the moment the observer is called, this
	 * object has no more active components alive (e.g. threads, active
	 * objects) and it can be freely destroyed. 
	 */
	void FinishL(); 

private: // Infra

	CConsoleBase& iConsole;

	RFs iFs; 
	
	CLog* iOwnedLog; 

	TShortTimeStampLog iLog;

private: // Testing utilities

	MRobustnessTestObserver& iObserver;
	
	CPeriodic* iPeriodic; 
	
	CProcessMonitor* iSearchServerMonitor; 

	CStressWorker* iStressWorker; 
	
private: // State
	
	TInt iTick;
	
private: // Configuration

	TInt iTickLengthSeconds; 

	TInt iTickCount;

	TInt iPreIndex; 

	TInt iIndexAverageItems; 

	TBool iIndexingEnabled; 

	TBool iDeletesEnabled; 

	TBool iSearchersEnabled;
	
	TBool iCancellingEnabled;

	TInt iIndexerSleep; 

	TInt iSearcherSleep; 

	};

#endif /* CROBUSTNESSTEST_H_ */
