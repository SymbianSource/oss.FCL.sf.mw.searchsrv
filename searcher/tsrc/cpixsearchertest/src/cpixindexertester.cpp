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
#include "cpixindexerterster.h"
#include "ccpixsearcher.h"
#include "CSearchDocument.h"
#include "CCPixIndexer.h"
#include "common.h"

#define TS_ASSERT( a ) \
    if ( !(a) ) User::Leave( KErrGeneral )

_LIT(KUniqueSearchTermInAct0Txt, "uniquewordonlyfoundonceincorpus");
_LIT(KQueryString, "act");

_LIT(KFileBaseAppClassC, "@c:test root file");
_LIT(KNonExistentField, "_foo");
_LIT(KAppClassMp3, MP3APPCLASS);

_LIT(KTestFileAct0, "c:\\Data\\cpixS60unittest\\act0.txt");
_LIT(KTestFileAct1, "c:\\Data\\cpixS60unittest\\act1.txt");
_LIT(KTestFileAct2, "c:\\Data\\cpixS60unittest\\act2.txt");
_LIT(KTestFileAct3, "c:\\Data\\cpixS60unittest\\act3.txt");
_LIT(KTestFileAct4, "c:\\Data\\cpixS60unittest\\act4.txt");
_LIT(KTestFileAct5, "c:\\Data\\cpixS60unittest\\act5.txt");
_LIT(KTestFileAct6, "c:\\Data\\cpixS60unittest\\act6.txt");
_LIT(KTestFileActComplete, "c:\\Data\\cpixS60unittest\\complete.txt");
_LIT(KTestFileActIndex, "c:\\Data\\cpixS60unittest\\index.txt");
_LIT(KTestFileActIntro, "c:\\Data\\cpixS60unittest\\introduction.txt");
// UCS-2 Little Endian
_LIT(KTestFileInvalidEncoding, "c:\\Data\\cpixS60unittest\\invalidEncoding.txt");
_LIT(KTestFileMp3, "c:\\Data\\cpixS60unittest\\One Step For Man.mp3");

CHandleIndexingResultLeaves::CHandleIndexingResultLeaves()
    {
    // iWait will cause waiting until some asynchronous event has happened
    iWait = new (ELeave) CActiveSchedulerWait;
    }

CHandleIndexingResultLeaves::~CHandleIndexingResultLeaves()
    {
    delete iWait;
    }

void CHandleIndexingResultLeaves::HandleIndexingResultL(TInt /*aError */)
    {
    iWait->AsyncStop();
    User::Leave(KErrCancel);
    }

void CTestIndexer::setUp()
    {
    User::LeaveIfError(iSession.Connect());
    iSession.DefineVolume(KFileBaseAppClassC, KNullDesC);

    iIndexer = CCPixIndexer::NewL(iSession);
    iIndexer->OpenDatabaseL(KFileBaseAppClassC);
    iIndexer->ResetL();
    }

void CTestIndexer::InitSearcher()
    {
    iSearcher = CCPixSearcher::NewL(iSession);
    iSearcher->OpenDatabaseL(KFileBaseAppClassC);
    }

void CTestIndexer::ReleaseSearcher()
    {
    delete iSearcher;
    iSearcher = NULL;
    }
void CTestIndexer::InitAsyncModules()
    {
    iHandleResultLeaves = new (ELeave) CHandleIndexingResultLeaves;
    iIndexerWait = new (ELeave) CActiveSchedulerWait;
    iMyAOClass = CAOTestClass::NewL(this);
    }
void CTestIndexer::ReleaseAsyncModules()
    {
    if( iMyAOClass )
        {
        delete iMyAOClass;
        iMyAOClass = NULL;
        }
    delete iHandleResultLeaves;
    iHandleResultLeaves = NULL;

    delete iIndexerWait;
    }
void CTestIndexer::tearDown()
    {
    iIndexer->ResetL();    
    delete iIndexer;
    iIndexer = NULL;
    iSession.UnDefineVolume(KFileBaseAppClassC);
    iSession.Close();
    }

