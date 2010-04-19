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

#include "CCPixSearch.h"
#include "CSearchDocument.h"
#include "common.h"
#include "SearchServerLogger.h"
#include "CCPixAsyncronizer.h"
#include "SearchServerHelper.h"

namespace {

/**
 * cpix_DocFieldEnum destroyer for TCleanupItem
 * @param aCpixDocFieldEnum CPix document
 */
void CpixDocFieldEnumDestroyer(TAny* aCpixDocFieldEnum)
	{
	cpix_DocFieldEnum_destroy( static_cast<cpix_DocFieldEnum*>( aCpixDocFieldEnum ) );
	}

} // namespace

CCPixSearch* CCPixSearch::NewL()
	{
	CCPixSearch* self = CCPixSearch::NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CCPixSearch* CCPixSearch::NewLC()
	{
	CCPixSearch* self = new (ELeave) CCPixSearch();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CCPixSearch::CCPixSearch()
	: iPendingTask(EPendingTaskNone)
	{
	}

CCPixSearch::~CCPixSearch()
	{
	delete iAsyncronizer;
	if ( iHits )
		{
		cpix_Hits_destroy( iHits );
		}

	delete iBaseAppClass;
	delete iDefaultSearchFieldZ;
	
	cpix_Query_destroy(iQuery);
	cpix_QueryParser_destroy(iQueryParser);
	cpix_Analyzer_destroy(iAnalyzer);
	cpix_IdxSearcher_releaseDb(iIdxDb);
	}

void CCPixSearch::ConstructL()
	{
	iAsyncronizer = CCPixAsyncronizer::NewL();
	}

void CCPixSearch::CancelAll(const RMessage2& aMessage)
	{
    if (iPendingTask == EPendingTaskSearch)
	    {
		iAsyncronizer->Cancel();
		cpix_CancelAction cancelAction = 
            cpix_IdxSearcher_cancelSearch(iIdxDb, iPendingJobId);
		LogCancelAction(cancelAction);
		
		cpix_ClearError(iIdxDb);
		aMessage.Complete(KErrNone);
		}
    else if (iPendingTask == EPendingTaskDocument)
    	{
		iAsyncronizer->Cancel();
		cpix_CancelAction cancelAction = 
            cpix_Hits_cancelDoc(iHits, iPendingJobId);
	   	LogCancelAction(cancelAction);
	   	
		cpix_ClearError(iHits);
	   	aMessage.Complete(KErrNone);
    	}
    else if ( iPendingTask != EPendingTaskNone )
        {
        // Do nothing here. Wait until asynchronous functions
        // completes and complete request after that.
        iAsyncronizer->CancelWhenDone(aMessage);
        }
    else 
        {
        // else must complete the message now
        aMessage.Complete(KErrNone);
        }

    iPendingTask = EPendingTaskNone;
	}

void CCPixSearch::LogCancelAction(cpix_CancelAction aCancelAction)
    {
#ifdef CPIX_LOGGING_ENABLED
    CPIXLOGSTRING2("Cancel action code %d", aCancelAction);
#endif // CPIX_LOGGING_ENABLED
    }

void CCPixSearch::CompletionCallback(void *aCookie, cpix_JobId aJobId)
	{
	CCPixSearch* object = (CCPixSearch*)aCookie;

	// Sanity check
	if (object == NULL || 
		object->iPendingJobId != aJobId)
		return;
	
	// Call the asyncronizers completion code
	CCPixAsyncronizer* asyncronizer = object->iAsyncronizer;
	asyncronizer->CompletionCallback();
	}

TBool CCPixSearch::SearchL(const TDesC& aSearchTerms, MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage)
    {
    PERFORMANCE_LOG_START("CCPixSearch::SearchL");
    
    if (iPendingTask != EPendingTaskNone)
        User::Leave(KErrInUse);

    // Destroy previous hits
    iDocumentCount = 0;
    cpix_Hits_destroy( iHits );
    iHits = NULL; 
    // Allocate search terms (allow zero termination)
    HBufC* searchTerms = HBufC::NewLC(aSearchTerms.Length() + 1);
    TPtr searchTermsPtr = searchTerms->Des();
    searchTermsPtr.Copy(aSearchTerms);
    
    // Destroy previous query
    cpix_Query_destroy( iQuery );
    iQuery = NULL;
    
    // Commit the new query
    iQuery = cpix_QueryParser_parse(iQueryParser, reinterpret_cast<const wchar_t*>(searchTermsPtr.PtrZ()) );
    SearchServerHelper::CheckCpixErrorL(iQuery, KErrQueryParsingFailed);
    
    // Cleanup terms
    CleanupStack::PopAndDestroy(searchTerms);
    
    // Commit the search
    if ( iQuery )
	    {
	    iPendingJobId = cpix_IdxSearcher_asyncSearch(iIdxDb, iQuery, (void*)this, &CompletionCallback);
	    SearchServerHelper::CheckCpixErrorL(iIdxDb, KErrDatabaseQueryFailed);
    	iPendingTask = EPendingTaskSearch;
	    iAsyncronizer->Start(ECPixTaskTypeSearch, aObserver, aMessage);
	    
        return ETrue;
        }
    
    // Search was not committed
    return EFalse;
    }

TInt CCPixSearch::SearchCompleteL()
    {
    PERFORMANCE_LOG_START("CCPixSearch::SearchCompleteL");
    // Request is no more pending
    iPendingTask = EPendingTaskNone;
    
    // Get the hits
    iHits = cpix_IdxSearcher_asyncSearchResults(iIdxDb, iPendingJobId);
    SearchServerHelper::CheckCpixErrorL(iIdxDb, KErrDatabaseQueryFailed);
	iDocumentCount = cpix_Hits_length(iHits);
	SearchServerHelper::CheckCpixErrorL(iHits, KErrDatabaseQueryFailed);
	
	return iDocumentCount; 
    }
       
void CCPixSearch::GetDocumentL(TInt aIndex, MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage)
	{
	PERFORMANCE_LOG_START("CCPixSearch::GetDocumentL");
	
    if (iPendingTask != EPendingTaskNone)
        User::Leave(KErrInUse);
		
	if (aIndex<0 || aIndex >= iDocumentCount )
		{
		User::Leave(KErrDocumentAccessFailed);
		}
	
    iPendingTask = EPendingTaskDocument;
    iPendingJobId = cpix_Hits_asyncDoc(iHits, aIndex, &iCurrentCpixDocument, (void*)this, &CompletionCallback);
    if ( cpix_Failed(iHits) )
        {
        SearchServerHelper::LogErrorL(*(iHits->err_));
        cpix_ClearError(iHits);
        User::Leave(KErrDocumentAccessFailed);
        }
    iAsyncronizer->Start(ECPixTaskTypeGetDocument, aObserver, aMessage);
	}

CSearchDocument* CCPixSearch::GetDocumentCompleteL()
	{
	PERFORMANCE_LOG_START("CCPixSearch::GetDocumentCompleteL");
	
    // Request is no more pending
    iPendingTask = EPendingTaskNone;
	
	cpix_Hits_asyncDocResults(iHits, iPendingJobId);
	SearchServerHelper::CheckCpixErrorL(iHits, KErrDocumentAccessFailed);

	const wchar_t* documentId = cpix_Document_getFieldValue(&iCurrentCpixDocument, LCPIX_DOCUID_FIELD);
	SearchServerHelper::CheckCpixErrorL(&iCurrentCpixDocument, KErrDatabaseQueryFailed);
	
	TPtrC documentIdPtr(KNullDesC);
	if (documentId)
		documentIdPtr.Set(reinterpret_cast<const TUint16*>(documentId));

	const wchar_t* documentAppClass = cpix_Document_getFieldValue(&iCurrentCpixDocument, LCPIX_APPCLASS_FIELD);
	SearchServerHelper::CheckCpixErrorL(&iCurrentCpixDocument, KErrDatabaseQueryFailed);
	
	TPtrC documentAppClassPtr(KNullDesC);
	if (documentAppClass)
		documentAppClassPtr.Set(reinterpret_cast<const TUint16*>(documentAppClass));

	const wchar_t* documentExcerpt = cpix_Document_getFieldValue(&iCurrentCpixDocument, LCPIX_EXCERPT_FIELD);
	SearchServerHelper::CheckCpixErrorL(&iCurrentCpixDocument, KErrDatabaseQueryFailed);
	
	TPtrC documentExcerptPtr(KNullDesC);
	if (documentExcerpt)
		documentExcerptPtr.Set(reinterpret_cast<const TUint16*>(documentExcerpt));

	CSearchDocument* document = CSearchDocument::NewLC(documentIdPtr, documentAppClassPtr, documentExcerptPtr);

	cpix_DocFieldEnum* docFieldEnum = cpix_Document_fields(&iCurrentCpixDocument);

	// push to cleanup stack.
	CleanupStack::PushL( TCleanupItem(CpixDocFieldEnumDestroyer, docFieldEnum) );

	SearchServerHelper::CheckCpixErrorL(&iCurrentCpixDocument, KErrDocumentAccessFailed);

	cpix_Field field;

	while (cpix_DocFieldEnum_hasMore(docFieldEnum))
		{
		cpix_DocFieldEnum_next(docFieldEnum, &field);
		SearchServerHelper::CheckCpixErrorL(docFieldEnum, KErrDatabaseQueryFailed);
		
		const wchar_t* name = cpix_Field_name(&field);
		SearchServerHelper::CheckCpixErrorL(&field, KErrDatabaseQueryFailed);

		TPtrC namePtr( reinterpret_cast<const TUint16*>( name ) );
		if (    namePtr == TPtrC( (TUint16*)LCPIX_DOCUID_FIELD )
			 || namePtr == TPtrC( (TUint16*)LCPIX_APPCLASS_FIELD )
			 || namePtr == TPtrC( (TUint16*)LCPIX_EXCERPT_FIELD ) )
			{
			continue;  // These fields have already been added
			}

		const wchar_t* value = cpix_Field_stringValue(&field);
		SearchServerHelper::CheckCpixErrorL(&field, KErrDatabaseQueryFailed);
		
		TPtrC stringvalue( reinterpret_cast<const TUint16*>( value ) );
		document->AddFieldL(namePtr, stringvalue);
		}

	CleanupStack::PopAndDestroy(docFieldEnum);
	
	CleanupStack::Pop(document);
	
	return document;
	}

void CCPixSearch::SetAnalyzerL(const TDesC& aAnalyzer)
	{
	cpix_QueryParser_destroy( iQueryParser ); 
	iQueryParser = NULL; 
	
	cpix_Analyzer_destroy( iAnalyzer ); 
	iAnalyzer = NULL; 
	
	// Create analyzer
	cpix_Result result;
	
	HBufC* analyzer = HBufC::NewLC(aAnalyzer.Length() + 1);
	TPtr analyzerPtr = analyzer->Des();
	analyzerPtr.Copy(aAnalyzer);
	const wchar_t* cAnalyzer = reinterpret_cast<const wchar_t*>(analyzerPtr.PtrZ());

	iAnalyzer = cpix_Analyzer_create(&result, cAnalyzer);
	SearchServerHelper::CheckCpixErrorL(&result, KErrCannotCreateAnalyzer);

	CleanupStack::PopAndDestroy( analyzer ); 

	iQueryParser = 
		cpix_QueryParser_create(&result,
			reinterpret_cast<const wchar_t*>(iDefaultSearchFieldZ->Des().PtrZ()), 
			iAnalyzer);
	SearchServerHelper::CheckCpixErrorL(&result, KErrCannotCreateQueryParser);
	}

void CCPixSearch::OpenDatabaseL(const TDesC& aBaseAppClass, const TDesC& aDefaultSearchField)
	{
	// Release data associated with old database 
	delete iDefaultSearchFieldZ;
	iDefaultSearchFieldZ = NULL;
	delete iBaseAppClass;
	iBaseAppClass = NULL;
	cpix_IdxSearcher_releaseDb( iIdxDb );
	iIdxDb = NULL;
	
	// Allocate default search field (space for zero terminated)
	iDefaultSearchFieldZ = HBufC::NewL(aDefaultSearchField.Size() + 1);
	TPtr defaultSearchFieldPtr = iDefaultSearchFieldZ->Des();
	defaultSearchFieldPtr.Copy(aDefaultSearchField);
	
	// Allocate base app class (space for zero terminated)
	iBaseAppClass = HBufC8::NewL(aBaseAppClass.Size() + 1);
	TPtr8 baseAppClassPtr = iBaseAppClass->Des();
	baseAppClassPtr.Copy(aBaseAppClass);
	
    // Try to open database
    cpix_Result result;
    iIdxDb = 
		cpix_IdxSearcher_openDb(
			&result,
            reinterpret_cast<const char*>( baseAppClassPtr.PtrZ() ));
    SearchServerHelper::CheckCpixErrorL(&result, KErrCannotOpenDatabase);

	// Set default analyzer
	SetAnalyzerL(TPtrC((TUint16*)CPIX_ANALYZER_DEFAULT)); 
	}

TBool CCPixSearch::IsOpen()
	{
	return (iIdxDb != NULL);
	}

