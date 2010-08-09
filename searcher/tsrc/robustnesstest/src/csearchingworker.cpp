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

#include "CSearchingWorker.h"

#include "CSearchDocument.h"
#include "CCPixSearcher.h"

#include <e32math.h>

CSearchingWorker::CSearchingWorker( TInt aSleep, TBool aCancelSearch, TBool aCancelDocumentFetching ) 
:   iSearches( 0 ),
    iTermSearches( 0 ),
    iSearchesMicroSeconds( 0 ),
    iTermSearchesMicroSeconds(),
    iSearchesPeakMicroSeconds( 0 ),
    iTermSearchesPeakMicroSeconds(),
    
    iDocs( 0 ), 
    iDocsMicroSeconds( 0 ), 
    iDocsPeakMicroSeconds( 0 ),
    
    iTerms( 0 ),
    iTermsMicroSeconds( 0 ), 
    iTermsPeakMicroSeconds( 0 ),
    
    iStop( EFalse ),
    iSleep( aSleep ),
    iCancelSearch( aCancelSearch ),
    iCancelDocumentFetching( aCancelDocumentFetching ),
    iAppClass( 0 ), 
	iName( 0 )
	{
	}

CSearchingWorker* CSearchingWorker::NewL( 
		const TDesC& aAppClass,
		TInt aSleep,
		TBool aCancelSearch,
		TBool aCancelDocumentFetching ) 
	{
	CSearchingWorker* self = new ( ELeave ) CSearchingWorker( aSleep, aCancelSearch, aCancelDocumentFetching ); 
	CleanupStack::PushL( self ); 
	self->ConstructL( aAppClass ); 
	CleanupStack::Pop( self ); 
	return self; 
	}


void CSearchingWorker::ConstructL( const TDesC& aAppClass ) 
	{	
	CWorker::ConstructL(); 
	
	iAppClass = aAppClass.AllocL(); 

	iName = HBufC::NewL( 512 );
	iName->Des().Append( _L( "searcher") ); 
	iName->Des().AppendNum( WorkerId() ); 
	iName->Des().Append( _L( " ") ); 
	iName->Des().Append( aAppClass );
	// Replace problematic characters
	for ( TInt i = 0; i < iName->Length(); i++ ) 			
		{
		if ( (*iName)[i] == '@' || (*iName)[i] == ':' )
			{
			iName->Des()[i] = ' ';
			}
		}
	}

CSearchingWorker::~CSearchingWorker() 
	{
	delete iName;
	delete iAppClass; 
	}

void CSearchingWorker::DoPrepareL() 
	{
	iStop = EFalse; 
	}

void CSearchingWorker::DoCancel() 
	{
	iStop = ETrue; 
	}

void CSearchingWorker::DoRunL()
	{
	RSearchServerSession session;
	User::LeaveIfError(session.Connect());
	CleanupClosePushL(session);
	CCPixSearcher* searcher = CCPixSearcher::NewLC(session,
			KContentsField);

	searcher->OpenDatabaseL(*iAppClass);

	while (!iStop)
		{
		TBuf<128> query;
		// Pick randomly either normal search or term search
		TInt op = Math::Random() % 2;
		switch (op)
			{
			case 0:
				// Normal query
				query.AppendNum(LogRand(KTermDeviation));
				iSearches++;
				break;
			case 1:
				// Terms query
				query.Append(_L("$terms<10>("));
				query.AppendNum(LogRand(KPrefixDeviation));
				query.Append(_L("*)"));
				iTermSearches++;
				break;
			}

		TTime before;
		before.UniversalTime();

		TInt n = 0;
		if ( !iCancelSearch )
			{
			n = searcher->SearchL( query );
			}
		else
			{
			searcher->Cancel();
			searcher->SearchL( *this, query);
			}
			
		TTime after;
		after.UniversalTime();
		TInt64 time = after.MicroSecondsFrom(before).Int64();

		switch (op)
			{
			case 0:
				iSearchesMicroSeconds
						+= after.MicroSecondsFrom(before).Int64();
				iSearchesPeakMicroSeconds 
						= Max( iSearchesPeakMicroSeconds, 
							   time );  
				break;
			case 1:
				iTermSearchesMicroSeconds += after.MicroSecondsFrom(
						before).Int64();
				iTermSearchesPeakMicroSeconds 
						= Max( iTermSearchesPeakMicroSeconds, 
								time );  
				break;
			}

		if ( !iCancelSearch )
			{
			for ( TInt i = 0; i < n; i++ )
				{
				// Randomly reset GetDocumentL
				if ( iCancelDocumentFetching && Math::Random() % 2 )
					{
					TRAP_IGNORE( searcher->GetDocumentL( i++, *this ) ); 
					searcher->Cancel();
					}
				else
					{
					TTime before; 
					before.UniversalTime();
					
					TRAP_IGNORE( delete searcher->GetDocumentL( i++ ); );
					
					TTime after;
					after.UniversalTime();
					
					TInt64 time = after.MicroSecondsFrom(before).Int64();
					switch (op) 
						{
						case 0:
							iDocs ++; 
							iDocsMicroSeconds += time; 
							iDocsPeakMicroSeconds = Max(iDocsPeakMicroSeconds, time);
							break;
						case 1:
							iTerms ++; 
							iTermsMicroSeconds += time; 
							iTermsPeakMicroSeconds = Max(iDocsPeakMicroSeconds, time);
							break;
						}
					
					}
				}
			}

		Unlock();
		SleepRandom(iSleep);
		Lock();

		if ( iCancelSearch && Math::Random()%2 )
			{
			searcher->Cancel();
			}
		}
	CleanupStack::PopAndDestroy(searcher); // session
	CleanupStack::PopAndDestroy(); // session
	}
	