void CTestIndexer::CreateFileIndexItemL(const TDesC& aFilename)
    {
    
    // creating CSearchDocument object with unique ID for this application
    CSearchDocument* index_item = CSearchDocument::NewLC(aFilename, KNullDesC, KNullDesC, CSearchDocument::EFileParser);
    
    // Send for indexing
    if (iIndexer)
        {
        iIndexer->AddL(*index_item);
        }
    CleanupStack::PopAndDestroy(index_item);
    }


TInt CTestIndexer::SearchForTextL(const TDesC& aQueryString, const TDesC& aDefaultField, TBool aFlush )
    {
    TInt estimatedDocumentCount(KErrNotFound);
    
    // Make sure CPix flush is done before searching
    if ( aFlush && iIndexer)
        {
        iIndexer->FlushL();
        }
    
    // Send for indexing
    if ( iSearcher )
        {
        estimatedDocumentCount = iSearcher->SearchL(aQueryString, aDefaultField);
        }
        
    return estimatedDocumentCount;
    }

TBool CTestIndexer::CheckBaseAppClassIsExpectedL(const TDesC& aExpectedAppClass, 
                                                 const TDesC& aDocUid)
    {
    TBool result(EFalse);

    iIndexer->FlushL();
    const TInt count = iSearcher->SearchL(aDocUid, TPtrC((TUint16*)LCPIX_DOCUID_FIELD));
    //  TODO XXX TIM why is count==2? This is not an expected result and needs 
    //  understanding.
#if 0
    // as searching with DocUid, the result count must be 1
    TS_ASSERT(count==1);
#endif // 0

    CSearchDocument* doc = iSearcher->GetDocumentL(0);
    TS_ASSERT(doc); // Not NULL

    const TInt cmp = aExpectedAppClass.Compare(doc->AppClass());
    if (cmp == KErrNone)
        {
        result = ETrue;
        }
    delete doc;
    doc = NULL;
    return result;
    }

void CTestIndexer::HandleOpenDatabaseResultL( TInt /* aError */ )
    {
    if (iIndexerWait && iIndexerWait->IsStarted())
        {
        // All done, signal that can continue now.
        iIndexerWait->AsyncStop();
        }
    }

void CTestIndexer::HandleSetAnalyzerResultL(TInt /*aError*/)
    {
    if (iIndexerWait && iIndexerWait->IsStarted())
        {
        // All done, signal that can continue now.
        iIndexerWait->AsyncStop();
        }
    }

void CTestIndexer::HandleIndexingResultL(TInt aError)
    {
    TS_ASSERT(aError == KErrNone);
    
    // if the test case started iIndexerWait, then stop it
    if (iIndexerWait && iIndexerWait->IsStarted())
        {
        iIndexerWait->AsyncStop();
        }
    }

// Timeout callback
void CTestIndexer::CallCompleted( int /* i */ )
    {
    if (iIndexerWait && iIndexerWait->IsStarted())
        {
        iIndexerWait->AsyncStop();
        }
    }

void CTestIndexer::testOpenIndexDb()
    {
    // Make sure cannot open an invalid index db
    _LIT(KInvalidQualifiedBaseAppClass, "@x:test invalid");
    CCPixIndexer* indexer = CCPixIndexer::NewLC(iSession);
    TRAPD(err, indexer->OpenDatabaseL(KInvalidQualifiedBaseAppClass) );
    TS_ASSERT(!indexer->IsDatabaseOpen());
    TS_ASSERT(err == KErrCannotOpenDatabase);
    CleanupStack::PopAndDestroy(indexer);

    indexer = NULL; 

    // check that same indexer object can open a second indexDb
    indexer = CCPixIndexer::NewLC(iSession);
    TRAP(err, indexer->OpenDatabaseL(KFileBaseAppClassC));
    TS_ASSERT(indexer->IsDatabaseOpen());
    TS_ASSERT(err == KErrNone);

    TRAP(err, indexer->OpenDatabaseL(KFileBaseAppClassC));
    TS_ASSERT(err == KErrNone);
    TS_ASSERT(indexer->IsDatabaseOpen());
    CleanupStack::PopAndDestroy(indexer);

    indexer = NULL; 

    // check that asynchronous open works as expected
    indexer = CCPixIndexer::NewLC(iSession);
    TRAP(err, indexer->OpenDatabaseL(*this, KFileBaseAppClassC));

    // If want to set a maximum timeout value for this test case then
    // uncomment this line, which will cause CTestSearcher::CallCompleted
    // to be triggered.
    iMyAOClass->StartL(1000000*10); //Async call: Maximum TimeOut time 10 seconds

    iIndexerWait->Start();
    TS_ASSERT(indexer->IsDatabaseOpen());
    CleanupStack::PopAndDestroy(indexer);
    }

