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

#include "CIndexingWorker.h"
#include "CSearchDocument.h"
#include "CCPixIndexer.h"

#include <e32math.h>

CSearchDocument* GenerateDocumentLC( TInt aIndexAverageItems,  
									 const TDesC& aAppClass,
								     TDes& buf) 
	{
	TBuf<64> id;
	TInt deletionId = Math::Random() % aIndexAverageItems; 
	id.AppendNum( deletionId );
	
	CSearchDocument* doc = CSearchDocument::NewLC( id,
		                                           aAppClass );

	buf.Zero(); 
	TInt n = LogRand( KTermCountDeviation ) + 1; 
	for ( TInt i = 0; i < n; i++ ) 
		{
		buf.AppendNum( LogRand( KTermDeviation ) ); 
		if ( i + 1 < n ) 
			{
			buf.Append( _L( " " ) );
			}
		}
		
	doc->AddFieldL( KContentsField, buf, CDocumentField::EStoreYes | 
										  CDocumentField::EIndexTokenized | 
										  CDocumentField::EAggregateNo ); 
	doc->AddExcerptL( buf ); 

	return doc;
	}

//
// CIndexingWorker
//

CIndexingWorker::CIndexingWorker( TBool aDeletesEnabled, TInt aIndexAverageItems, TInt aSleep ) 
:	iStop( EFalse ),
 	iNextFreeUid( 0 ),
	iQualAppClass( 0 ),
	iAppClass( 0 ),
	iDeletesEnabled( aDeletesEnabled ), 
	iIndexAverageItems( aIndexAverageItems ),
	iSleep( aSleep ),
	iAdditions(), 
	iDeletes(), 
	iAdditionsMicroSeconds(), 
	iDeletesMicroSeconds()
	{
	}

CIndexingWorker* CIndexingWorker::NewL( const TDesC& aAppClass, TBool aDeletesEnabled, TInt aIndexAverageItems, TInt aSleep ) 
	{
	CIndexingWorker* self = new ( ELeave ) CIndexingWorker( aDeletesEnabled, aIndexAverageItems, aSleep ); 
	CleanupStack::PushL( self ); 
	self->ConstructL( aAppClass ); 
	CleanupStack::Pop( self ); 
	return self; 
	}
 
void CIndexingWorker::ConstructL( const TDesC& aQualAppClass ) 
	{
	CWorker::ConstructL(); 
	iQualAppClass = aQualAppClass.AllocL();
	iAppClass = aQualAppClass.Right( aQualAppClass.Length() - aQualAppClass.Find( _L( ":" ) ) - 1 ).AllocL();
	
	iName = HBufC::NewL( 512 );
	iName->Des().Append( _L( "indexer" ) ); 
	iName->Des().AppendNum( WorkerId() ); 
	iName->Des().Append( _L( "_") ); 
	iName->Des().Append( *iAppClass ); 
	}

CIndexingWorker::~CIndexingWorker() 
	{
	delete iName; 
	delete iAppClass;
	delete iQualAppClass; 
	}

void CIndexingWorker::DoPrepareL() 
	{
	iStop = EFalse; 
	}

void CIndexingWorker::DoCancel() 
	{
	iStop = ETrue; 
	}

void CIndexingWorker::DoRunL() 
	{
	RSearchServerSession session;
	User::LeaveIfError( session.Connect() );
	CleanupClosePushL( session ); 
	CCPixIndexer* indexer = CCPixIndexer::NewLC( session );
	indexer->OpenDatabaseL( *iQualAppClass );
	
	HBufC* buf = HBufC::NewLC( 1024*8 ); 
	
	while ( !iStop ) 
		{
		//
		// Following pieces of code will always add one item and after that 
		// it will delete an item at random. The average amount of items
		// released by the deletion part is the amount of documents n divided
		// by constant KAverageItemsPerIndex. This will keep the amount of
		// items around KAverageItemsPerIndex on average. 
		
		//
		// Add item
		TBuf<64> docuid; 
		
		TInt choose;  
		if ( iDeletesEnabled ) 
			{
			choose = Math::Random()%2;
			}
		else 
			{
			choose = 0; // always add 
			}
		
		switch ( choose ) 
			{
			case 0:
				//
				// Add item
				{
				TPtr pbuf( buf->Des() );
				CSearchDocument* doc = 
					GenerateDocumentLC( iIndexAverageItems, 
										*iAppClass, 
										pbuf );
				
				TTime before;
				before.UniversalTime();
				indexer->AddL( *doc ); 
				TTime after; 
				after.UniversalTime();
				iAdditions++; 
				iAdditionsMicroSeconds += after.MicroSecondsFrom(before).Int64(); 
				
				CleanupStack::PopAndDestroy( doc ); 
				}
				break;
			case 1: 
				//
				// Delete item
				{
				docuid.Zero(); 
				GetId( docuid ); 
				TTime before;
				before.UniversalTime();
				indexer->DeleteL( docuid ); 
				TTime after; 
				after.UniversalTime();
				iDeletes++; 
				iDeletesMicroSeconds += after.MicroSecondsFrom(before).Int64(); 
				}
				break;
			}
		Unlock(); 
		SleepRandom( iSleep );
		Lock(); 
		}
	CleanupStack::PopAndDestroy( buf ); 
	CleanupStack::PopAndDestroy( indexer ); 
	CleanupStack::PopAndDestroy(); // session 
	}

const TDesC& CIndexingWorker::Name() {
	return *iName; 
}

void CIndexingWorker::GetId(TDes& aId) 
	{
	TInt deletionId = Math::Random() % iIndexAverageItems; 
	aId.AppendNum( deletionId );
	}

TInt CIndexingWorker::ConsumeAdditions() 
	{
	TInt ret = iAdditions; 
	iAdditions = 0; 
	return ret; 
	}

TInt CIndexingWorker::ConsumeAdditionsMicroSeconds() 
	{
	TInt ret = iAdditionsMicroSeconds; 
	iAdditionsMicroSeconds = 0; 
	return ret; 
	}

TInt CIndexingWorker::ConsumeDeletes() 
	{
	TInt ret = iDeletes; 
	iDeletes = 0; 
	return ret; 
	}

TInt CIndexingWorker::ConsumeDeletesMicroSecond () 
	{
	TInt ret = iDeletesMicroSeconds; 
	iDeletesMicroSeconds = 0; 
	return ret; 
	}
