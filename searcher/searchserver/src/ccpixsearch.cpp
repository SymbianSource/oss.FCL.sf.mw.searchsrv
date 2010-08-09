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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "ccpixsearchTraces.h"
#endif




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
	: iQueryParserType(EIncrementalQueryParser),
	  iPendingTask(EPendingTaskNone)
	  
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
	delete ibatchDocuments;
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
    OstTrace1( TRACE_NORMAL, CCPIXSEARCH_LOGCANCELACTION, "CCPixSearch::LogCancelAction;Cancel action code=%d", aCancelAction );
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
    OstTraceFunctionEntry0( CCPIXSEARCH_SEARCHL_ENTRY );
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
	    
        OstTraceFunctionExit0( CCPIXSEARCH_SEARCHL_EXIT );
        return ETrue;
        }
    
    // Search was not committed
    OstTraceFunctionExit0( DUP1_CCPIXSEARCH_SEARCHL_EXIT );
    return EFalse;
    }

TInt CCPixSearch::SearchCompleteL()
    {
    OstTraceFunctionEntry0( CCPIXSEARCH_SEARCHCOMPLETEL_ENTRY );
    PERFORMANCE_LOG_START("CCPixSearch::SearchCompleteL");
    // Request is no more pending
    iPendingTask = EPendingTaskNone;
    
    // Get the hits
    iHits = cpix_IdxSearcher_asyncSearchResults(iIdxDb, iPendingJobId);
    SearchServerHelper::CheckCpixErrorL(iIdxDb, KErrDatabaseQueryFailed);
	iDocumentCount = cpix_Hits_length(iHits);
	SearchServerHelper::CheckCpixErrorL(iHits, KErrDatabaseQueryFailed);
	
	OstTraceFunctionExit0( CCPIXSEARCH_SEARCHCOMPLETEL_EXIT );
	return iDocumentCount; 
    }
       
void CCPixSearch::GetDocumentL(TInt aIndex, MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage)
	{
	OstTraceFunctionEntry0( CCPIXSEARCH_GETDOCUMENTL_ENTRY );
	PERFORMANCE_LOG_START("CCPixSearch::GetDocumentL");
	
    if (iPendingTask != EPendingTaskNone)
        User::Leave(KErrInUse);
		
	if (aIndex<0 || aIndex >= iDocumentCount )
		{
		User::Leave(KErrDocumentAccessFailed);
		}
	
    iPendingTask = EPendingTaskDocument;
    
    iCurrentCpixDocument = new cpix_Document;
    iCurrentCpixDocument->ptr_ = NULL;
    iPendingJobId = cpix_Hits_asyncDoc(iHits, aIndex, &iCurrentCpixDocument, (void*)this, &CompletionCallback,1);
    if ( cpix_Failed(iHits) )
        {
        SearchServerHelper::LogErrorL(*(iHits->err_));
        cpix_ClearError(iHits);
        User::Leave(KErrDocumentAccessFailed);
        }
    iAsyncronizer->Start(ECPixTaskTypeGetDocument, aObserver, aMessage);
	OstTraceFunctionExit0( CCPIXSEARCH_GETDOCUMENTL_EXIT );
	}
	
void CCPixSearch::GetBatchDocumentL(TInt aIndex, MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage, TInt aCount)
	{	
	PERFORMANCE_LOG_START("CCPixSearch::GetBatchDocumentL");
	
	if (iPendingTask != EPendingTaskNone)
	     User::Leave(KErrInUse);
	
	if (aIndex<0 || aIndex >= iDocumentCount )
        {
        User::Leave(KErrDocumentAccessFailed);
        }
	
	ibatchDocCount = aCount;	
	ibatchDocuments = STATIC_CAST(cpix_Document**, User::AllocZL(aCount * sizeof(cpix_Document*)));
	for (int i =0; i< aCount; i++)
	    {
            ibatchDocuments[i] = new cpix_Document;
            ibatchDocuments[i]->ptr_ = NULL;
	    }
	
    iPendingTask = EPendingTaskDocument;
    iPendingJobId = cpix_Hits_asyncDoc(iHits, aIndex, ibatchDocuments, (void*)this, &CompletionCallback, aCount);
    if ( cpix_Failed(iHits) )
        {
        SearchServerHelper::LogErrorL(*(iHits->err_));
        cpix_ClearError(iHits);
        User::Leave(KErrDocumentAccessFailed);
        }
    iAsyncronizer->Start(ECPixTaskTypeGetBatchDocument, aObserver, aMessage);
	}

CSearchDocument* CCPixSearch::GetDocumentCompleteL()
	{
	OstTraceFunctionEntry0( CCPIXSEARCH_GETDOCUMENTCOMPLETEL_ENTRY );
	PERFORMANCE_LOG_START("CCPixSearch::GetDocumentCompleteL");
	
    // Request is no more pending
    iPendingTask = EPendingTaskNone;
	
	cpix_Hits_asyncDocResults(iHits, iPendingJobId);
	SearchServerHelper::CheckCpixErrorL(iHits, KErrDocumentAccessFailed);
	
	return ConvertDocumentL( iCurrentCpixDocument );
#if 0 // TODO XXX TIM
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
	
	OstTraceFunctionExit0( CCPIXSEARCH_GETDOCUMENTCOMPLETEL_EXIT );
	return document;
#endif // 0
	}
	
RPointerArray<CSearchDocument> CCPixSearch::GetBatchDocumentCompleteL()
	{
	PERFORMANCE_LOG_START("CCPixSearch::GetBatchDocumentCompleteL");
	
    // Request is no more pending
    iPendingTask = EPendingTaskNone;
	
	cpix_Hits_asyncDocResults(iHits, iPendingJobId);
	SearchServerHelper::CheckCpixErrorL(iHits, KErrDocumentAccessFailed);
	
	return ConvertBatchDocumentL( ibatchDocuments, ibatchDocCount );
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
	
	RefreshQueryParserL(); 
	}

void CCPixSearch::SetQueryParserL(TInt aQueryParser)
	{
	iQueryParserType = TQueryParser(aQueryParser); 
	RefreshQueryParserL(); 
	}

void CCPixSearch::RefreshQueryParserL() 
	{
	cpix_QueryParser_destroy( iQueryParser );
	iQueryParser = NULL; 
	cpix_Result result; 
	
	if ( iQueryParserType == EDatabaseQueryParser ) 
		{
		iQueryParser = 
			cpix_QueryParser_create( &result, 
									  reinterpret_cast<const wchar_t*>(
									      iDefaultSearchFieldZ->Des().PtrZ()), 
									  iAnalyzer );
		} 
	else if ( iQueryParserType == EIncrementalQueryParser ) 
		{
		iQueryParser = 
			cpix_CreatePrefixQueryParser( &result, 
										  reinterpret_cast<const wchar_t*>(
										      iDefaultSearchFieldZ->Des().PtrZ()) );
		
		}
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
// End of File
