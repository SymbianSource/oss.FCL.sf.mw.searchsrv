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

#ifndef MULTITHREADTESTER_H_
#define MULTITHREADTESTER_H_

#include <e32base.h>

class CWorker; 

/**
 * This test mainly tests, whether the server crashes or not under concurrent
 * use from a moderate amount of threads. The test has 4 worker threads 
 * performing concurrent indexing and 5 searcher threads performing concurrent
 * searching. The test fails, if any operations fail (still, whether items
 * indexed are really in index, or if the search results are valid are not 
 * tested)
 */
class CTestMultiThreading : public CBase
{   
public:
    CTestMultiThreading(){}
private:
    void setUp();
    void tearDown();
public:
    void PrepareL();
    void Wait(TInt aSeconds);
    void RunL(); 
    void Finish();    
    void DoTestMultiThreadingL();

private:
    // CPix database 
    
    RPointerArray<CWorker> *iWorkers; 
};

#endif /* MULTITHREADTESTER_H_ */
