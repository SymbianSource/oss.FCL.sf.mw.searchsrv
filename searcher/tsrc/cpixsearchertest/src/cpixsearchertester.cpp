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
#include "cpixsearchertester.h"
//#include "TestDriver.h"
//#include "Logger.h"
#include <f32file.h>
#include "indevicecfg.h"

#include "CCPixSearcher.h"
#include "CSearchDocument.h"
#include "CCPixIndexer.h"
#include "common.h"

#define TS_ASSERT(a) \
    if ( !a ) return KErrNotFound 

_LIT(KQueryString, "act");

_LIT(KCpixRegFile, "c:\\private\\2001f6f7\\cpixreg.txt");

_LIT(KTestFileBaseAppClassC, "@c:test root file");

_LIT(KAppClassField, CPIX_APPCLASS_FIELD);
_LIT(KNonExistentField, "_foo");
_LIT(KRootFileUserTxt, TEXTAPPCLASS);

_LIT8(KRootMapsGbr8, "@GBR:foo bar root maps");
_LIT8(KRootMapsGbrPath8, "c:\\Data\\foo\\bar\\root\\maps\\street\\gbr");
_LIT(KRootMapsGbr, "@GBR:foo bar root maps");
_LIT(KRootMapsGbrPath, "c:\\Data\\foo\\bar\\root\\maps\\street\\gbr");

_LIT(KInvalidQualifiedBaseAppClass1, "@:foo bar root maps");
_LIT(KInvalidQualifiedBaseAppClass2, ":foo bar root maps");
_LIT(KInvalidQualifiedBaseAppClass3, "foo bar root maps");
_LIT(KInvalidQualifiedBaseAppClass4, "root file");


const TInt KMaxFileSize = 400;

TInt CTestSearcher::SearchForTextL(const TDesC& aQueryString, const TDesC& aDefaultField)
    {
    TInt documentCount(KErrNotFound);
    // Send for indexing
    if ( iSearcher )
        {
        documentCount = iSearcher->SearchL(aQueryString, aDefaultField);
        }
    return documentCount;
    }

void CTestSearcher::AddFileToIndexL(const TDesC& aFileName)
    {
    CSearchDocument* doc = CSearchDocument::NewLC(aFileName, KNullDesC, KNullDesC, CSearchDocument::EFileParser);   
    iIndexer->AddL(*doc);
    CleanupStack::PopAndDestroy(doc);
    }

void CTestSearcher::setUp()
    {
    iFs.Connect();
    User::LeaveIfError(iSession.Connect());

    
    iSession.UnDefineVolume(KTestFileBaseAppClassC);
    // define a test volume
    iSession.DefineVolume(KTestFileBaseAppClassC, KNullDesC);

    iIndexer = CCPixIndexer::NewL(iSession);
    iIndexer->OpenDatabaseL(KTestFileBaseAppClassC);
    iIndexer->ResetL();
    
    AddFileToIndexL(_L("c:\\Data\\cpixS60unittest\\act0.txt")); 
    AddFileToIndexL(_L("c:\\Data\\cpixS60unittest\\act1.txt")); 
    AddFileToIndexL(_L("c:\\Data\\cpixS60unittest\\act2.txt")); 
    AddFileToIndexL(_L("c:\\Data\\cpixS60unittest\\act3.txt")); 
    AddFileToIndexL(_L("c:\\Data\\cpixS60unittest\\act4.txt")); 
    AddFileToIndexL(_L("c:\\Data\\cpixS60unittest\\act5.txt")); 
    AddFileToIndexL(_L("c:\\Data\\cpixS60unittest\\act6.txt")); 
    AddFileToIndexL(_L("c:\\Data\\cpixS60unittest\\complete.txt")); 
    AddFileToIndexL(_L("c:\\Data\\cpixS60unittest\\index.txt"));    
    AddFileToIndexL(_L("c:\\Data\\cpixS60unittest\\introduction.txt")); 

    iSearcher = CCPixSearcher::NewL(iSession);
    iSearcher->OpenDatabaseL(KTestFileBaseAppClassC);

    // iWait will cause waiting until some asynchronous event has happened
    iWait = new (ELeave) CActiveSchedulerWait;
    iMyAOClass = CAOTestClass::NewL(this);
    iDocumentCount = 0;
    iDocument = NULL;
    iHandleDocumentLFunctionCalled = EFalse;
    iHandleSetAnalyzerCalled = EFalse;
    }

