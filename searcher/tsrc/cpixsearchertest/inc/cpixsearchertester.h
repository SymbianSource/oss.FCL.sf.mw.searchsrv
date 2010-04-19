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

#ifndef CPIXSEARCHERTESTER_H_
#define CPIXSEARCHERTESTER_H_

//#include "TestSuite.h"
#include "RSearchServerSession.h"
#include "MCPixSearcherObserver.h"
#include <f32file.h>
#include "AOTestClass.h"

class CCPixSearcher;
class CCPixIndexer;

class CTestSearcher : //public CxxTest::TestSuite,
                      public MCPixOpenDatabaseRequestObserver,
                      public MCPixSearchRequestObserver,
                      public MCPixNextDocumentRequestObserver,
                      public MAOTestObserver,
                      public MCPixSetAnalyzerRequestObserver
{   
public:
    CTestSearcher(){}
    void setUp();
    void tearDown();
private:    

    TInt SearchForTextL(const TDesC& aText, const TDesC& aDefaultField);
    void AddFileToIndexL(const TDesC& aFileName);

public: // From MCPixOpenDatabaseRequestObserver
    void HandleSearchResultsL(TInt aError, TInt aEstimatedResultCount);
    //From MCPixSetAnalyzerRequestObserver
    void HandleSetAnalyzerResultL(TInt /*aError*/);

public: // From MCPixOpenDatabaseRequestObserver
    void HandleOpenDatabaseResultL( TInt aError );

public: // From MCPixNextDocumentRequestObserver
    void HandleDocumentL(TInt aError, CSearchDocument* aDocument);
    
public: // From MAOTestObserver
    void CallCompleted( int i );

public:
    /*
     * Basic search tests - requires IndexDb with Shakespeare corpus data 
     * already indexed.
     */
    TInt testKnownTermTestsL();
    /*
     * Simple tests for wildcard searches
     */
    void testWildcardTermTests();
    /*
     * Makes sure that RSearchServerSession::DefineVolume() works with non-empty
     * path parameter.
     */
    void testDefineVolumeWorksNonEmptyPath();
    /*
     * Makes sure that RSearchServerSession::DefineVolume() works with empty 
     * path parameter.
     */
    void testDefineVolumeWorksWithEmptyPath();

    /**
     * Volume error scenarios.
     */
    void testDefineVolumeErrorScenarios();

    /**
     * Test search cancellation
     */
    void testSearchCancellation();

    /**
     * Test asynchronous search works as expected
     */
    void testSearchAsynchronous();

    /**
     * Tests that a second search will leave if previous search
     * is not cancelled.
     */
    void testSearchLeavesIfNotCancelled();
    void testOpenIndexDb();
    void testDeleteIndexDbWhileSearching();
    void testGetDocumentAsync();
    /**
     *  Test that if an invalid document is requested that the HandleDocumentL
     *  observer function will be called. This is important for completing the
     *  search statemachine.
     */
    void testGetInvalidDocumentAsync();
private:
    CCPixSearcher* iSearcher;
    RSearchServerSession iSession;
    RFs iFs;
    RFile iFile;
    CCPixIndexer* iIndexer;
    CActiveSchedulerWait* iWait;
    TInt iDocumentCount;
    CAOTestClass* iMyAOClass;
    CSearchDocument* iDocument;
    TBool iHandleDocumentLFunctionCalled; // ETrue if HandleDocumentL called
    TBool iHandleSetAnalyzerCalled; // ETrue if HandleSetAnalyzerResultL called
    };

#endif /* CPIXSEARCHERTESTER_H_ */
