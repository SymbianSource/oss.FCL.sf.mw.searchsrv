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
	SetActive();

	}

EXPORT_C CSearchDocument* CCPixSearcher::GetDocumentL(TInt aIndex)
    {
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
    PERFORMANCE_LOG_START("CCPixSearcher::GetDocumentL");
    
    if ( !iIsDatabaseOpen ) 	User::Leave( KErrNotReady );
	if ( IsActive() )
		{
		User::Leave(KErrInUse); // Need ::RunError to handle this
		}
	
	iObserver.iNextDocument = &aObserver;
	
	iState = EStateGetDocument;
	iSubSession.GetDocument(aIndex, iStatus);
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