void CTestSearcher::tearDown()
    {
    iDocumentCount = 0;

    delete iSearcher;
    iSearcher = NULL;

    if ( iDocument )
        {
        delete iDocument;
        iDocument = NULL;
        }

    if (iIndexer)
        {
        iIndexer->ResetL();
        }

    if( iWait )
        {
        delete iWait;
        iWait = NULL;
        }
    
    if( iMyAOClass )
        {
        delete iMyAOClass;
        iMyAOClass = NULL;
        }

    delete iIndexer;
    iIndexer = NULL;

    // undefine a test volume
    iSession.UnDefineVolume(KTestFileBaseAppClassC);

    iSession.Close();
    iFs.Close();
    }


void CTestSearcher::HandleOpenDatabaseResultL( TInt /* aError */ )
    {
    if (iWait && iWait->IsStarted())
        {
        // All done, signal that can continue now.
        iWait->AsyncStop();
        }
    }

void CTestSearcher::HandleSearchResultsL(TInt /* aError */, TInt aEstimatedResultCount)
    {
    iDocumentCount = aEstimatedResultCount;
    
    if (iWait && iWait->IsStarted())
        {
        // All done, signal that can continue now.
        iWait->AsyncStop();
        }
    }

void CTestSearcher::HandleSetAnalyzerResultL(TInt /*aError*/)
    {
    iHandleSetAnalyzerCalled = ETrue;
    if (iWait && iWait->IsStarted())
        {
        // All done, signal that can continue now.
        iWait->AsyncStop();
        }
    }


void CTestSearcher::HandleDocumentL(TInt /* aError */, CSearchDocument* aDocument)
    {
    iHandleDocumentLFunctionCalled = ETrue;
    if (iWait && iWait->IsStarted())
        {
        if ( iDocument )
            {
            delete iDocument;
            iDocument = NULL;
            }

        iDocument = aDocument;
        // All done, signal that can continue now.
        iWait->AsyncStop();
        }
    }

// Timeout callback
void CTestSearcher::CallCompleted( int /* i */ )
    {
    if (iWait && iWait->IsStarted())
        {
        iWait->AsyncStop();
        }
    }

TInt CTestSearcher::testKnownTermTestsL()
    {
    // Shakespeare's corpus files have plenty of terms "act" in them EXPECTED RESULTS > 0
    TInt result = SearchForTextL(KQueryString, KNullDesC);
    TS_ASSERT(result > 0);
    
    // Shakespeare's corpus files have plenty of terms "scene" in them EXPECTED RESULTS > 0
    result = SearchForTextL(_L("scene"), KNullDesC);
    TS_ASSERT(result > 0);

    if(result > 0)
        {
        for (TInt i=0; i<result; i++)
            {
            CSearchDocument* document = iSearcher->GetDocumentL(i);     
            //TS_ASSERT(document != NULL);
            delete document;
            }
        }
        
    
    // Shakespeare's corpus files have no terms "ipod" in them EXPECTED RESULTS IS ZERO
    result = SearchForTextL(_L("ipod"), KNullDesC);
    TS_ASSERT(result == 0);

    // Shakespeare's corpus files have plenty of terms "act" in them EXPECTED RESULTS == 0, but not in this field
    result = SearchForTextL(KQueryString, KNonExistentField);
    TS_ASSERT(result == 0);

    // can search for a known value in a different field
    result = SearchForTextL(KRootFileUserTxt, KAppClassField);
    TS_ASSERT(result > 0);

    // can search for a known value in a different field
//  result = SearchForTextL(KRootFileUserTxt, KNonExistentField);
//  TS_ASSERT(result == 0); // TODO XXX TIM why does searching for an existing term in a non-existent field return a non-zero value
    return KErrNone;
    }