void CTestIndexer::testAddL()
    {
    // There should be plenty of terms "act"  EXPECTED RESULTS > 0
    TInt result = SearchForTextL(KQueryString, KNullDesC);
    TS_ASSERT(result == 0);
    
    // Shakespeare's corpus files have no terms "ipod" in them EXPECTED RESULTS IS ZERO
    result = SearchForTextL(_L("ipod"), KNullDesC);
    TS_ASSERT(result == 0);

    // Shakespeare's corpus files have plenty of terms "act" in them EXPECTED RESULTS == 0, but not in this field
    result = SearchForTextL(KQueryString, KNonExistentField);
    TS_ASSERT(result == 0);

    TRAPD(err, CreateFileIndexItemL(KTestFileAct0));
    TS_ASSERT(err == KErrNone);
    TRAP(err, CreateFileIndexItemL(KTestFileAct1));
    TS_ASSERT(err == KErrNone);
    TRAP(err, CreateFileIndexItemL(KTestFileAct2));
    TS_ASSERT(err == KErrNone);
    TRAP(err, CreateFileIndexItemL(KTestFileAct3));
    TS_ASSERT(err == KErrNone);
    TRAP(err, CreateFileIndexItemL(KTestFileAct4));
    TS_ASSERT(err == KErrNone);
    TRAP(err, CreateFileIndexItemL(KTestFileAct5));
    TS_ASSERT(err == KErrNone);
    TRAP(err, CreateFileIndexItemL(KTestFileAct6));
    TS_ASSERT(err == KErrNone);

    // Attempting to add a document with invalid encoding must fail
    TRAP(err, CreateFileIndexItemL(KTestFileInvalidEncoding));
    TS_ASSERT(err == KErrCannotAddDocument);

    // Add a MP3 and test that the base app class is MP3APPCLASS.
    TRAP(err, CreateFileIndexItemL(KTestFileMp3));
    TS_ASSERT(err == KErrNone);
    const TBool isMp3 = CheckBaseAppClassIsExpectedL(
                            KAppClassMp3, KTestFileMp3);
    TS_ASSERT(isMp3);

    TRAP(err, CreateFileIndexItemL(KTestFileActComplete));
    TS_ASSERT(err == KErrNone);
    TRAP(err, CreateFileIndexItemL(KTestFileActIndex));
    TS_ASSERT(err == KErrNone);
    TRAP(err, CreateFileIndexItemL(KTestFileActIntro));
    TS_ASSERT(err == KErrNone);

    // There should be plenty of terms "act"  EXPECTED RESULTS > 0
    result = SearchForTextL(KQueryString, KNullDesC);
    TS_ASSERT(result > 0);
    
    // Shakespeare's corpus files have no terms "ipod" in them EXPECTED RESULTS IS ZERO
    result = SearchForTextL(_L("ipod"), KNullDesC);
    TS_ASSERT(result == 0);

    // Shakespeare's corpus files have plenty of terms "act" in them EXPECTED RESULTS == 0, but not in this field
    result = SearchForTextL(KQueryString, KNonExistentField);
    TS_ASSERT(result == 0);
    }

