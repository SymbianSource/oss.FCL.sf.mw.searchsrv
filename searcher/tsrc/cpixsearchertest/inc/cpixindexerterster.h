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

#ifndef CPIXINDEXERTERSTER_H_
#define CPIXINDEXERTERSTER_H_

#include "RSearchServerSession.h"
#include "MCPixIndexerObserver.h"
#include "AOTestClass.h"

class CCPixIndexer;
class CCPixSearcher;

class CHandleIndexingResultLeaves : public CBase,
                                    public MCPixIndexingRequestObserver
{
public:
    CHandleIndexingResultLeaves();
    ~CHandleIndexingResultLeaves();
    void HandleIndexingResultL( TInt aError );
public:
    CActiveSchedulerWait* iWait;
};

class CTestIndexer : public CBase,
                     public MCPixOpenDatabaseRequestObserver,
                     public MCPixIndexingRequestObserver,
                     public MAOTestObserver,
                     public MCPixSetAnalyzerRequestObserver
{   
public:
    CTestIndexer(){}

private:   

    void CreateFileIndexItemL(const TDesC& aFilename);
    TInt SearchForTextL(const TDesC& aQueryString, const TDesC& aDefaultField);
    TBool CheckBaseAppClassIsExpectedL(const TDesC& aExpectedAppClass, 
                                       const TDesC& aDocUid);
    

public: // From MCPixOpenDatabaseRequestObserver
    void HandleOpenDatabaseResultL(TInt aError);
    //from MCPixSetAnalyzerRequestObserver
    void HandleSetAnalyzerResultL(TInt /*aError*/);

public: // From MCPixIndexingRequestObserver
    void HandleIndexingResultL(TInt aError);

public: // From MAOTestObserver
    void CallCompleted( int i );

public:
    void setUp();
    void tearDown();
        
    void testOpenIndexDb();
    void testAddL();
    void testAsyncAddL();
    void testCancelAddL();
    void testUpdateL();
    void testAsyncUpdateL();
    void testDeleteL();
    void testAsyncDeleteL();
    void testResetL();
    void testFlushL();
    void testAsyncFlushL();
    void testAsyncResetL();
    void testHandleIndexingResultL_Leaves();
    void testSetAnalyzerAsync();
    void testSearchSession();

private:
    CCPixIndexer* iIndexer;// CPix database 
    CCPixSearcher* iSearcher;
    RSearchServerSession iSession;
    TInt iCurrentIndex;
    CHandleIndexingResultLeaves* iHandleResultLeaves;
    CActiveSchedulerWait* iWait;
    CAOTestClass* iMyAOClass;
};

#endif /* CPIXINDEXERTERSTER_H_ */