void CTestSearcher::testWildcardTermTests()
    {
    // Shakespeare's corpus files have plenty of terms "act" in them EXPECTED RESULTS > 0
    TInt result = SearchForTextL(_L("a*t"), KNullDesC);
    //TS_ASSERT(result > 0);

    // Shakespeare's corpus files have plenty of terms "act" in them EXPECTED RESULTS > 0
    result = SearchForTextL(_L("sc*ne"), KNullDesC);
   // TS_ASSERT(result > 0);
    }

void CTestSearcher::testDefineVolumeWorksNonEmptyPath()
    {
    TBuf8<KMaxFileSize> cpixRegBuf;

    // open up the cpixreg.txt file again and make sure 
    // that it IS NOT in there
    TInt err = iFile.Open(iFs, KCpixRegFile, EFileStreamText);
    iFile.Read(0, cpixRegBuf);
    err = cpixRegBuf.Find(KRootMapsGbr8());
   // TS_ASSERT(KErrNotFound == err);
    err = cpixRegBuf.Find(KRootMapsGbrPath8());
   // TS_ASSERT(KErrNotFound == err);
    iFile.Close();

    // define a new volume
    err = iSession.DefineVolume(KRootMapsGbr, KRootMapsGbrPath);
   // TS_ASSERT(err == KErrNone);

    // open up the cpixreg.txt file and make sure that it IS in there
    err = iFile.Open(iFs, KCpixRegFile, EFileStreamText);
    iFile.Read(0, cpixRegBuf);
    err = cpixRegBuf.Find(KRootMapsGbr8());
   // TS_ASSERT(KErrNotFound != err);
    err = cpixRegBuf.Find(KRootMapsGbrPath8());
   // TS_ASSERT(KErrNotFound != err);
    iFile.Close();

    // now undefine the same volume
    err = iSession.UnDefineVolume(KRootMapsGbr);
   // TS_ASSERT(err == KErrNone);

    // open up the cpixreg.txt file again and make sure 
    // that it IS NOT in there
    err = iFile.Open(iFs, KCpixRegFile, EFileStreamText);
    iFile.Read(0, cpixRegBuf);
    err = cpixRegBuf.Find(KRootMapsGbr8());
   // TS_ASSERT(KErrNotFound == err);
    err = cpixRegBuf.Find(KRootMapsGbrPath8());
   // TS_ASSERT(KErrNotFound == err);
    iFile.Close();
    }

void CTestSearcher::testDefineVolumeWorksWithEmptyPath()
    {
    TBuf8<KMaxFileSize> cpixRegBuf;

    // open up the cpixreg.txt file again and make sure 
    // that it IS NOT in there
    TInt err = iFile.Open(iFs, KCpixRegFile, EFileStreamText);
    iFile.Read(0, cpixRegBuf);
    err = cpixRegBuf.Find(KRootMapsGbr8());
  //  TS_ASSERT(KErrNotFound == err);
    err = cpixRegBuf.Find(KRootMapsGbrPath8());
  //  TS_ASSERT(KErrNotFound == err);
    iFile.Close();

    // define a new volume
    err = iSession.DefineVolume(KRootMapsGbr, KNullDesC);
  //  TS_ASSERT(err == KErrNone);

    // open up the cpixreg.txt file and make sure that it IS in there
    err = iFile.Open(iFs, KCpixRegFile, EFileStreamText);
    iFile.Read(0, cpixRegBuf);
    err = cpixRegBuf.Find(KRootMapsGbr8());
  //  TS_ASSERT(KErrNotFound != err);
    iFile.Close();

    // now try to redefine the same volume again
    err = iSession.DefineVolume(KRootMapsGbr, KNullDesC);
  //  TS_ASSERT(err == KErrNone);

    // now undefine the same volume
    err = iSession.UnDefineVolume(KRootMapsGbr);
  //  TS_ASSERT(err == KErrNone);

    // open up the cpixreg.txt file again and make sure 
    // that it IS NOT in there
    err = iFile.Open(iFs, KCpixRegFile, EFileStreamText);
    iFile.Read(0, cpixRegBuf);
    err = cpixRegBuf.Find(KRootMapsGbr8());
  //  TS_ASSERT(KErrNotFound == err);
    iFile.Close();
    }

