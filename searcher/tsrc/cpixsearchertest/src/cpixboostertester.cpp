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
#include "cpixboostertester.h"
#include "ccpixsearcher.h"
#include "CSearchDocument.h"
#include "CCPixIndexer.h"
#include "indevicecfg.h"

_LIT( KTestBaseAppClass, "test" );
_LIT( KTestQualifiedBaseAppClass, "@c:test" );
_LIT( KContentFieldName, "content" ); 

void CTestBoost::setUp()
    {
    User::LeaveIfError(iSession.Connect());
    iSession.DefineVolume(KTestQualifiedBaseAppClass, KTestBaseAppClass );

    iIndexer = CCPixIndexer::NewL(iSession);
    iIndexer->OpenDatabaseL(KTestQualifiedBaseAppClass);

    iSearcher = CCPixSearcher::NewL(iSession, _L( "" CONTENTS_FIELD ) );
    iSearcher->OpenDatabaseL(KTestBaseAppClass);
    }

void CTestBoost::tearDown()
    {
    delete iIndexer;
    delete iSearcher;
    iSession.Close();
    }

void CTestBoost::testNoBoost()
    {
    // 
    // This is the 'control' test. It is used to assert that 
    // without tinkering with boosts, the engine would return 
    // the result with most 'hits' 
    //
    iIndexer->ResetL(); 

    AddDocumentL( 0, _L( "hit hit hit" ), 1.0f );  // first result
    AddDocumentL( 1, _L( "hit hit miss" ), 1.0f ); // second result
    AddDocumentL( 2, _L( "hit miss miss" ), 1.0f );// last result

    if( iSearcher->SearchL( _L( "hit" ), KContentFieldName ) != 3 )
        User::Leave( KErrNotFound );
    
    AssertHitDocumentL( 0, 0 );  // id 1 is first hit
    AssertHitDocumentL( 1, 1 );  // id 2 is second hit
    AssertHitDocumentL( 2, 2 );  // id 3 is third hit
    }


void CTestBoost::testBoost() 
    {
    //
    // This is the actual boost test. It is used to show that 
    // changing boost value actually affects the rank. The 'normal'
    // document order is reversed with heavy boost usage. 
    //
    iIndexer->ResetL(); 
    
    AddDocumentL( 0, _L( "hit hit hit" ), 1.0f );    // last result
    AddDocumentL( 1, _L( "hit hit miss" ), 10.0f );  // second result
    AddDocumentL( 2, _L( "hit miss miss" ), 100.0f );// first result
    
    if( iSearcher->SearchL( _L( "hit" ), KContentFieldName ) != 3 )
        User::Leave( KErrNotFound );
    
    AssertHitDocumentL( 0, 2 ); // id 1 is third hit
    AssertHitDocumentL( 1, 1 ); // id 2 is second hit
    AssertHitDocumentL( 2, 0 ); // id 3 is first hit
    }

void CTestBoost::testFieldBoost() 
    {
    //
    // This is the actual boost test. It is used to show that 
    // changing boost value actually affects the rank. The 'normal'
    // document order is reversed with heavy boost usage. 
    //
    iIndexer->ResetL(); 
    
    AddDocumentWithFieldBoostL( 0, _L( "hit hit hit" ), 1.0f );      // last result
    AddDocumentWithFieldBoostL( 1, _L( "hit hit miss" ), 10.0f );  // second result
    AddDocumentWithFieldBoostL( 2, _L( "hit miss miss" ), 100.0f );// first result
    
    if( iSearcher->SearchL( _L( "hit" ), KContentFieldName ) != 3 )
        User::Leave( KErrNotFound );
    
    AssertHitDocumentL( 0, 2 ); // id 1 is third hit
    AssertHitDocumentL( 1, 1 ); // id 2 is second hit
    AssertHitDocumentL( 2, 0 ); // id 3 is first hit
    }

void CTestBoost::AssertHitDocumentL( TInt aIndex, TInt aId )
    {
    TBuf<32> buf;
    TInt err = KErrNone;
    buf.AppendNum( aId );
    CSearchDocument* doc = iSearcher->GetDocumentL( aIndex ); 
    CleanupStack::PushL( doc ); 
    if( doc->Id().Compare( buf ) )
        err = KErrNotFound;
    CleanupStack::PopAndDestroy( doc );
    User::LeaveIfError( err );
    }

void CTestBoost::AddDocumentL( TInt aId, const TDesC& aContent, TReal aBoost ) 
    {
    TBuf<32> buf; 
    buf.AppendNum( aId );
    
    CSearchDocument* doc = CSearchDocument::NewLC( buf, KTestBaseAppClass, aContent ); 
    
    doc->SetBoost( aBoost ); 
    doc->AddFieldL( KContentFieldName, aContent, CDocumentField::EStoreNo | CDocumentField::EIndexTokenized | CDocumentField::EAggregateYes );
    
    iIndexer->AddL( *doc ); 
    iIndexer->FlushL();
    
    CleanupStack::PopAndDestroy( doc ); 
    }

void CTestBoost::AddDocumentWithFieldBoostL( TInt aId, const TDesC& aContent, TReal aBoost ) 
    {
    TBuf<32> buf; 
    buf.AppendNum( aId );
    
    CSearchDocument* doc = CSearchDocument::NewLC( buf, KTestBaseAppClass, aContent ); 
    
    doc->AddFieldL( KContentFieldName, 
                    aContent, 
                    CDocumentField::EStoreNo | CDocumentField::EIndexTokenized | CDocumentField::EAggregateYes )
        .SetBoost( aBoost ); // Boost field instead of document
    
    iIndexer->AddL( *doc );
    iIndexer->FlushL();
    
    CleanupStack::PopAndDestroy( doc ); 
    }
