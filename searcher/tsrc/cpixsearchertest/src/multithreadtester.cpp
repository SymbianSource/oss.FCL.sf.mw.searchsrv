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
#include "multithreadtester.h"
#include "RSearchServerSession.h"
#include "CCPixIndexer.h"
#include "CCPixSearcher.h"
#include "CSearchDocument.h"
#include <e32math.h>

#define KIndexingWorkerCount 4
#define KSearchingWorkerCount 16
#define KTermDeviation 10 // The bigger this word, the more different term 
#define KTermCountDeviation 10 // The bigger this word, the more terms per document
_LIT( KContentsField, "Contents " ); 

class CWorker : public CBase 
    {
public: 
    CWorker() 
    : iWorkerId( iNextFreeWorkerId++ ), 
      iActive( EFalse ), 
      iThread(), 
      iSemaphore()
        {
        }

    void ConstructL() 
        {
        User::LeaveIfError( iSemaphore.CreateLocal( 0 ) ); 
        }
    
    ~CWorker() 
        {
        if ( iActive ) 
            {
            Finish(); 
            }
        iSemaphore.Close(); 
        }
    
    TInt WorkerId() 
        {
        return iWorkerId; 
        }

    TInt Error() 
        {
        return iError; 
        }

    virtual const TDesC& Name() = 0;
    
    void StartL() 
        {
        TThreadFunction function = CWorker::ThreadFunction; 
        
        User::LeaveIfError( iThread.Create( Name(), function, 2048, 0, this ) ); 
         
        iActive = ETrue;
        iThread.Resume(); 
        // Go thread go
    }
    
    void Finish() 
        {
        iActive = EFalse;
        DoCancel(); 
        iSemaphore.Wait(); 
        iThread.Close(); 
        }
    
protected: 

    /**
     * For doing any required preparations in the original thread. 
     */
    virtual void DoPrepareL() = 0;
    virtual void DoCancel() = 0;
    virtual void DoRunL() = 0;

    void RunL() 
        {
        CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
        CleanupStack::PushL(scheduler);
        CActiveScheduler::Install(scheduler);

        TRAP(iError, DoRunL());
        iSemaphore.Signal(); 
        
        CleanupStack::PopAndDestroy(scheduler);
        }
    
public: 
    
    static TInt ThreadFunction(void *aThis) 
        {
        CTrapCleanup* cleanup = CTrapCleanup::New();
    
        TRAP_IGNORE
            ( 
            (reinterpret_cast<CWorker*>(aThis))->RunL();
            ); 

        delete cleanup;
        return KErrNone;
        }
    
private: 
    
    TInt iWorkerId;     
    TInt iError; 
    static TInt iNextFreeWorkerId;  
    TBool iActive; 
    RThread iThread; 
    RSemaphore iSemaphore; 
    };


TInt CWorker::iNextFreeWorkerId = 0;  

/**
 * Returns random numbers so, that small numbers
 * are most common. Used to approximate the distribution
 * of real world words. In the distribution the probability that n is 
 * one of the numbers following i is 9 times as probable as that n is i. 
 * 
 * NOTE: Doesn't really follow the real distribution of words, because
 *       the word distribution follows Zipf's law and is not logarithmic.
 *       These things won't probably matter too much in any case. 
 */
TInt LogRand( TInt aDeviation ) 
    {
    TInt rv = 0; 
    while ( Math::Random() % aDeviation ) rv++;
    return rv; 
    }

/**
 * Worker, which continuously adds items to the index. Operates on its own
 * thread. 
 */
