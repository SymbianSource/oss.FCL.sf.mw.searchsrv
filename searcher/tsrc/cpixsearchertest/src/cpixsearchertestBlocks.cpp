/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description: This file contains testclass implementation.
*
*/

// [INCLUDE FILES] - do not remove
#include <e32svr.h>
#include <StifParser.h>
#include <StifTestInterface.h>
#include "cpixsearchertest.h"

#include <ccpixsearcher.h>
#include <CSearchDocument.h>
#include <CCPixIndexer.h>
#include <common.h>
#include "cpixsearchertester.h"
#include "multithreadtester.h"
#include "cpixboostertester.h"
#include "cpixanalyzertester.h"
#include "cpixindexerterster.h"
// EXTERNAL DATA STRUCTURES
//extern  ?external_data;

// EXTERNAL FUNCTION PROTOTYPES  
//extern ?external_function( ?arg_type,?arg_type );

// CONSTANTS
//const ?type ?constant_var = ?constant;

// MACROS
//#define ?macro ?macro_def

// LOCAL CONSTANTS AND MACROS
//const ?type ?constant_var = ?constant;
//#define ?macro_name ?macro_def
_LIT(KFileBaseAppClassC, "@c:test root file");
_LIT(KQueryString, "act");
// MODULE DATA STRUCTURES
//enum ?declaration
//typedef ?declaration

// LOCAL FUNCTION PROTOTYPES
//?type ?function_name( ?arg_type, ?arg_type );

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;

// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// ?function_name ?description.
// ?description
// Returns: ?value_1: ?description
//          ?value_n: ?description_line1
//                    ?description_line2
// -----------------------------------------------------------------------------
//
/*
?type ?function_name(
    ?arg_type arg,  // ?description
    ?arg_type arg)  // ?description
    {

    ?code  // ?comment

    // ?comment
    ?code
    }
*/
_LIT( KNoErrorString, "No Error" );
_LIT( KErrorString, " *** Error ***" );

TInt doSearch( const TDesC& aSearchString, const TDesC& aBaseAppClass )
    {
    RSearchServerSession session;
    User::LeaveIfError( session.Connect() );
    CCPixSearcher* searcher = CCPixSearcher::NewLC( session );
    searcher->OpenDatabaseL( aBaseAppClass );
    
    TInt docCount = searcher->SearchL( aSearchString, KNullDesC);
    
    CleanupStack::PopAndDestroy( searcher );
    session.Close();
    return docCount;
    }

void doLog( CStifLogger* logger, TInt error, const TDesC& errorString )
    {
    if( KErrNone == error ) logger->Log( KNoErrorString );
    else logger->Log( errorString );
    }
// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// Ccpixsearchertest::Delete
// Delete here all resources allocated and opened from test methods. 
// Called from destructor. 
// -----------------------------------------------------------------------------
//
void Ccpixsearchertest::Delete() 
    {

    }

