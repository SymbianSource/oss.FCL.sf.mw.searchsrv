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

// INCLUDE FILES
#include <e32svr.h>
#include "common.h"

#include "SearchClient.pan"
#include "CCPixSearcher.h"
#include "MCPixSearcherObserver.h"
#include "SearchServerCommon.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "ccpixsearcherTraces.h"
#endif


// CCPixSearcher::NewL()
// Two-phased constructor.
EXPORT_C CCPixSearcher* CCPixSearcher::NewL(RSearchServerSession& aSearchServerSession, const TDesC& aDefaultSearchField)
	{
	CCPixSearcher* self = NewLC(aSearchServerSession, aDefaultSearchField);
	CleanupStack::Pop( self );
	return self;
	}

// CCPixSearcher::NewLC()
// Two-phased constructor.
EXPORT_C CCPixSearcher* CCPixSearcher::NewLC(RSearchServerSession& aSearchServerSession, const TDesC& aDefaultSearchField)
	{
	CCPixSearcher* self = new ( ELeave ) CCPixSearcher(aSearchServerSession);
	CleanupStack::PushL( self );
	self->ConstructL(aDefaultSearchField);
	return self;
	}

// CCPixSearcher::ConstructL()
// Symbian 2nd phase constructor can leave.
void CCPixSearcher::ConstructL(const TDesC& aDefaultSearchField)
	{
	User::LeaveIfError( iSubSession.Open(iSearchServerSession) );
	iDefaultSearchField = aDefaultSearchField.AllocL(); 
	}

// CCPixSearcher::CCPixSearcher()
// C++ default constructor can NOT contain any code, that might leave.
CCPixSearcher::CCPixSearcher(RSearchServerSession& aSearchServerSession)
	: CActive( EPriorityStandard ),
	  iState( EStateNone ),
	  iIsDatabaseOpen( EFalse ),
	  iSearchServerSession( aSearchServerSession ),
	  iBaseAppClass( 0 ) 
	{
	CActiveScheduler::Add( this );
	}

// CCPixSearcher::~CCPixSearcher()
// Destructor.
EXPORT_C CCPixSearcher::~CCPixSearcher()
	{	
	Cancel(); // Causes call to DoCancel()
	iSubSession.Close();
	
	delete iDefaultSearchField;
	delete iQueryString;
	delete iBaseAppClass;
	delete iAnalyzer;
	}

EXPORT_C TBool CCPixSearcher::IsDatabaseOpen() const
	{
	return iIsDatabaseOpen; 
	}

EXPORT_C const TDesC& CCPixSearcher::GetBaseAppClass() const
{
	if (iBaseAppClass)
		return *iBaseAppClass;
	else
		return KNullDesC;
}

//
//
EXPORT_C void CCPixSearcher::OpenDatabaseL(const TDesC& aBaseAppClass)
	{
	if ( IsActive() )
		{
		User::Leave(KErrInUse);
		}

	iIsDatabaseOpen = EFalse; // If opening database fails, this is the safe assumption
	if( iBaseAppClass )
	    {
	    delete iBaseAppClass;
	    iBaseAppClass = NULL;
	    }
	iBaseAppClass = aBaseAppClass.AllocL();
	iSubSession.OpenDatabaseL(ETrue, *iBaseAppClass, *iDefaultSearchField);
	iIsDatabaseOpen = ETrue; 
	}

//
//
EXPORT_C void CCPixSearcher::OpenDatabaseL(MCPixOpenDatabaseRequestObserver& aObserver, const TDesC& aBaseAppClass)
	{
	if ( IsActive() )
		{
		User::Leave( KErrInUse);
		}

	iIsDatabaseOpen = EFalse; // If opening database fails, this is the safe assumption
	iObserver.iOpenDatabase = &aObserver;
	
	iState = EStateOpenDatabase;
	if(iBaseAppClass)
	    {
	    delete iBaseAppClass;
	    iBaseAppClass = NULL;
	    }
	iBaseAppClass = aBaseAppClass.AllocL();
	iSubSession.OpenDatabase(ETrue, *iBaseAppClass, *iDefaultSearchField, iStatus); // Create if not found
	SetActive(); 
	}

EXPORT_C void CCPixSearcher::SetAnalyzerL( const TDesC& aAnalyzer )
	{
	if ( !iIsDatabaseOpen ) 	User::Leave(KErrNotReady);
	if ( IsActive() ) 			User::Leave(KErrInUse);

	iSubSession.SetAnalyzerL( aAnalyzer ); 
	}