class CIndexingWorker : public CWorker
    {
public:
    CIndexingWorker() 
    :   iStop( EFalse ),
        iNextFreeUid( 0 ),
        iQualAppClass( 0 ),
        iAppClass( 0 ) 
        {
        }
    
    static CIndexingWorker* NewL( const TDesC& aAppClass ) 
        {
        CIndexingWorker* self = new ( ELeave ) CIndexingWorker(); 
        CleanupStack::PushL( self ); 
        self->ConstructL( aAppClass ); 
        CleanupStack::Pop( self ); 
        return self; 
        }
     
    void ConstructL( const TDesC& aQualAppClass ) 
        {
        CWorker::ConstructL(); 
        iQualAppClass = aQualAppClass.AllocL();
        iAppClass = aQualAppClass.Right( aQualAppClass.Length() - aQualAppClass.Find( _L( ":" ) ) - 1 ).AllocL();
        
        iName = HBufC::NewL( 512 );
        iName->Des().Append( _L( "indexer") ); 
        iName->Des().AppendNum( WorkerId() ); 
        iName->Des().Append( _L( "_") ); 
        iName->Des().Append( *iAppClass ); 
        }

    ~CIndexingWorker() 
        {
        delete iName; 
        delete iAppClass;
        delete iQualAppClass; 
        }
    
public: // From Cworker

    virtual void DoPrepareL() {
        iStop = EFalse; 
    }

    virtual void DoCancel() {
        iStop = ETrue; 
    }
    
    virtual void DoRunL() 
        {
        RSearchServerSession session;
        User::LeaveIfError( session.Connect() );
        CleanupClosePushL( session ); 
        CCPixIndexer* indexer = CCPixIndexer::NewLC( session );
        indexer->OpenDatabaseL( *iQualAppClass );
        
        HBufC* buf = HBufC::NewLC( 1024*8 ); 
        
        while ( !iStop ) 
            {
            TBuf<64> docuid; 
            docuid.Append( _L("iw") ); 
            docuid.AppendNum( WorkerId() ); 
            docuid.Append( _L("_d") );  
            docuid.AppendNum( iNextFreeUid++ ); 

            CSearchDocument* doc = CSearchDocument::NewLC( docuid,
                                                           *iAppClass );
            
            buf->Des().Zero(); 
            TInt n = LogRand( KTermCountDeviation ) + 1; 
            for ( TInt i = 0; i < n; i++ ) 
                {
                buf->Des().AppendNum( LogRand( KTermDeviation ) ); 
                if ( i + 1 < n ) 
                    {
                    buf->Des().Append( _L( " " ) );
                    }
                }
                
            doc->AddFieldL( KContentsField, *buf, CDocumentField::EStoreYes | 
                                                  CDocumentField::EIndexTokenized | 
                                                  CDocumentField::EAggregateNo ); 
            doc->AddExcerptL( *buf ); 
            
            indexer->AddL( *doc ); 
            
            CleanupStack::PopAndDestroy( doc ); 
            }
        CleanupStack::PopAndDestroy( buf ); 
        CleanupStack::PopAndDestroy( indexer ); 
        CleanupStack::PopAndDestroy(); // session 
        }

    virtual const TDesC& Name() {
        return *iName; 
    }
    
private: 
    
    TBool iStop; 
    
    TInt iNextFreeUid; 

    HBufC* iQualAppClass;

    HBufC* iAppClass;

    HBufC* iName;

    };


/**
 * A worker that commits normal searches to the given  search domain. 
 * Worker operates within its own thread.  
 */
class CSearchingWorker : public CWorker
    {
public: 
    CSearchingWorker() 
    :   iStop( EFalse ),
        iAppClass( 0 ), 
        iName( 0 )
        {
        }
    
    static CSearchingWorker* NewL( const TDesC& aAppClass ) 
        {
        CSearchingWorker* self = new ( ELeave ) CSearchingWorker(); 
        CleanupStack::PushL( self ); 
        self->ConstructL( aAppClass ); 
        CleanupStack::Pop( self ); 
        return self; 
        }

    
    void ConstructL( const TDesC& aAppClass ) 
        {   
        CWorker::ConstructL(); 
        
        iAppClass = aAppClass.AllocL(); 

        iName = HBufC::NewL( 512 );
        iName->Des().Append( _L( "searcher") ); 
        iName->Des().AppendNum( WorkerId() ); 
/*      iName->Des().Append( _L( "_") ); 
        iName->Des().Append( aAppClass ); */
        }
    
    ~CSearchingWorker() 
        {
        delete iName;
        delete iAppClass; 
        }

public: // From Cworker

    virtual void DoPrepareL() 
        {
        iStop = EFalse; 
        }

    virtual void DoCancel() 
        {
        iStop = ETrue; 
        }
    
    virtual void DoRunL() 
        {
        RSearchServerSession session;
        User::LeaveIfError( session.Connect() );
        CleanupClosePushL( session ); 
        CCPixSearcher* searcher = CCPixSearcher::NewLC( session, KContentsField );
        
        searcher->OpenDatabaseL( *iAppClass );
        
        while ( !iStop ) 
            {
            TBuf<64> num; 
            num.AppendNum( LogRand( KTermDeviation ) );  
            TInt n = searcher->SearchL( num ); 
            for ( TInt i = 0; i < n; i++ ) 
                {
                TRAP_IGNORE( delete searcher->GetDocumentL( i++ ); ); 
                }
            }
        CleanupStack::PopAndDestroy( searcher ); // session
        CleanupStack::PopAndDestroy(); // session
        }
        
    virtual const TDesC& Name() 
        {
        return *iName; 
        }
    
private:
    
    TBool iStop; 
    HBufC* iAppClass; 
    HBufC* iName; 
    };