void CTestIndexer::testAsyncAddL()
    {
    // At the start this term will not be in the indexDb
    TInt result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 0); 

    //Improve code coverage
    CSearchDocument* index_item = CSearchDocument::NewL(KTestFileAct0, KNullDesC, _L("excerpt"), CSearchDocument::EFileParser);    
    CleanupStack::PushL(index_item);
    iIndexer->AddL(*this, *index_item);
    CleanupStack::PopAndDestroy(index_item);

    iIndexerWait->Start();
    User::After( 30000000 );
    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC, false);
    TS_ASSERT(result == 1);
    CSearchDocument* doc = iSearcher->GetDocumentL(0);
    TS_ASSERT(doc != NULL);
    TS_ASSERT(doc->Excerpt() != KNullDesC);//Should not be NULL
    delete doc;    
    }

void CTestIndexer::testCancelAddL()
    {
    // At the start this term will not be in the indexDb
    TInt result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 0); 

    CSearchDocument* index_item = CSearchDocument::NewLC(KTestFileAct0, KNullDesC, KNullDesC, CSearchDocument::EFileParser);

    iIndexer->AddL(*this, *index_item);
    iIndexer->Cancel(); // AddL is an atomic action, cannot cancel it
    CleanupStack::PopAndDestroy(index_item);

    // Cancelled, but the document will still have been added to the IndexDB
    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 1);
    }

void CTestIndexer::testUpdateL()
    {
    TInt result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 0);

    // add the file once
    CreateFileIndexItemL(KTestFileAct0);
    
    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 1);

    // now update the same document
    CSearchDocument* index_item = CSearchDocument::NewLC(KTestFileAct0, 
            KNullDesC, KNullDesC, CSearchDocument::EFileParser);
    iIndexer->UpdateL(*index_item);
    CleanupStack::PopAndDestroy(index_item);
    
    // As the document has been updated the amount of search hits
    // must remain the same as before.
    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 1);
    }


void CTestIndexer::testAsyncUpdateL()
    {
    TInt result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 0);

    // add the file once
    CreateFileIndexItemL(KTestFileAct0);
    
    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 1);

    // now update the same document
    CSearchDocument* index_item = CSearchDocument::NewLC(KTestFileAct0, 
            KNullDesC, KNullDesC, CSearchDocument::EFileParser);
    iIndexer->UpdateL(*this, *index_item);
    CleanupStack::PopAndDestroy(index_item);

    iIndexerWait->Start();

    // As the document has been updated the amount of search hits
    // must remain the same as before.
    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 1);
    }

void CTestIndexer::testDeleteL()
    {
    // At the start this term will not be in the indexDb
    TInt result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 0); 

    CreateFileIndexItemL(KTestFileAct0);
    CreateFileIndexItemL(KTestFileAct1);
    CreateFileIndexItemL(KTestFileAct2);
    CreateFileIndexItemL(KTestFileAct3);
    CreateFileIndexItemL(KTestFileAct4);
    CreateFileIndexItemL(KTestFileAct5);
    CreateFileIndexItemL(KTestFileAct6);
    CreateFileIndexItemL(KTestFileActComplete);
    CreateFileIndexItemL(KTestFileActIndex);
    CreateFileIndexItemL(KTestFileActIntro);

    // This term should only result in one result in (KTestFileAct0)
    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 1); 

    // Now delete the DocUid containing the above search result.
    if (iIndexer)
        {
        iIndexer->DeleteL(KTestFileAct0);
        }

    // Now that deleted act0.txt, the same search must yield zero results.
    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 0);
    }

void CTestIndexer::testAsyncDeleteL()
    {
    // At the start this term will not be in the indexDb
    TInt result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 0); 

    CreateFileIndexItemL(KTestFileAct0);
    CreateFileIndexItemL(KTestFileAct1);
    CreateFileIndexItemL(KTestFileAct2);
    CreateFileIndexItemL(KTestFileAct3);
    CreateFileIndexItemL(KTestFileAct4);
    CreateFileIndexItemL(KTestFileAct5);
    CreateFileIndexItemL(KTestFileAct6);
    CreateFileIndexItemL(KTestFileActComplete);
    CreateFileIndexItemL(KTestFileActIndex);
    CreateFileIndexItemL(KTestFileActIntro);

    // This term should only result in one result in (KTestFileAct0)
    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 1); 

    // Now delete the DocUid containing the above search result.
    if (iIndexer)
        {
        iIndexer->DeleteL(*this, KTestFileAct0);
        }
    iIndexerWait->Start();

    // Now that deleted act0.txt, the same search must yield zero results.
    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 0);
    }

