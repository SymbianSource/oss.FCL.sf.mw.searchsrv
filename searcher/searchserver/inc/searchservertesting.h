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

#ifndef SEARCHSERVERTESTING_H_
#define SEARCHSERVERTESTING_H_

#include "SearchServerConfiguration.h"
#include <indevicecfg.h>
#include <e32base.h>
#include <e32cmn.h>
#include <f32file.h>

#ifdef PROVIDE_TESTING_UTILITY

class TPerformanceRecord
{
public: 
	
	TPerformanceRecord();

	void ToStringL( TDes& aString, const TDesC& aUnit, TInt aScale );
	
	void Record( TInt aValue );

	void Record( TPerformanceRecord& aRecord );

	TInt iMinimum;
	TReal iAverage;
	/** Estimation for variance. Not statistically correct variance. */
	TReal iVarEst;
	TInt iPeak;
	TInt iSampleCount;
};


class MemoryRecorder 
	{
public: // Constructors & Destructors

	static MemoryRecorder* NewL(); 

	~MemoryRecorder();

private:

	MemoryRecorder();

	void ConstructL();
	
public: // API 

	void StartL( TInt aRecordingIntervalMs );

	TBool IsActive(); 
	
	TPerformanceRecord* Finish();
	
private: // Internal

	void Record();
	
	void Work();
	
	static TInt ThreadFunction(void *aThis);
	
private: // state 

	RSemaphore iSemaphore; 
	
	RThread iWorkerThread; 
	
	TBool iAbandon; 
	
	TPerformanceRecord* iRecord; 
	
	TInt iIntervalMs;
	
private: // InstanceCount
	
	static TInt iInstances; 
	}; 



#ifdef LOG_PERIODIC_MEMORY_USAGE

const TInt KMemoryLoggingIntervalMs = 3 * 1000; 

class CMemoryLogger : public CBase 
	{
public: // Constructors & destructors
	
	CMemoryLogger();

	~CMemoryLogger();
	
public: // API 
	
	void StartL();

	void Stop();
	
private: // Internal 
	
	void RunL();

	static TInt RunThisL(void *aThis);

private: // State 
	
	CPeriodic* iPeriodic;
	
	RMemoryRecorder* iRecorder;
	
	};

#endif // LOG_PERIODIC_MEMORY_USAGE

#endif // RECORD_MEMORY_USAGE


#ifdef LOG_EXECUTION_MEMORY_USAGE

#define RECORDED_EXECUTION_MEMORY_USAGE_BEGIN 			  	   	\
	{ 													  		\
	RMemoryRecorder* memoryRecorder_ = RMemoryRecorder::NewL(); \
	CleanupClosePushL( *memoryRecorder_ ); 					  	\
	memoryRecorder_->StartL( 1 ); 

#define RECORDED_EXECUTION_MEMORY_USAGE_END(x)   	  \
	TPerformanceRecord* r = memoryRecorder_->Finish(); 	  \
	CleanupStack::PopAndDestroy( memoryRecorder_ );  \
	CSearchServerTesting::Instance()->FunctionEntry( _L( x ) ).iMemory.Record( *r ); \
	delete r; 										 \
	}												  

#else // LOG_EXECUTION_MEMORY_USAGE

#define RECORDED_EXECUTION_MEMORY_USAGE_BEGIN
#define RECORDED_EXECUTION_MEMORY_USAGE_END(x) 

#endif // LOG_EXECUTION_MEMORY_USAGE

#ifdef LOG_EXECUTION_TIMES

#define RECORDED_EXECUTION_TIME_BEGIN				  \
	{											  \
	int beginMs_ = User::NTickCount(); 				  

#define RECORDED_EXECUTION_TIME_END(x)				  \
	int endMs_ = User::NTickCount(); 				  \
	CSearchServerTesting::Instance()->FunctionEntry( _L( x ) ).iTime.Record( endMs_ - beginMs_ ); \
	}

#else // LOG_EXECUTION_PERFORMANCE_TIME

#define RECORDED_EXECUTION_TIME_BEGIN
#define RECORDED_EXECUTION_TIME_END(x)

#endif // LOG_EXECUTION_PERFORMANCE_TIME

//
// NOTE: Watch, when you use these macros !
// They are most approriate in the beginning of block and in the end of one 
// 

#define RECORDED_EXECUTION_BEGIN RECORDED_EXECUTION_MEMORY_USAGE_BEGIN \
 								 RECORDED_EXECUTION_TIME_BEGIN 

#define RECORDED_EXECUTION_END(x) RECORDED_EXECUTION_TIME_END(x) \
								  RECORDED_EXECUTION_MEMORY_USAGE_END(x)


#ifdef PROVIDE_TESTING_UTILITY

class TFunctionPerformanceEntry 
	{
public: 
	TFunctionPerformanceEntry( const TDesC& aId ); 
public:
	
	TBuf<32> iId; 
	TPerformanceRecord iMemory; 
	TPerformanceRecord iTime; 
	};

_LIT( KShutdownSignalFile, 			CPIX_INDEVICE_SERVERIO L"\\shutdown.txt" );
_LIT( KStartRecordingSignalFile, 	CPIX_INDEVICE_SERVERIO L"\\start_recording.txt" );
_LIT( KStopRecordingSignalFile, 	CPIX_INDEVICE_SERVERIO L"\\stop_recording.txt" );
_LIT( KDumpRecordSignalFile, 		CPIX_INDEVICE_SERVERIO L"\\dump_record.txt" );
_LIT( KServerRecordFile, 			CPIX_INDEVICE_SERVERIO L"\\record.csv" );

const TInt TSignalCheckIntervalMs = 500; 
const TInt KMemoryRecordingIntervalMs = 50; 

class CSearchServer; 

class CSearchServerTesting : public CBase
	{
public: 

	static CSearchServerTesting* Instance(); 
	
	/**
	 * Registers oneself automatically as an instance
	 */
	CSearchServerTesting(); 
	
	void ConstructL(); 

	/**
	 * Cleans the instance reference
	 */
	~CSearchServerTesting(); 
	
	void SetServer( CSearchServer* aServer );
	
public:

	void Reset(); 
	
	TFunctionPerformanceEntry & FunctionEntry( const TDesC& aEntry );
	
public: 
	
	void StartL();

	void Stop();
	
protected: // Internal 
	
	void RunL();
	
	static TInt RunThisL(void *aThis);

private: // Functionality
	
	void ShutdownL(); 
	
	void StartRecordingL(); 

	void StopRecording(); 
	
	void DumpRecordL(); 

private: 
	
	MemoryRecorder* iRecorder;

	CSearchServer* iServer;
	
	CPeriodic* iPeriodic;
	
	RArray<TFunctionPerformanceEntry > iEntries; 
	
	RFs iFs;
	
	static CSearchServerTesting* iInstance; 
	
	};

#endif // PROVIDE_TESTING_UTILITY

#endif /*SEARCHSERVERTESTING_H_*/