// -----------------------------------------------------------------------------
// Ccpixsearchertest::RunMethodL
// Run specified method. Contains also table of test mothods and their names.
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::RunMethodL( 
    CStifItemParser& aItem ) 
    {

    static TStifFunctionInfo const KFunctions[] =
        {  
        // Copy this line for every implemented function.
        // First string is the function name used in TestScripter script file.
        // Second is the actual implementation member function. 
        ENTRY( "TestOpenInvalidIndexdb", Ccpixsearchertest::TestOpenInvalidIndexdbL ),
        ENTRY( "TestOpenValidIndexdb", Ccpixsearchertest::TestOpenValidIndexdbL ),
        ENTRY( "TestAddDocument", Ccpixsearchertest::TestAddDocumentL ),
        ENTRY( "TestAddCancel", Ccpixsearchertest::TestAddCancelL),
        ENTRY( "TestAsyncAddDocument", Ccpixsearchertest::TestAsyncAddDocumentL),
        ENTRY( "TestUpdateDocument", Ccpixsearchertest::TestUpdateDocumentL),
        ENTRY( "TestAsyncUpdate", Ccpixsearchertest::TestAsyncUpdateL),
        ENTRY( "TestDeleteDocument", Ccpixsearchertest::TestDeleteDocumentL),
        ENTRY( "TestAsyncDelete", Ccpixsearchertest::TestAsyncDeleteL),
        ENTRY( "TestReset", Ccpixsearchertest::TestResetL),
        ENTRY( "TestFlush", Ccpixsearchertest::TestFlushL),
        ENTRY( "TestAsyncFlush", Ccpixsearchertest::TestAsyncFlushL),
        ENTRY( "TestAsyncReset", Ccpixsearchertest::TestAsyncResetL),
        ENTRY( "TestHandleIndexingResult", Ccpixsearchertest::TestHandleIndexingResultL),
        ENTRY( "TestSetAnalyzerAsync", Ccpixsearchertest::TestSetAnalyzerAsyncL),
        ENTRY( "TestSearchSession", Ccpixsearchertest::TestSearchSessionL),
        ENTRY( "TestMultiThreading", Ccpixsearchertest::TestMultiThreadingL),
        ENTRY( "TestNoBoost", Ccpixsearchertest::TestNoBoostL),
        ENTRY( "TestBoost", Ccpixsearchertest::TestBoostL),
        ENTRY( "TestFieldBoost", Ccpixsearchertest::TestFieldBoostL),
        ENTRY( "TestStandardAnalyzer", Ccpixsearchertest::TestStandardAnalyzerL),
        ENTRY( "TestWhitespaceTokenizer", Ccpixsearchertest::TestWhitespaceTokenizerL),
        ENTRY( "TestRemoveSnowballAnalyzer", Ccpixsearchertest::TestRemoveSnowballAnalyzerL),
        ENTRY( "TestKnownTerm", Ccpixsearchertest::TestKnownTermL),
        ENTRY( "TestWildcardTerm", Ccpixsearchertest::TestWildcardTermL),
        ENTRY( "TestDefineVolumeWorksNonEmptyPath", Ccpixsearchertest::TestDefineVolumeWorksNonEmptyPathL),
        ENTRY( "TestDefineVolumeWorksWithEmptyPath", Ccpixsearchertest::TestDefineVolumeWorksWithEmptyPathL),
        ENTRY( "TestDefineVolumeErrorScenarios", Ccpixsearchertest::TestDefineVolumeErrorScenariosL),
        ENTRY( "TestSearchCancellation", Ccpixsearchertest::TestSearchCancellationL),        
        ENTRY( "TestSearchAsynchronous", Ccpixsearchertest::TestSearchAsynchronousL),
        ENTRY( "TestSearchLeavesIfNotCancelled", Ccpixsearchertest::TestSearchLeavesIfNotCancelledL),
        ENTRY( "TestOpenIndexDb", Ccpixsearchertest::TestOpenIndexDbL),
        ENTRY( "TestDeleteIndexDbWhileSearching", Ccpixsearchertest::TestDeleteIndexDbWhileSearchingL),
        ENTRY( "TestGetDocumentAsync", Ccpixsearchertest::TestGetDocumentAsyncL),
        ENTRY( "TestGetInvalidDocumentAsync", Ccpixsearchertest::TestGetInvalidDocumentAsyncL),
        ENTRY( "TestExerptLength", Ccpixsearchertest::TestExerptLengthL),
        
        //ADD NEW ENTRY HERE
        // [test cases entries] - Do not remove

        };

    const TInt count = sizeof( KFunctions ) / 
                        sizeof( TStifFunctionInfo );

    return RunInternalL( KFunctions, count, aItem );

    }

// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestOpenInvalidIndexdbL
// Example test method function.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestOpenInvalidIndexdbL( CStifItemParser& /*aItem*/ )
    {
    RSearchServerSession iSession;
    TInt error = KErrGeneral;
    User::LeaveIfError(iSession.Connect());
    iSession.DefineVolume(KFileBaseAppClassC, KNullDesC);
    _LIT(KInvalidQualifiedBaseAppClass, "@x:test invalid");
    _LIT( KTestFormBaseAppClassNoError, "TestOpenInvalidIndexdb: No Error" );
    CCPixIndexer* indexer = CCPixIndexer::NewLC(iSession);
    TRAPD(err, indexer->OpenDatabaseL(KInvalidQualifiedBaseAppClass) );   
    if(err == KErrCannotOpenDatabase)
        {
        error = KErrNone;
        }
    CleanupStack::PopAndDestroy(indexer);
    indexer = NULL;
    iSession.UnDefineVolume( KFileBaseAppClassC );
    iSession.Close();
    doLog( iLog, error, KTestFormBaseAppClassNoError );   
    return error;
    }

// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestOpenValidIndexdbL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestOpenValidIndexdbL( CStifItemParser& /*aItem*/ )
    {
    RSearchServerSession iSession;
    TInt error = KErrGeneral;
    User::LeaveIfError(iSession.Connect());
    iSession.DefineVolume(KFileBaseAppClassC, KNullDesC);
    _LIT( KTestFormBaseAppClassNoError, "TestOpenValidIndexdb: No Error" );
    CCPixIndexer* indexer = CCPixIndexer::NewLC(iSession);
    TRAPD(err, indexer->OpenDatabaseL( KFileBaseAppClassC ) );   
    if(err != KErrCannotOpenDatabase)
        {
        error = KErrNone;
        }
    CleanupStack::PopAndDestroy(indexer);
    indexer = NULL;
    iSession.UnDefineVolume( KFileBaseAppClassC );
    iSession.Close();
    doLog( iLog, error, KTestFormBaseAppClassNoError );
    return error;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestOpenValidIndexdbL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestAddDocumentL( CStifItemParser& aItem )
    {    
    RSearchServerSession iSession;
    TInt error = KErrNotFound;
    TPtrC fileName;
    TPtrC searchstring;
    aItem.GetNextString ( searchstring );
    User::LeaveIfError(iSession.Connect());
    iSession.DefineVolume(KFileBaseAppClassC, KNullDesC);
    _LIT( KTestFormBaseAppClassNoError, "TestAddDocumentL: No Error" );
    CCPixIndexer* indexer = CCPixIndexer::NewLC(iSession);
    TRAPD(err, indexer->OpenDatabaseL( KFileBaseAppClassC ) );      
    // creating CSearchDocument object with unique ID for this application
    
    if( aItem.GetNextString ( fileName ) == KErrNone )
        {
        CSearchDocument* index_item = CSearchDocument::NewLC(fileName, KNullDesC, KNullDesC, CSearchDocument::EFileParser);
        
        // Send for indexing
        if (indexer)
            {
            indexer->AddL(*index_item);
            }
        CleanupStack::PopAndDestroy(index_item);
        }        
    User::After((TTimeIntervalMicroSeconds32)30000000);
    TInt result =  doSearch( searchstring , KFileBaseAppClassC );
    if ( result > 0 )
        error = KErrNone;
    indexer->ResetL();
    CleanupStack::PopAndDestroy(indexer);
    indexer = NULL;
    iSession.UnDefineVolume( KFileBaseAppClassC );
    iSession.Close();
    doLog( iLog, error, KTestFormBaseAppClassNoError );
    return error;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestAddCancelL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestAddCancelL( CStifItemParser& aItem )
    {
    RSearchServerSession iSession;
    TInt error = KErrNotFound;
    TPtrC fileName;
    TPtrC searchstring;
    aItem.GetNextString ( searchstring );
    User::LeaveIfError(iSession.Connect());
    iSession.DefineVolume(KFileBaseAppClassC, KNullDesC);
    _LIT( KTestFormBaseAppClassNoError, "TestAddCancelL: No Error" );    
    CCPixIndexer* indexer = CCPixIndexer::NewLC(iSession);
    TRAPD(err, indexer->OpenDatabaseL( KFileBaseAppClassC ) );   
    // creating CSearchDocument object with unique ID for this application
    indexer->ResetL();
    if( aItem.GetNextString ( fileName ) == KErrNone )
        {
        CSearchDocument* index_item = CSearchDocument::NewLC(fileName, KNullDesC, KNullDesC, CSearchDocument::EFileParser);
        
        // Send for indexing
        if (indexer)
            {
            indexer->AddL(*index_item);
            }
        indexer->Cancel();// AddL is an atomic action, cannot cancel it
        CleanupStack::PopAndDestroy(index_item);
        }
    CleanupStack::PopAndDestroy(indexer);
    indexer = NULL;
    // Cancelled, but the document will still have been added to the IndexDB
    TInt result =  doSearch( searchstring , KFileBaseAppClassC );
    if ( result > 0 )
        error = KErrNone;
    iSession.UnDefineVolume( KFileBaseAppClassC );
    iSession.Close();
    doLog( iLog, error, KTestFormBaseAppClassNoError );
    return error;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestAsyncAddDocumentL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestAsyncAddDocumentL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestAsyncAddDocumentL: No Error" );
    
    CTestIndexer* testindexer = new CTestIndexer();
    testindexer->setUp();
    testindexer->InitSearcher();    
    testindexer->InitAsyncModules();       
    TRAPD ( error , testindexer->testAsyncAddL() );    
    testindexer->ReleaseSearcher();
    testindexer->ReleaseAsyncModules();
    testindexer->tearDown();
    delete testindexer;
    doLog( iLog, error, KTestFormBaseAppClassNoError );
    return error;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestUpdateDocumentL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestUpdateDocumentL( CStifItemParser& aItem )
    {
    RSearchServerSession iSession;
    TInt error = KErrNone;
    TPtrC fileName;
    TPtrC searchstring;
    aItem.GetNextString ( searchstring );
    User::LeaveIfError(iSession.Connect());
    iSession.DefineVolume(KFileBaseAppClassC, KNullDesC);
    _LIT( KTestFormBaseAppClassNoError, "TestUpdateDocumentL: No Error" );    
    CCPixIndexer* indexer = CCPixIndexer::NewLC(iSession);
    TRAPD(err, indexer->OpenDatabaseL( KFileBaseAppClassC ) );   
    // creating CSearchDocument object with unique ID for this application
    indexer->ResetL();
    if( aItem.GetNextString ( fileName ) == KErrNone )
        {
        CSearchDocument* index_item = CSearchDocument::NewLC(fileName, KNullDesC, KNullDesC, CSearchDocument::EFileParser);
        
        // Send for indexing
        if (indexer)
            {
            indexer->AddL(*index_item);
            }
        CleanupStack::PopAndDestroy(index_item);
        User::After((TTimeIntervalMicroSeconds32)30000000);
        TInt result =  doSearch( searchstring , KFileBaseAppClassC );
        index_item = CSearchDocument::NewLC(fileName, KNullDesC, KNullDesC, CSearchDocument::EFileParser);
        // Send for indexing
        if (indexer)
            {
            indexer->UpdateL(*index_item);
            }
        CleanupStack::PopAndDestroy(index_item);
        User::After((TTimeIntervalMicroSeconds32)30000000);
        TInt result1 = doSearch( searchstring , KFileBaseAppClassC );
        
        if ( result != result1)
            error = KErrUnknown;
        }
    CleanupStack::PopAndDestroy(indexer);
    indexer = NULL;    
    iSession.UnDefineVolume( KFileBaseAppClassC );
    iSession.Close();
    doLog( iLog, error, KTestFormBaseAppClassNoError );
    return error;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestAsyncUpdateL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestAsyncUpdateL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestAsyncUpdateL: No Error" );        
    CTestIndexer* testindexer = new CTestIndexer();
    testindexer->setUp();
    testindexer->InitSearcher();    
    testindexer->InitAsyncModules();   
    TRAPD ( error , testindexer->testAsyncUpdateL() );    
    testindexer->ReleaseSearcher();    
    testindexer->ReleaseAsyncModules();
    testindexer->tearDown();
    delete testindexer;
    doLog( iLog, error, KTestFormBaseAppClassNoError );
    return error;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestDeleteDocumentL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestDeleteDocumentL( CStifItemParser& aItem )
    {
    RSearchServerSession iSession;
    TInt error = KErrNone;
    TPtrC fileName;
    TPtrC searchstring;
    aItem.GetNextString ( searchstring );
    User::LeaveIfError(iSession.Connect());
    iSession.DefineVolume(KFileBaseAppClassC, KNullDesC);
    _LIT( KTestFormBaseAppClassNoError, "TestDeleteDocumentL: No Error" );    
    CCPixIndexer* indexer = CCPixIndexer::NewLC(iSession);
    TRAPD(err, indexer->OpenDatabaseL( KFileBaseAppClassC ) );   
    // creating CSearchDocument object with unique ID for this application
    
    if( aItem.GetNextString ( fileName ) == KErrNone )
        {
        CSearchDocument* index_item = CSearchDocument::NewLC(fileName, KNullDesC, KNullDesC, CSearchDocument::EFileParser);
        
        // Send for indexing
        if (indexer)
            {
            indexer->AddL(*index_item);
            }
        CleanupStack::PopAndDestroy(index_item);
        User::After((TTimeIntervalMicroSeconds32)30000000);
        TInt result =  doSearch( searchstring , KFileBaseAppClassC );
        
        // Send for indexing
        if (indexer)
            {
            indexer->DeleteL(fileName);
            }
        User::After((TTimeIntervalMicroSeconds32)30000000);
        TInt result1 = doSearch( searchstring , KFileBaseAppClassC );
        
        if ( (result-1) != result1)
            error = KErrUnknown;
        }
    indexer->ResetL();
    CleanupStack::PopAndDestroy(indexer);
    indexer = NULL;    
    iSession.UnDefineVolume( KFileBaseAppClassC );
    iSession.Close();
    doLog( iLog, error, KTestFormBaseAppClassNoError );
    return error;
    }

// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestAsyncDeleteL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestAsyncDeleteL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestAsyncDeleteL: No Error" );        
    CTestIndexer* testindexer = new CTestIndexer();
    testindexer->setUp();
    testindexer->InitSearcher();    
    testindexer->InitAsyncModules(); 
    TRAPD ( error , testindexer->testAsyncDeleteL() );    
    testindexer->ReleaseSearcher();    
    testindexer->ReleaseAsyncModules();
    testindexer->tearDown();
    delete testindexer;
    doLog( iLog, error, KTestFormBaseAppClassNoError );
    return error;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestResetL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestResetL( CStifItemParser& aItem )
    {
    RSearchServerSession iSession;
    TInt error = KErrUnknown;
    TPtrC fileName;
    TPtrC searchstring;
    aItem.GetNextString ( searchstring );
    User::LeaveIfError(iSession.Connect());
    iSession.DefineVolume(KFileBaseAppClassC, KNullDesC);
    _LIT( KTestFormBaseAppClassNoError, "TestResetL: No Error" );    
    CCPixIndexer* indexer = CCPixIndexer::NewLC(iSession);
    TRAPD(err, indexer->OpenDatabaseL( KFileBaseAppClassC ) );   
    // creating CSearchDocument object with unique ID for this application
    
    if( aItem.GetNextString ( fileName ) == KErrNone )
        {
        CSearchDocument* index_item = CSearchDocument::NewLC(fileName, KNullDesC, KNullDesC, CSearchDocument::EFileParser);
        
        // Send for indexing
        if (indexer)
            {
            indexer->AddL(*index_item);
            }
        CleanupStack::PopAndDestroy(index_item);
        User::After((TTimeIntervalMicroSeconds32)30000000);
        TInt result =  doSearch( searchstring , KFileBaseAppClassC );
        
        if ( result )
            {        
            // Send for indexing
            if (indexer)
                {
                indexer->ResetL();
                }        
            result = doSearch( searchstring , KFileBaseAppClassC );
            
            if ( !result )
                error = KErrNone;
              }
        }
    CleanupStack::PopAndDestroy(indexer);
    indexer = NULL;    
    iSession.UnDefineVolume( KFileBaseAppClassC );
    iSession.Close();
    doLog( iLog, error, KTestFormBaseAppClassNoError );
    return error;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestFlushL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestFlushL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestFlushL: No Error" );        
    CTestIndexer* testindexer = new CTestIndexer();
    testindexer->setUp();
    testindexer->InitSearcher();
    TRAPD ( error , testindexer->testFlushL() );    
    testindexer->ReleaseSearcher();
    testindexer->tearDown();
    delete testindexer;
    doLog( iLog, error, KTestFormBaseAppClassNoError );
    return error;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestAsyncFlushL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestAsyncFlushL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestAsyncFlushL: No Error" );        
    CTestIndexer* testindexer = new CTestIndexer();
    testindexer->setUp();
    testindexer->InitSearcher();    
    testindexer->InitAsyncModules();
    TRAPD ( error , testindexer->testAsyncFlushL() );    
    testindexer->ReleaseSearcher();    
    testindexer->ReleaseAsyncModules();
    testindexer->tearDown();
    delete testindexer;
    doLog( iLog, error, KTestFormBaseAppClassNoError );
    return error;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestAsyncResetL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestAsyncResetL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestAsyncResetL: No Error" );        
    CTestIndexer* testindexer = new CTestIndexer();
    testindexer->setUp();
    testindexer->InitSearcher();    
    testindexer->InitAsyncModules();
    TRAPD ( error , testindexer->testAsyncResetL() );    
    testindexer->ReleaseSearcher();    
    testindexer->ReleaseAsyncModules();
    testindexer->tearDown();
    delete testindexer;
    doLog( iLog, error, KTestFormBaseAppClassNoError );
    return error;
    }

// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestHandleIndexingResultL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestHandleIndexingResultL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestHandleIndexingResultL: No Error" );        
    CTestIndexer* testindexer = new CTestIndexer();
    testindexer->setUp();
    testindexer->InitSearcher();    
    testindexer->InitAsyncModules();
    TRAPD ( error , testindexer->testHandleIndexingResultL_Leaves() );    
    testindexer->ReleaseSearcher();    
    testindexer->ReleaseAsyncModules();
    testindexer->tearDown();
    delete testindexer;
    doLog( iLog, error, KTestFormBaseAppClassNoError );
    return error;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestSetAnalyzerAsyncL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestSetAnalyzerAsyncL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestSetAnalyzerAsyncL: No Error" );        
    CTestIndexer* testindexer = new CTestIndexer();
    testindexer->setUp();
    testindexer->InitAsyncModules();
    TRAPD ( error , testindexer->testSetAnalyzerAsync() );    
    testindexer->ReleaseAsyncModules();
    testindexer->tearDown();
    delete testindexer;
    doLog( iLog, error, KTestFormBaseAppClassNoError );
    return error;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestSearchSessionL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestSearchSessionL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestSearchSessionL: No Error" );        
    CTestIndexer* testindexer = new CTestIndexer();
    testindexer->setUp();
    TRAPD ( error , testindexer->testSearchSession() );
    testindexer->tearDown();
    delete testindexer;
    doLog( iLog, error, KTestFormBaseAppClassNoError );
    return error;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestMultiThreadingL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestMultiThreadingL( CStifItemParser& /*aItem*/ )
    {    
    _LIT( KTestFormBaseAppClassNoError, "TestMultiThreadingL: No Error" ); 
    CTestMultiThreading* multithreadtest = new CTestMultiThreading();
    TRAPD( err , multithreadtest->DoTestMultiThreadingL());
    delete multithreadtest;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestNoBoostL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestNoBoostL( CStifItemParser& /*aItem*/ )
    {    
    _LIT( KTestFormBaseAppClassNoError, "TestNoBoostL: No Error" ); 
    CTestBoost* testboost = new CTestBoost();
    testboost->setUp();
    TRAPD( err , testboost->testNoBoost());
    testboost->tearDown();
    delete testboost;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestBoostL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestBoostL( CStifItemParser& /*aItem*/ )
    {    
    _LIT( KTestFormBaseAppClassNoError, "TestBoostL: No Error" ); 
    CTestBoost* testboost = new CTestBoost();
    testboost->setUp();
    TRAPD( err , testboost->testBoost());
    testboost->tearDown();
    delete testboost;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestFieldBoostL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestFieldBoostL( CStifItemParser& /*aItem*/ )
    {    
    _LIT( KTestFormBaseAppClassNoError, "TestFieldBoostL: No Error" ); 
    CTestBoost* testboost = new CTestBoost();
    testboost->setUp();
    TRAPD( err , testboost->testFieldBoost());
    testboost->tearDown();
    delete testboost;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestStandardAnalyzerL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestStandardAnalyzerL( CStifItemParser& /*aItem*/ )
    {    
    _LIT( KTestFormBaseAppClassNoError, "TestStandardAnalyzerL: No Error" ); 
    CTestAnalyzer* testanalyser = new CTestAnalyzer();
    testanalyser->setUp();
    TRAPD( err , testanalyser->testStandardAnalyzer());
    testanalyser->tearDown();
    delete testanalyser;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestWhitespaceTokenizerL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestWhitespaceTokenizerL( CStifItemParser& /*aItem*/ )
    {    
    _LIT( KTestFormBaseAppClassNoError, "TestWhitespaceTokenizerL: No Error" ); 
    CTestAnalyzer* testanalyser = new CTestAnalyzer();
    testanalyser->setUp();
    TRAPD( err , testanalyser->testWhitespaceTokenizer());
    testanalyser->tearDown();
    delete testanalyser;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestRemoveSnowballAnalyzerL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestRemoveSnowballAnalyzerL( CStifItemParser& /*aItem*/ )
    {    
    _LIT( KTestFormBaseAppClassNoError, "TestRemoveSnowballAnalyzerL: No Error" ); 
    CTestAnalyzer* testanalyser = new CTestAnalyzer();
    testanalyser->setUp();
    TRAPD( err , testanalyser->RemovetestSnowballAnalyzer());
    testanalyser->tearDown();
    delete testanalyser;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestKnownTermL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestKnownTermL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestKnownTermL: No Error" ); 
    CTestSearcher* testsearcher = new CTestSearcher();
    testsearcher->initialsetUp();
    testsearcher->createandinitindexer();
    testsearcher->createsearcher();
    testsearcher->harvesttestcontent( 8 );
    TRAPD ( err , testsearcher->testKnownTermTestsL() );    
    testsearcher->ReleaseIndexer();
    testsearcher->ReleaseSearcher();
    testsearcher->tearDown();
    delete testsearcher;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestWildcardTermL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestWildcardTermL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestWildcardTermL: No Error" ); 
    CTestSearcher* testsearcher = new CTestSearcher();
    testsearcher->initialsetUp();
    testsearcher->createandinitindexer();
    testsearcher->createsearcher();
    testsearcher->harvesttestcontent( 8 );
    TRAPD ( err , testsearcher->testWildcardTermTests() );    
    testsearcher->ReleaseIndexer();
    testsearcher->ReleaseSearcher();
    testsearcher->tearDown();
    delete testsearcher;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestDefineVolumeWorksNonEmptyPathL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestDefineVolumeWorksNonEmptyPathL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestDefineVolumeWorksNonEmptyPathL: No Error" ); 
    CTestSearcher* testsearcher = new CTestSearcher();
    testsearcher->initialsetUp();
    TRAPD ( err , testsearcher->testDefineVolumeWorksNonEmptyPath() );
    testsearcher->tearDown();
    delete testsearcher;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestDefineVolumeWorksWithEmptyPathL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestDefineVolumeWorksWithEmptyPathL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestDefineVolumeWorksWithEmptyPathL: No Error" ); 
    CTestSearcher* testsearcher = new CTestSearcher();
    testsearcher->initialsetUp();
    TRAPD ( err , testsearcher->testDefineVolumeWorksWithEmptyPath() );
    testsearcher->tearDown();
    delete testsearcher;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestDefineVolumeErrorScenariosL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestDefineVolumeErrorScenariosL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestDefineVolumeErrorScenariosL: No Error" ); 
    CTestSearcher* testsearcher = new CTestSearcher();
    testsearcher->initialsetUp();
    TRAPD ( err , testsearcher->testDefineVolumeErrorScenarios() );
    testsearcher->tearDown();
    delete testsearcher;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestSearchCancellationL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestSearchCancellationL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestSearchCancellationL: No Error" ); 
    CTestSearcher* testsearcher = new CTestSearcher();
    testsearcher->initialsetUp();
    testsearcher->createandinitindexer();
    testsearcher->createsearcher();
    testsearcher->harvesttestcontent( 8 );
    testsearcher->InitAsyncComponents();
    TRAPD ( err , testsearcher->testSearchCancellation() );    
    testsearcher->ReleaseIndexer();
    testsearcher->ReleaseSearcher();
    testsearcher->ReleaseAsyncComponents();
    testsearcher->tearDown();
    delete testsearcher;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestSearchAsynchronousL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestSearchAsynchronousL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestSearchAsynchronousL: No Error" ); 
    CTestSearcher* testsearcher = new CTestSearcher();
    testsearcher->initialsetUp();
    testsearcher->createandinitindexer();
    testsearcher->createsearcher();
    testsearcher->harvesttestcontent( 4 );
    testsearcher->InitAsyncComponents();
    TRAPD ( err , testsearcher->testSearchAsynchronous() );    
    testsearcher->ReleaseIndexer();
    testsearcher->ReleaseSearcher();
    testsearcher->ReleaseAsyncComponents();
    testsearcher->tearDown();
    delete testsearcher;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestSearchLeavesIfNotCancelledL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestSearchLeavesIfNotCancelledL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestSearchLeavesIfNotCancelledL: No Error" ); 
    CTestSearcher* testsearcher = new CTestSearcher();
    testsearcher->initialsetUp();
    testsearcher->createandinitindexer();
    testsearcher->createsearcher();
    testsearcher->harvesttestcontent( 4 );
    testsearcher->InitAsyncComponents();
    TRAPD ( err , testsearcher->testSearchLeavesIfNotCancelled() );    
    testsearcher->ReleaseIndexer();
    testsearcher->ReleaseSearcher();
    testsearcher->ReleaseAsyncComponents();
    testsearcher->tearDown();
    delete testsearcher;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestOpenIndexDbL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestOpenIndexDbL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestOpenIndexDbL: No Error" ); 
    CTestSearcher* testsearcher = new CTestSearcher();
    testsearcher->initialsetUp();
    testsearcher->createandinitindexer();
    testsearcher->harvesttestcontent( 4 );
    testsearcher->InitAsyncComponents();
    TRAPD ( err , testsearcher->testOpenIndexDb() );    
    testsearcher->ReleaseIndexer();
    testsearcher->ReleaseAsyncComponents();
    testsearcher->tearDown();
    delete testsearcher;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestDeleteIndexDbWhileSearchingL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestDeleteIndexDbWhileSearchingL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestDeleteIndexDbWhileSearchingL: No Error" ); 
    CTestSearcher* testsearcher = new CTestSearcher();
    testsearcher->initialsetUp();    
    testsearcher->InitAsyncComponents();
    TRAPD ( err , testsearcher->testDeleteIndexDbWhileSearching() );
    testsearcher->ReleaseAsyncComponents();
    testsearcher->tearDown();
    delete testsearcher;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestGetDocumentAsyncL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestGetDocumentAsyncL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestGetDocumentAsyncL: No Error" ); 
    CTestSearcher* testsearcher = new CTestSearcher();
    testsearcher->initialsetUp();
    testsearcher->createandinitindexer();
    testsearcher->createsearcher();
    testsearcher->harvesttestcontent( 4 );
    testsearcher->InitAsyncComponents();
    TRAPD ( err , testsearcher->testGetDocumentAsync() );    
    testsearcher->ReleaseIndexer();
    testsearcher->ReleaseSearcher();
    testsearcher->ReleaseAsyncComponents();
    testsearcher->tearDown();
    delete testsearcher;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestGetInvalidDocumentAsyncL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestGetInvalidDocumentAsyncL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestGetInvalidDocumentAsyncL: No Error" ); 
    CTestSearcher* testsearcher = new CTestSearcher();
    testsearcher->initialsetUp();
    testsearcher->createandinitindexer();
    testsearcher->createsearcher();
    testsearcher->harvesttestcontent( 4 );
    testsearcher->InitAsyncComponents();
    TRAPD ( err , testsearcher->testGetInvalidDocumentAsync() );    
    testsearcher->ReleaseIndexer();
    testsearcher->ReleaseSearcher();
    testsearcher->ReleaseAsyncComponents();
    testsearcher->tearDown();
    delete testsearcher;
    doLog( iLog, err, KTestFormBaseAppClassNoError );
    return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::TestExerptLengthL
// -----------------------------------------------------------------------------
//
TInt Ccpixsearchertest::TestExerptLengthL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KTestFormBaseAppClassNoError, "TestExerptLengthL: No Error" );
    TInt err = KErrNone;
    CTestSearcher* testsearcher = new CTestSearcher();
    testsearcher->initialsetUp();
    testsearcher->createandinitindexer();
    testsearcher->createsearcher();
    err = testsearcher->testEcerptLenth();
    testsearcher->ReleaseSearcher();
    testsearcher->ReleaseIndexer();
    testsearcher->tearDown();
    doLog( iLog, err, KTestFormBaseAppClassNoError );
        return err;
    }
// -----------------------------------------------------------------------------
// Ccpixsearchertest::?member_function
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
/*
TInt Ccpixsearchertest::?member_function(
   CItemParser& aItem )
   {

   ?code

   }
*/

// ========================== OTHER EXPORTED FUNCTIONS =========================
// None

//  [End of File] - Do not remove