void CTestSearcher::testDefineVolumeErrorScenarios()
    {
    // Attempt to define a volume for a invalid qualified base app classes.
    TInt result = iSession.DefineVolume(KInvalidQualifiedBaseAppClass1, KNullDesC);
 //   TS_ASSERT(result == KErrCannotDefineVolume);

    result = iSession.DefineVolume(KInvalidQualifiedBaseAppClass2, KNullDesC);
 //   TS_ASSERT(result == KErrCannotDefineVolume);

    result = iSession.DefineVolume(KInvalidQualifiedBaseAppClass3, KNullDesC);
  //  TS_ASSERT(result == KErrCannotDefineVolume);

    result = iSession.DefineVolume(KInvalidQualifiedBaseAppClass4, KNullDesC);
  //  TS_ASSERT(result == KErrCannotDefineVolume);

    // Attempt to undefine a volume that was not defined.
    result = iSession.UnDefineVolume(KRootMapsGbr);
  //  TS_ASSERT(result == KErrNone);
    }

void CTestSearcher::testSearchCancellation()
    {
    // Test basic cancellation of search request
    iSearcher->SearchL(*this, KQueryString);
    iSearcher->Cancel();

    // If want to set a maximum timeout value for this test case then
    // uncomment this line, which will cause CTestSearcher::CallCompleted
    // to be triggered.
    iMyAOClass->StartL(1000000*20); //Async call: Maximum TimeOut time 20 seconds

    // Wait until either HandleSearchResultsL or Timeout exceeded
    iWait->Start();
   // TS_ASSERT(iDocumentCount == 0);

    // Now make sure that subsequent searches work correctly
    iSearcher->SearchL(*this, KQueryString);
    // Wait until HandleSearchResultsL completes
    iWait->Start();

   // TS_ASSERT(iDocumentCount > 0);
    }

void CTestSearcher::testSearchAsynchronous()
    {
    iSearcher->SearchL(*this, KQueryString);
    // Wait until HandleSearchResultsL completes
    iWait->Start();

  //  TS_ASSERT(iDocumentCount > 0);
    }

void CTestSearcher::testSearchLeavesIfNotCancelled()
    {
    // Perform first search
    TRAPD(err, iSearcher->SearchL(*this, KQueryString) );
   // TS_ASSERT(iDocumentCount == 0);
   // TS_ASSERT(err == KErrNone);

    // before ::HandleSearchResults has been called, 
    // initiate a second search. 
    TRAP(err, iSearcher->SearchL(*this, KQueryString));
  //  TS_ASSERT(err == KErrInUse);

    // Wait until HandleSearchResultsL completes
    iWait->Start();

   // TS_ASSERT(iDocumentCount > 0);
    }

