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

#ifndef MEMORYRECORDER_H_
#define MEMORYRECORDER_H_

#include <memory>
#include <string>
#include <pthread.h>

class MemoryRecord {

public:
		
	MemoryRecord();
	
	int peak_; 
	
	int minimum_; 
	
	double average_; 
	
	int sampleCount_; 
	
public: 

	/**
	 * Returns ownership 
	 */
	std::auto_ptr<std::string> toString() const;

	std::auto_ptr<std::wstring> toWstring() const;

};

/**
 *  Currently a heap recorder to be exact
 */
class MemoryRecorder {

protected: 

	static void* runInstance(void* thiz);

public:
	
	MemoryRecorder(); 
	
	~MemoryRecorder(); 
		
	bool start(int intervalMs);
	
	/**
	 * Returns ownership
	 */
	std::auto_ptr<MemoryRecord> finish(); 

protected: 
	
	void run(); 
	
	void record();
	
private: 
	
	pthread_t thread_;
	
	bool finish_; 
	
	int intervalMs_; // How often it takes samples
	
	MemoryRecord* record_;

};

#endif /*MEMORYRECORDER_H_*/
