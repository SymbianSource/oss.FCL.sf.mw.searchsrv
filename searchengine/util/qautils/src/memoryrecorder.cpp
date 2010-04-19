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
#include "Memoryrecorder.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <exception>
#include <sstream>

using namespace std;

MemoryRecord::MemoryRecord() 
: peak_(0),
  minimum_(0x7fffffff), // FIXME: replace with max int
  average_(0),
  sampleCount_(0)
	{
	}


// FIXME: Redundancy between toString and toWstring 

auto_ptr<string> MemoryRecord::toString() const
	{
	ostringstream in;
	in<<"heap: "<<(minimum_/1000)<<"KB < ~"<<((int)average_/1000)<<"KB < "<<(peak_ / 1000)<<" KB (n="<<sampleCount_<<")";
	return auto_ptr<string>(new string(in.str()));
	}

auto_ptr<wstring> MemoryRecord::toWstring() const
	{
	wostringstream in;
	in<<"heap: "<<(minimum_/1000)<<"KB < ~"<<((int)average_/1000)<<"KB < "<<(peak_ / 1000)<<" KB (n="<<sampleCount_<<")";
	return auto_ptr<wstring>(new wstring(in.str()));
	}

MemoryRecorder::MemoryRecorder()
: 	finish_(false),
	record_(0)
	{
	}
	
MemoryRecorder::~MemoryRecorder()
	{
	if ( record_ ) 
		{
		if ( !finish_ ) 
			{
			finish(); 
			}
		else 
			{
			printf("PANIC: Memory recorder killed, while someone waits worker thread to finish!");
			getchar(); 
			exit(0); 			
			}
		}
	}

class already_running : public exception { 
	virtual const char* what() const { return "already running"; } 
}; 

class worker_creation_failed : public exception { 
	virtual const char* what() const { return "Failed to create worker threads"; } 
}; 

bool MemoryRecorder::start(int intervalMs)
	{
	if (record_) 
		{
		return false;
		}

	intervalMs_ = intervalMs; 
	finish_ = false;
	record_ = new MemoryRecord(); 

	if ( pthread_create( &thread_, NULL, MemoryRecorder::runInstance, this ) != 0 ) 
		{
		return false; 
		}
	
	return true;
	}
	
void MemoryRecorder::run()
	{
	while (!finish_) 
		{
		record();
		
		struct timespec tim;
	    tim.tv_sec = 0;
	    tim.tv_nsec = intervalMs_*1000000;
	    nanosleep(&tim , NULL);
		}
	}

void* MemoryRecorder::runInstance(void* thiz) {
	reinterpret_cast<MemoryRecorder*>(thiz)->run();
	return NULL;
}

class not_running : public exception { 
	virtual const char* what() const { return "Cannot finish() recorder, which is not running."; } 
}; 

class failed_joining_worker : public exception { 
	virtual const char* what() const { return "Failed to join worker thread."; } 
}; 

auto_ptr<MemoryRecord> MemoryRecorder::finish() 
	{
	if ( !record_ ) 
		{
		throw not_running(); 
		}
	
	finish_ = true; 

	if ( pthread_join( thread_, NULL ) != 0 ) 
		{
		throw failed_joining_worker(); 
		}
	record(); // Create the last record 
	MemoryRecord* ret = record_;
	record_ = NULL;
	return auto_ptr<MemoryRecord>(ret);
	}

#ifdef __SYMBIAN32__

#include <e32std.h>

void MemoryRecorder::record()
	{
	// Assumption: We are allowed to request from other thread
	// This threads heapSize
	int heapSize = User::Heap().Size();

	record_->average_ = ( record_->average_ )*( (double)record_->sampleCount_ / (double)( record_->sampleCount_ + 1. ) )
					   + ( ( double ) heapSize ) / ( record_->sampleCount_ + 1. );
	record_->peak_ = Max( record_->peak_, heapSize);
	record_->minimum_ = Min(record_->minimum_, heapSize);
	record_->sampleCount_++; 
	}

#else 

   // TODO: use malloc_t in standard UNIX environments

#endif