void CTestSearcher::testOpenIndexDb()
    {
    // Make sure cannot open an invalid index db
    _LIT(KInvalidQualifiedBaseAppClass, "@x:test invalid");
    CCPixSearcher* searcher = CCPixSearcher::NewLC(iSession);
    TRAPD(err, searcher->OpenDatabaseL(KInvalidQualifiedBaseAppClass) );
   // TS_ASSERT(!searcher->IsDatabaseOpen());
   // TS_ASSERT(err == KErrCannotOpenDatabase);
    TRAP(err, searcher->SearchL(KQueryString));
   // TS_ASSERT(err == KErrNotReady);
    CleanupStack::PopAndDestroy(searcher);

    searcher = NULL; 

    // check that same searcher object can open a second indexDb
    searcher = CCPixSearcher::NewLC(iSession);
    TRAP(err, searcher->OpenDatabaseL(_L("root")));
  //  TS_ASSERT(searcher->IsDatabaseOpen());
    searcher->SearchL(KQueryString);
  //  TS_ASSERT(err == KErrNone);

    TRAP(err, searcher->OpenDatabaseL(KTestFileBaseAppClassC));
   // TS_ASSERT(err == KErrNone);
   // TS_ASSERT(searcher->IsDatabaseOpen());
    const TInt docCount = searcher->SearchL(KQueryString);
  //  TS_ASSERT(docCount > 0);
    CleanupStack::PopAndDestroy(searcher);

    searcher = NULL; 

    // check that asynchronous open works as expected
    searcher = CCPixSearcher::NewLC(iSession);
    TRAP(err, searcher->OpenDatabaseL(*this, _L("root")));

    // If want to set a maximum timeout value for this test case then
    // uncomment this line, which will cause CTestSearcher::CallCompleted
    // to be triggered.
    iMyAOClass->StartL(1000000*10); //Async call: Maximum TimeOut time 10 seconds

    iWait->Start();
  //  TS_ASSERT(searcher->IsDatabaseOpen());
    CleanupStack::PopAndDestroy(searcher);
    }

void CTestSearcher::testDeleteIndexDbWhileSearching()
    {
    CCPixSearcher* searcher = CCPixSearcher::NewLC(iSession);
    TRAPD(err, searcher->OpenDatabaseL(KTestFileBaseAppClassC) );
   // TS_ASSERT(searcher->IsDatabaseOpen());
  //  TS_ASSERT(searcher->GetBaseAppClass() != KNullDesC);
    //Set Standard Analyzer to improve code coverage of search
    searcher->SetAnalyzerL(*this,_L( "" CPIX_ANALYZER_STANDARD ));
    iWait->Start(); //Start Wait AO
   // TS_ASSERT(iHandleSetAnalyzerCalled);
    TRAP(err, searcher->SearchL(*this, KQueryString));
    // don't wait for the search results.
    CleanupStack::PopAndDestroy(searcher);
    searcher = NULL; 
    // is there a panic???
    }

void CTestSearcher::testGetDocumentAsync()
    {
    const TInt result = SearchForTextL(KQueryString, KNullDesC);
  //  TS_ASSERT(result > 0);

    TInt loopCount(0);

    for (loopCount=0; loopCount<result; loopCount++)
        {
        iSearcher->GetDocumentL(loopCount, *this);
        iWait->Start();
   //     TS_ASSERT(iDocument != NULL);
        delete iDocument;
        iDocument = NULL;
        }

  //  TS_ASSERT(loopCount == result);
    }

void CTestSearcher::testGetInvalidDocumentAsync()
    {
    const TInt result = SearchForTextL(KQueryString, KNullDesC);
  //  TS_ASSERT(result > 0);

    // request a document that does not exist. 
    TRAPD(err, iSearcher->GetDocumentL(result+1, *this));
    // above call is not exepected to leave
 //   TS_ASSERT(err == KErrNone);

   iMyAOClass->StartL(1000000*10); //Async call: Maximum TimeOut time 10 seconds

    iWait->Start();

    // Either the iMyAOClass Timeout executed or the the HandleDocumentL
    // function. If the HandleDocumentL function was executed, then
    // this assert will be true.
  //  TS_ASSERT(iHandleDocumentLFunctionCalled);
    // as requested an invalid document, this must be NULL
  //  TS_ASSERT(iDocument == NULL);
    }