EXPORT_C void CCPixSearcher::SetAnalyzerL( MCPixSetAnalyzerRequestObserver& aObserver, const TDesC& aAnalyzer )
	{
	if ( !iIsDatabaseOpen ) 	User::Leave(KErrNotReady);
	if ( IsActive() ) 			User::Leave(KErrInUse);
	
	iAnalyzer = aAnalyzer.AllocL(); 

	iObserver.iSetAnalyzer = &aObserver;
	iState = EStateSetAnalyzer; 
	iSubSession.SetAnalyzer( aAnalyzer, iStatus );
	SetActive(); 
	}
	
	EXPORT_C void CCPixSearcher::SetQueryParserL( TQueryParser aQueryParser ) 
	{
	if ( !iIsDatabaseOpen ) 	User::Leave(KErrNotReady);
	if ( IsActive() ) 			User::Leave(KErrInUse);
	
	iSubSession.SetQueryParserL( aQueryParser ); 
	}

EXPORT_C void CCPixSearcher::SetQueryParserL( MCPixSetQueryParserRequestObserver& aObserver, TQueryParser aQueryParser ) 
	{
	if ( !iIsDatabaseOpen ) 	User::Leave(KErrNotReady);
	if ( IsActive() ) 			User::Leave(KErrInUse);
	
	iObserver.iSetQueryParser = &aObserver;
	iState = EStateSetQueryParser; 
	iSubSession.SetQueryParser( aQueryParser, iStatus );
	SetActive(); 
	}


// CCPixSearcher::FormQueryString()
// Suport method for SearchL-methods
HBufC* CCPixSearcher::FormQueryStringL(const TDesC& aQueryString, const TDesC& aDocumentField)
	{
	// Search with the specified field
	if ( aDocumentField != KNullDesC )
		{
		HBufC* queryString = HBufC::NewL(aQueryString.Length() + aDocumentField.Length() + 1);
		TPtr searchTermsPtr = queryString->Des();
		searchTermsPtr.Append(aDocumentField);
		searchTermsPtr.Append(':');
		searchTermsPtr.Append(aQueryString);
		return queryString; 
		}
	// Search with the default field
	else 
		{
		// Allocate search terms
		return aQueryString.AllocL();	
		}
	}

// CCPixSearcher::Search()
// Issues a new search
EXPORT_C TInt CCPixSearcher::SearchL(const TDesC& aQueryString, const TDesC& aDocumentField)
	{
	OstTraceFunctionEntry0( CCPIXSEARCHER_SEARCHL_ENTRY );
	PERFORMANCE_LOG_START("CCPixSearcher::SearchL");
	
	if ( !iIsDatabaseOpen ) 	
		User::Leave( KErrNotReady );
	if ( IsActive() )
		{
		User::Leave(KErrInUse);
		}
	
	HBufC* query = FormQueryStringL( aQueryString, aDocumentField ); 
	CleanupStack::PushL( query ); 
	
	iSubSession.SearchL( *query );
	
	CleanupStack::PopAndDestroy( query ); 
	
	return iSubSession.GetEstimatedDocumentCount();
	}

EXPORT_C void CCPixSearcher::SearchL(MCPixSearchRequestObserver& aObserver, const TDesC& aQueryString, const TDesC& aDocumentField)
	{
    OstTraceFunctionEntry0( DUP1_CCPIXSEARCHER_SEARCHL_ENTRY );
    PERFORMANCE_LOG_START("CCPixSearcher::SearchL");

	if ( !iIsDatabaseOpen ) 	User::Leave( KErrNotReady );
	if ( IsActive() )
		{
		User::Leave(KErrInUse);
		}

	delete iQueryString;
	iQueryString = NULL;
	iQueryString = FormQueryStringL( aQueryString, aDocumentField ); 

	iObserver.iSearch = &aObserver;
	iState = EStateSearch;
	iSubSession.Search( *iQueryString, iStatus );
	OstTraceFunctionExit0( CCPIXSEARCHER_SEARCHL_EXIT );
	SetActive();	
	}

EXPORT_C CSearchDocument* CCPixSearcher::GetDocumentL(TInt aIndex)
    {
    OstTraceFunctionEntry0( CCPIXSEARCHER_GETDOCUMENTL_ENTRY );
    PERFORMANCE_LOG_START("CCPixSearcher::GetDocumentL");
    
	if ( !iIsDatabaseOpen ) 	User::Leave( KErrNotReady );
	if ( IsActive() )
		{
		User::Leave(KErrInUse);
		}

	return iSubSession.GetDocumentL(aIndex);
    }

EXPORT_C void CCPixSearcher::GetDocumentL(TInt aIndex, MCPixNextDocumentRequestObserver& aObserver)
    {
    OstTraceFunctionEntry0( DUP1_CCPIXSEARCHER_GETDOCUMENTL_ENTRY );
    PERFORMANCE_LOG_START("CCPixSearcher::GetDocumentL");
    
    if ( !iIsDatabaseOpen ) 	User::Leave( KErrNotReady );
	if ( IsActive() )
		{
		User::Leave(KErrInUse); // Need ::RunError to handle this
		}
	
	iObserver.iNextDocument = &aObserver;
	
	iState = EStateGetDocument;
	iSubSession.GetDocument(aIndex, iStatus);
	OstTraceFunctionExit0( CCPIXSEARCHER_GETDOCUMENTL_EXIT );
	SetActive();     
    }