void CTestMultiThreading::setUp() 
    {
    }

void CTestMultiThreading::tearDown() 
    {
    }

void CTestMultiThreading::PrepareL() 
    {
    RSearchServerSession session;
    User::LeaveIfError( session.Connect() ); 
    CleanupClosePushL( session ); 
    
    // First database has one reader and one or two searchers, what 
    // is something that can be expected in the actual device
    User::LeaveIfError
        (
        session.DefineVolume( _L( "@df:root test expected" ), 
                              _L( "c:\\data\\indexing\\indexdb\\root\\test\\expected" ) )
        ); 
    
    // Second database has one reader and one or two searchers, what 
    // is quite uncommon example and mainly used to stress the search server
    User::LeaveIfError
        (
        session.DefineVolume( _L( "@df:root test busy" ),
                              _L( "c:\\data\\indexing\\indexdb\\root\\test\\busy" ) )
        );
        
    CCPixIndexer* indexer = CCPixIndexer::NewLC( session );
    indexer->OpenDatabaseL( _L( "@df:root test expected" ) );
    indexer->ResetL(); 
    indexer->OpenDatabaseL( _L( "@df:root test busy" ) );
    indexer->ResetL(); 
    CleanupStack::PopAndDestroy( indexer); 
    CleanupStack::PopAndDestroy(); // session  

    // 
    // Indexers
    // 
    
    // Indexer with one's own index
    iWorkers->Append( CIndexingWorker::NewL( _L( "@df:root test expected" ) ) );
    
    // 3 indexers in the same index
    iWorkers->Append( CIndexingWorker::NewL( _L( "@df:root test busy" ) ) );
    iWorkers->Append( CIndexingWorker::NewL( _L( "@df:root test busy" ) ) );
    iWorkers->Append( CIndexingWorker::NewL( _L( "@df:root test busy" ) ) );
    
    // = 4 indexers in total 
    
    // 
    // Searchers
    // 
    
    // Multisearcher
    iWorkers->Append( CSearchingWorker::NewL( _L( "root test" ) ) );

    // Search qualified index
    iWorkers->Append( CSearchingWorker::NewL( _L( "@df:root test expected") ) );

    // Search big the busy index
    iWorkers->Append( CSearchingWorker::NewL( _L( "@df:root test busy" ) ) );
    iWorkers->Append( CSearchingWorker::NewL( _L( "@df:root test busy" ) ) );
    iWorkers->Append( CSearchingWorker::NewL( _L( "root test busy" ) ) );
    
    // = 5 searchers in total
    }

void CTestMultiThreading::Wait(TInt aSeconds) 
    {
    RTimer timer; 
    TRequestStatus timerStatus; 
    timer.CreateLocal();        
    CleanupClosePushL( timer ); 
    
    timer.After( timerStatus, aSeconds * 1000 * 1000 ); 
    User::WaitForRequest(timerStatus); 
    
    CleanupStack::PopAndDestroy(); // timer 
    }

void CTestMultiThreading::RunL() 
    {
    for ( TInt i = 0; i < iWorkers->Count(); i++ ) 
        {
        (*iWorkers)[i]->StartL(); 
        }
    
    Wait( 5 ); // Wait for 5 seconds
    
    for ( TInt i = 0; i < iWorkers->Count(); i++ ) 
        {
        (*iWorkers)[i]->Finish(); 
        if ( (*iWorkers)[i]->Error() != KErrNone ) 
            {
            TBuf8<512> msg; 
            const TDesC& name = (*iWorkers)[i]->Name();
            for ( TInt c = 0; c < name.Length(); c++ ) {
                msg.Append( name[c] ); 
            }
            msg.Append( _L8( " failed with" ) );
            msg.AppendNum( (*iWorkers)[i]->Error() );
            //TS_FAIL( reinterpret_cast<const char*>( msg.PtrZ() ) ); 
            }
        }
    }

void CTestMultiThreading::Finish() 
    {
    iWorkers->ResetAndDestroy();
    }

void CTestMultiThreading::DoTestMultiThreadingL() 
    {
    iWorkers = new ( ELeave ) RPointerArray<CWorker>(); 
    CleanupStack::PushL( iWorkers ); 
    CleanupClosePushL( *iWorkers ); 
    PrepareL(); 
    TRAPD( err, RunL() ); 
    Finish(); 
    User::LeaveIfError( err ); 
    CleanupStack::PopAndDestroy( iWorkers ); 
    CleanupStack::PopAndDestroy(); // iWorkers  
    }