void CTestIndexer::testResetL()
    {
    // At the start this term will not be in the indexDb
    TInt result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 0); 

    CreateFileIndexItemL(KTestFileAct0);

    // This term should only result in one result in (KTestFileAct0)
    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 1); 

    iIndexer->ResetL();

    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 0);
    }

void CTestIndexer::testFlushL()
    {
    // At the start this term will not be in the indexDb
    TInt result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 0); 

    // creating CSearchDocument object with unique ID for this application
    CSearchDocument* index_item = CSearchDocument::NewLC(KTestFileAct0, KNullDesC, KNullDesC, CSearchDocument::EFileParser);

    // Send for indexing
    if (iIndexer)
        {
        iIndexer->AddL(*index_item);
        }
    CleanupStack::PopAndDestroy(index_item);
    
    // No flush. Term should not be in the indexDb
    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    
    // TODO: Feature not yet in CPix. Change when flush is required.
    //TS_ASSERT(result == 0); 
    TS_ASSERT(result == 1); 

    iIndexer->FlushL();

    // After flush there should be one result.
    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 1); 
    }

void CTestIndexer::testAsyncFlushL()
    {
    // At the start this term will not be in the indexDb
    TInt result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 0); 

    // creating CSearchDocument object with unique ID for this application
    CSearchDocument* index_item = CSearchDocument::NewLC(KTestFileAct0, KNullDesC, KNullDesC, CSearchDocument::EFileParser);

    // Send for indexing
    if (iIndexer)
        {
        iIndexer->AddL(*index_item);
        }
    CleanupStack::PopAndDestroy(index_item);
    
    // No flush. Term should not be in the indexDb
    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC, false);
    
    // TODO: Feature not yet in CPix. Change when flush is required.
    //TS_ASSERT(result == 0); 
    TS_ASSERT(result == 0); 

    iIndexer->FlushL(*this);
    iIndexerWait->Start();

    // After flush term should be in the indexDb
    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 1); 
    }

void CTestIndexer::testAsyncResetL()
    {
    // At the start this term will not be in the indexDb
    TInt result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 0); 

    CreateFileIndexItemL(KTestFileAct0);

    // This term should only result in one result in (KTestFileAct0)
    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 1); 

    iIndexer->ResetL(*this);
    
    iIndexerWait->Start();

    result = SearchForTextL(KUniqueSearchTermInAct0Txt, KNullDesC);
    TS_ASSERT(result == 0);
    }

void CTestIndexer::testHandleIndexingResultL_Leaves()
    {
    CSearchDocument* index_item = CSearchDocument::NewLC(KTestFileAct0, KNullDesC, KNullDesC, CSearchDocument::EFileParser);

    iIndexer->AddL(*iHandleResultLeaves, *index_item);

    CleanupStack::PopAndDestroy(index_item);

    iHandleResultLeaves->iWait->Start();
    // There should be no panic !!!
    }

void CTestIndexer::testSetAnalyzerAsync()
    {
    TInt result = 0;//pass default
    if(iIndexer->IsDatabaseOpen())
        {
        const TDesC& db = iIndexer->GetBaseAppClass();//Increase Coverage
        if(db.Length() == 0)
            {
            //Set result to Zero
            result = 1; //DB failed
            }
        }
    else
        {
        const TDesC& db = iIndexer->GetBaseAppClass();//Increase Coverage
        }
    iIndexer->SetAnalyzerL(*this,_L( "" CPIX_ANALYZER_STANDARD ));
    iIndexerWait->Start();//wait till it returns
    TS_ASSERT(result == 0);
    }

void CTestIndexer::testSearchSession()
    {
    TInt result = 0;//pass default
    result = iSession.ForceHouseKeeping();
    TS_ASSERT(result == KErrNone);
    result = iSession.ContinueHouseKeeping();
    TS_ASSERT(result == KErrNone);
    result = iSession.StopHouseKeeping();
    TS_ASSERT(result == KErrNone);
    }