EXPORT_C CSearchDocument** CCPixSearcher::GetBatchDocumentL(TInt aIndex, TInt& aReturnDoc, TInt aCount)
    {
    PERFORMANCE_LOG_START("CCPixSearcher::GetBatchDocumentL");
    
	if ( !iIsDatabaseOpen ) 	User::Leave( KErrNotReady );
	if ( IsActive() )
		{
		User::Leave(KErrInUse);
		}

	return iSubSession.GetBatchDocumentL(aIndex,aReturnDoc, aCount);
    }

EXPORT_C void CCPixSearcher::GetBatchDocumentL(TInt aIndex, MCPixNextDocumentRequestObserver& aObserver, TInt aCount)
    {
    PERFORMANCE_LOG_START("CCPixSearcher::GetBatchDocumentL");
    
    if ( !iIsDatabaseOpen ) 	User::Leave( KErrNotReady );
	if ( IsActive() )
		{
		User::Leave(KErrInUse); // Need ::RunError to handle this
		}
	
	iObserver.iNextDocument = &aObserver;
	
	iState = EStateGetBatchDocument;
	iSubSession.GetBatchDocument(aIndex, iStatus, aCount);
	SetActive();     
    }
	
	
// CCPixSearcher::RunL()
// Invoked to handle responses from the server.
void CCPixSearcher::RunL()
	{
	// NOTE: Should we do cleanup here (!)
	
	// Observer might be replaced during observer call
	TObserver observer = iObserver; 
	iObserver.iAny = NULL; 
	
	TState oldState = iState; 
	iState = EStateNone;
	
	switch ( oldState )
		{
		case EStateOpenDatabase:
			iIsDatabaseOpen = (iStatus.Int() == KErrNone); // Inform the observer about the database
			if ( observer.iOpenDatabase )
				observer.iOpenDatabase->HandleOpenDatabaseResultL( iStatus.Int() );
			break;

		case EStateSetAnalyzer:
			delete iAnalyzer; iAnalyzer = NULL; // cleanup

			if ( observer.iSetAnalyzer ) {
				observer.iSetAnalyzer->HandleSetAnalyzerResultL( iStatus.Int() );
			}
			break;

		case EStateSetQueryParser:
			if ( observer.iSetQueryParser ) {
				observer.iSetQueryParser->HandleSetQueryParserResultL( iStatus.Int() );
			}
			break;

		case EStateSearch:
			delete iQueryString; iQueryString = NULL; // cleanup

			// Fetch search results and call back at the observer
			iEstimatedResultsCount = iSubSession.GetEstimatedDocumentCount();
			if ( observer.iSearch )
				observer.iSearch->HandleSearchResultsL(iStatus.Int(), iEstimatedResultsCount);
			break;

		case EStateGetDocument:
		    {
		    // Fetch search results and call back at the observer
		    CSearchDocument* document = 0;
		    TRAPD( err, document = iSubSession.GetDocumentObjectL() );
		    if ( observer.iNextDocument )
		        {
		        if ( err == KErrNone )
		            {
		            observer.iNextDocument->HandleDocumentL(iStatus.Int(), document);
		            }
		        else
		            {
		            observer.iNextDocument->HandleDocumentL(err, document);
		            }
		        }
		    }
		    break;
			
		case EStateGetBatchDocument:
		    {
			// Fetch search results and call back at the observer
		    CSearchDocument** document = 0;
		    TInt retCount = 0;
		    TRAPD( err, document = iSubSession.GetBatchDocumentObjectL( retCount ) );
		    if ( observer.iNextDocument )
		        {
		        if ( err == KErrNone )
		            {
		            observer.iNextDocument->HandleBatchDocumentL(iStatus.Int(),retCount, document);
		            }
		        else
		            {
		            observer.iNextDocument->HandleBatchDocumentL(err, retCount, document);
		            }
		        }
		    }
			break;
		

		case EStateNone:
		    // Do nothing, dont panic.
		    break;

		default:
		    User::Panic(KSearchClient, ESearchClientBadState);
		    }
	}

// CCPixSearcher::DoCancel()
// Cancels any outstanding operation.
void CCPixSearcher::DoCancel()
	{
	iState = EStateNone;
	iSubSession.CancelAll();
	}

// CCPixSearcher::RunError()
// Don't propagate error.
TInt CCPixSearcher::RunError(TInt /* aError */ )
	{
	iSubSession.CancelAll();
	return KErrNone;
	}

// End of File