const TDesC& CSearchingWorker::Name() 
	{
	return *iName; 
	}

void CSearchingWorker::HandleSearchResultsL(TInt /*aError*/, TInt /*aEstimatedResultCount*/)
	{
	// TODO: Fetch documents and randomly cancel
	}

void CSearchingWorker::HandleDocumentL(TInt /*aError*/, CSearchDocument* /*aDocument*/)
	{
	}
void CSearchingWorker::HandleBatchDocumentL(TInt aError, TInt aReturnCount, CSearchDocument** aDocument)
    {
    }

TInt CSearchingWorker::ConsumeSearches() 
	{
	TInt ret = iSearches; 
	iSearches = 0; 
	return ret; 
	}

TInt CSearchingWorker::ConsumeSearchesPeakMicroSeconds() 
	{
	TInt ret = iSearchesPeakMicroSeconds; 
	iSearchesPeakMicroSeconds = 0; 
	return ret; 
	}

TInt CSearchingWorker::ConsumeSearchesMicroSeconds() 
	{
	TInt ret = iSearchesMicroSeconds; 
	iSearchesMicroSeconds = 0; 
	return ret; 
	}

TInt CSearchingWorker::ConsumeTermSearches() 
	{
	TInt ret = iTermSearches; 
	iTermSearches = 0; 
	return ret; 
	}

TInt CSearchingWorker::ConsumeTermSearchesMicroSeconds() 
	{
	TInt ret = iTermSearchesMicroSeconds; 
	iTermSearchesMicroSeconds = 0; 
	return ret; 
	}

TInt CSearchingWorker::ConsumeTermSearchesPeakMicroSeconds() 
	{
	TInt ret = iTermSearchesPeakMicroSeconds; 
	iTermSearchesPeakMicroSeconds = 0; 
	return ret; 
	}

TInt CSearchingWorker::ConsumeDocs() 
	{
	TInt ret = iDocs; 
	iDocs = 0; 
	return ret; 
	}

TInt CSearchingWorker::ConsumeDocsMicroSeconds() 
	{
	TInt ret = iDocsMicroSeconds; 
	iDocsMicroSeconds = 0; 
	return ret; 
	}

TInt CSearchingWorker::ConsumeDocsPeakMicroSeconds() 
	{
	TInt ret = iDocsPeakMicroSeconds; 
	iDocsPeakMicroSeconds = 0; 
	return ret; 
	}

TInt CSearchingWorker::ConsumeTerms() 
	{
	TInt ret = iTerms; 
	iTerms = 0; 
	return ret; 
	}

TInt CSearchingWorker::ConsumeTermsMicroSeconds() 
	{
	TInt ret = iTermsMicroSeconds; 
	iTermsMicroSeconds = 0; 
	return ret; 
	}

TInt CSearchingWorker::ConsumeTermsPeakMicroSeconds() 
	{
	TInt ret = iTermsPeakMicroSeconds; 
	iTermsPeakMicroSeconds = 0; 
	return ret; 
	}
