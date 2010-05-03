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

#include <S32MEM.H>
#include <e32svr.h>
#include <e32uid.h>
#include <common.h>

#include "CSearchServerSession.h"
#include "CSearchServerSubSession.h"
#include "CCPixAsyncronizer.h"
#include "CCPixIdxDb.h"
#include "CCPixSearch.h"
#include "CSearchDocument.h"
#include "SearchServerLogger.h"
#include "SearchServer.pan"
#include "CLogPlayerRecorder.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "csearchserversubsessionTraces.h"
#endif


// Contants
//_LIT8( KFileBaseAppClass, "root file" );

CSearchServerSubSession* CSearchServerSubSession::NewLC(CSearchServerSession* aSession)
{
	CSearchServerSubSession* self = new (ELeave)CSearchServerSubSession(aSession);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

CSearchServerSubSession* CSearchServerSubSession::NewL(CSearchServerSession* aSession)
{
	CSearchServerSubSession* self = CSearchServerSubSession::NewLC(aSession);
	CleanupStack::Pop();
	return self;
}

CSearchServerSubSession::CSearchServerSubSession(CSearchServerSession* aSession) : iSession(aSession)
{
    OstTrace0( TRACE_NORMAL, CSEARCHSERVERSUBSESSION_CSEARCHSERVERSUBSESSION, "Creating search server subsession" );
    CPIXLOGSTRING("Creating search server subsession");
	
    // Dont ask why but I seem to have a compulsion to set everything NULL in constructor - AL
	iIndexDb = NULL;
	iSearchDb = NULL;
	iNextDocument = NULL;
}

CSearchServerSubSession::~CSearchServerSubSession()
{
    OstTrace0( TRACE_NORMAL, DUP1_CSEARCHSERVERSUBSESSION_CSEARCHSERVERSUBSESSION, "Deleting search server subsession" );
    CPIXLOGSTRING("Deleting search server subsession");

	delete iIndexDb;
	delete iSearchDb;	
	delete iNextDocument;
}

void CSearchServerSubSession::ConstructL()
{
	iIndexDb = CCPixIdxDb::NewL();
	iSearchDb = CCPixSearch::NewL();
}

// CSearchServerSession::CancelAll()
void CSearchServerSubSession::CancelAll(const RMessage2& aMessage)
	{
	if (iSearchDb->IsOpen())
	    {
	    // Cancel searching 
	    iSearchDb->CancelAll(aMessage);
	    }
	else if (iIndexDb->IsOpen())
	    {
	    // Cancel indexing
	    iIndexDb->CancelAll(aMessage);
	    }
	else
	    {
	    // Nothing to cancel. Just complete request
	    aMessage.Complete(KErrNone);
	    }
	
	TRAP_IGNORE( LOG_PLAYER_RECORD( CLogPlayerRecorder::LogCancelL( reinterpret_cast<TUint>( this ) ) ) );
	}

void CSearchServerSubSession::OpenDatabaseL(const RMessage2& aMessage)
	{
	HBufC* baseAppClass = HBufC::NewLC(aMessage.GetDesLength(0));
	TPtr baseAppClassPtr = baseAppClass->Des();
	aMessage.ReadL(0, baseAppClassPtr);
	
	HBufC* defaultSearchField = HBufC::NewLC(aMessage.GetDesLength(1));
	TPtr defaultSearchFieldPtr = defaultSearchField->Des();
	aMessage.ReadL(1, defaultSearchFieldPtr);
	
	const TBool search = aMessage.Int2();
    
	LOG_PLAYER_RECORD( CLogPlayerRecorder::LogDatabaseOpenL(reinterpret_cast<TUint>(this), search, *baseAppClass) );

	if (!search)
	{
    	iIndexDb->OpenDatabaseL(*baseAppClass);
	}
	else
	{
		iSearchDb->OpenDatabaseL(*baseAppClass, *defaultSearchField);
	}
	
	CleanupStack::PopAndDestroy(defaultSearchField);
	CleanupStack::PopAndDestroy(baseAppClass);

	// Complete the request
	aMessage.Complete(KErrNone);	
	}

void CSearchServerSubSession::SetAnalyzerL(const RMessage2& aMessage)
	{
	if (!iSearchDb->IsOpen() && !iIndexDb->IsOpen())
		{
		iSession->PanicClient(aMessage, EDatabaseNotOpen);
		return;
		}
	
	HBufC* analyzer = HBufC::NewLC(aMessage.GetDesLength(0));
	TPtr analyzerPtr = analyzer->Des();
	aMessage.ReadL(0, analyzerPtr);

	if (iSearchDb->IsOpen()) {
		iSearchDb->SetAnalyzerL( *analyzer ); 
	}
	if (iIndexDb->IsOpen()) {
		iIndexDb->SetAnalyzerL( *analyzer ); 
	}
	CleanupStack::PopAndDestroy( analyzer );
	
	// Complete the request
	aMessage.Complete(KErrNone);	
	}

void CSearchServerSubSession::SearchL(const RMessage2& aMessage)
	{
	OstTraceFunctionEntry0( CSEARCHSERVERSUBSESSION_SEARCHL_ENTRY );
	PERFORMANCE_LOG_START("CSearchServerSubSession::SearchL");
	
	// Sanity check
	if (!iSearchDb->IsOpen())
		{
		iSession->PanicClient(aMessage, EDatabaseNotOpen);
		OstTraceFunctionExit0( CSEARCHSERVERSUBSESSION_SEARCHL_EXIT );
		return;
		}
	
	// buf for the search terms
	HBufC* searchTerms = HBufC::NewLC(aMessage.GetDesLength(0));
	
	// read the search terms
	TPtr ptr = searchTerms->Des();
	aMessage.ReadL(0, ptr);

	// Commit search
	LOG_PLAYER_RECORD( CLogPlayerRecorder::LogSearchL( reinterpret_cast<TUint>(this), *searchTerms ) );
	if (!iSearchDb->SearchL(*searchTerms, this, aMessage))
		{
		// Not commited, complete with zero count
		TPckgBuf<TInt> resultCountPackage(0);
		aMessage.WriteL(1, resultCountPackage);

		// Complete the request
		aMessage.Complete(KErrNone);	
		}
	
	// Cleanup search terms
	CleanupStack::PopAndDestroy(searchTerms);
	OstTraceFunctionExit0( DUP1_CSEARCHSERVERSUBSESSION_SEARCHL_EXIT );
	}
	
void CSearchServerSubSession::SearchCompleteL(const RMessage2& aMessage)
	{
	OstTraceFunctionEntry0( CSEARCHSERVERSUBSESSION_SEARCHCOMPLETEL_ENTRY );
	PERFORMANCE_LOG_START("CSearchServerSubSession::SearchCompleteL");
	
	// Complete search
	const TInt resultCount = iSearchDb->SearchCompleteL();
		
	// Return the size of the search results in first parameter
	TPckgBuf<TInt> resultCountPackage(resultCount);
	aMessage.WriteL(1, resultCountPackage);
	
	OstTraceFunctionExit0( CSEARCHSERVERSUBSESSION_SEARCHCOMPLETEL_EXIT );
	}

// CSearchServerSession::GetDocumentL().
// Client gets the next documents (result) when SearchL has completed
void CSearchServerSubSession::GetDocumentL(const RMessage2& aMessage)
	{
	OstTraceFunctionEntry0( CSEARCHSERVERSUBSESSION_GETDOCUMENTL_ENTRY );
	PERFORMANCE_LOG_START("CSearchServerSubSession::GetDocumentL");
	
	// Sanity check
	if (!iSearchDb->IsOpen())
		{
		iSession->PanicClient(aMessage, EDatabaseNotOpen);
		OstTraceFunctionExit0( CSEARCHSERVERSUBSESSION_GETDOCUMENTL_EXIT );
		return;
		}
	
	// buf for the search terms
	TInt index = aMessage.Int0();
	
	delete iNextDocument; 
	iNextDocument = NULL;
	LOG_PLAYER_RECORD( CLogPlayerRecorder::LogGetDocumentL( reinterpret_cast<TUint>( this ), index ) );
	iSearchDb->GetDocumentL(index, this, aMessage);
	OstTraceFunctionExit0( DUP1_CSEARCHSERVERSUBSESSION_GETDOCUMENTL_EXIT );
	}
	
void CSearchServerSubSession::GetDocumentCompleteL(const RMessage2& aMessage)
	{
	OstTraceFunctionEntry0( CSEARCHSERVERSUBSESSION_GETDOCUMENTCOMPLETEL_ENTRY );
	PERFORMANCE_LOG_START("CSearchServerSubSession::GetDocumentCompleteL");
	
	iNextDocument = iSearchDb->GetDocumentCompleteL();	
	TPckgBuf<TInt> documentSizePackage(iNextDocument ? iNextDocument->Size() : 0);
	aMessage.WriteL(1, documentSizePackage);
	OstTraceFunctionExit0( CSEARCHSERVERSUBSESSION_GETDOCUMENTCOMPLETEL_EXIT );
	}

// CSearchServerSession::GetDocumentObjectL()
// Client gets the object after GetDocumentL() has completed 
void CSearchServerSubSession::GetDocumentObjectL(const RMessage2& aMessage)
	{
	OstTraceFunctionEntry0( CSEARCHSERVERSUBSESSION_GETDOCUMENTOBJECTL_ENTRY );
	PERFORMANCE_LOG_START("CSearchServerSubSession::GetDocumentObjectL");
	
	// Sanity check
	if (!iSearchDb->IsOpen())
		{
		iSession->PanicClient(aMessage, EDatabaseNotOpen);
		OstTraceFunctionExit0( CSEARCHSERVERSUBSESSION_GETDOCUMENTOBJECTL_EXIT );
		return;
		}
	
	if (iNextDocument)
		{
		// Create long enough descriptor for serialized hits
		HBufC8* buf = HBufC8::NewLC(iNextDocument->Size());
		TPtr8 ptr = buf->Des();
	
		// Initialize a new stream
		RDesWriteStream stream;
		stream.Open(ptr);
		stream.PushL();
	
		// Externalize hits to the stream
		iNextDocument->ExternalizeL(stream);
	
		// Commit and destroy the stream
		stream.CommitL();
		CleanupStack::PopAndDestroy(&stream);
	
		// write the serialized hits in to the message
		aMessage.WriteL(0, ptr);
	
		// Delete descriptor
		CleanupStack::PopAndDestroy(buf);
		}
	else
		{
		// Empty
		aMessage.WriteL(0, KNullDesC8);
		}
	
	// Complete the request
	aMessage.Complete(KErrNone);		
	OstTraceFunctionExit0( DUP1_CSEARCHSERVERSUBSESSION_GETDOCUMENTOBJECTL_EXIT );
	}

void CSearchServerSubSession::AddL(const RMessage2& aMessage)
	{
	// Sanity check
	if (!iIndexDb->IsOpen())
		{
		iSession->PanicClient(aMessage, EDatabaseNotOpen);
		return;
		}
	
	// buf for the search terms
	HBufC8* serializedDocument = HBufC8::NewLC(aMessage.GetDesLength(0));
	
	// read the search terms
	TPtr8 ptr = serializedDocument->Des();
	aMessage.ReadL(0, ptr);

	RDesReadStream stream;
	stream.Open(ptr);
	stream.PushL();
	CSearchDocument* document = CSearchDocument::NewL(stream);
	CleanupStack::PopAndDestroy(&stream);
	CleanupStack::PushL(document);
	
	// Index the thing
	LOG_PLAYER_RECORD(
	if ( iIndexDb->BaseAppClass().Find( KFileBaseAppClass ) == KErrNotFound )
	    {
		CLogPlayerRecorder::LogAddL( reinterpret_cast<TUint>(this), *document );
	    }
	);
	//check if excerpt is more then maximum allowed
	LimitExcerptToMaxLengthL(document);
	iIndexDb->AddL(*document, this, aMessage);
	
	CleanupStack::PopAndDestroy(document);
	CleanupStack::PopAndDestroy(serializedDocument);
	}

void CSearchServerSubSession::LimitExcerptToMaxLengthL(CSearchDocument* aSearchDocument)
    {
    //check if excerpt is more then maximum allowed
    if(aSearchDocument->Excerpt().Length() > MAX_EXCERPT_LENGTH)
        {
        OstTraceExt1( TRACE_NORMAL, CSEARCHSERVERSUBSESSION_LIMITEXCERPTTOMAXLENGTHL, "CSearchServerSubSession::LimitExcerptToMaxLengthL;docuid=%S", (aSearchDocument->Id()) );
        CPIXLOGSTRING2("CSearchServerSubSession::AddL docuid=%S", &(aSearchDocument->Id()));
        OstTrace1( TRACE_NORMAL, DUP1_CSEARCHSERVERSUBSESSION_LIMITEXCERPTTOMAXLENGTHL, "CSearchServerSubSession::LimitExcerptToMaxLengthL;Excerpt Length=%d", aSearchDocument->Excerpt().Length() );
        CPIXLOGSTRING2("CSearchServerSubSession::AddL Excerpt Length =%d", aSearchDocument->Excerpt().Length());
        const TDesC& excerpt = aSearchDocument->Excerpt();
        HBufC* bufExcerpt = HBufC::NewLC(MAX_EXCERPT_LENGTH);
        TPtr ptrExcerpt = bufExcerpt->Des();
        ptrExcerpt.Append(excerpt.Ptr(),MAX_EXCERPT_LENGTH - 1);
        ptrExcerpt.Append(KNullDesC); //Append NULL
        aSearchDocument->AddExcerptL(*bufExcerpt);
        CleanupStack::PopAndDestroy(bufExcerpt);
        }
    }

void CSearchServerSubSession::AddCompleteL(const RMessage2& /*aMessage*/)
	{
	iIndexDb->AddCompleteL();	
	}


void CSearchServerSubSession::UpdateL(const RMessage2& aMessage)
	{
	// Sanity check
	if (!iIndexDb->IsOpen())
		{
		iSession->PanicClient(aMessage, EDatabaseNotOpen);
		return;
		}

	// buf for the search terms
	HBufC8* serializedDocument = HBufC8::NewLC(aMessage.GetDesLength(0));
	
	// read the search terms
	TPtr8 ptr = serializedDocument->Des();
	aMessage.ReadL(0, ptr);

	RDesReadStream stream;
	stream.Open(ptr);
	stream.PushL();
	CSearchDocument* document = CSearchDocument::NewL(stream);
	CleanupStack::PopAndDestroy(&stream);
	CleanupStack::PushL(document);
	
	// Index the thing
	LOG_PLAYER_RECORD( CLogPlayerRecorder::LogUpdateL( reinterpret_cast<TUint>(this), *document ) );
	LimitExcerptToMaxLengthL(document);
	iIndexDb->UpdateL(*document, this, aMessage);
	
	CleanupStack::PopAndDestroy(document);
	CleanupStack::PopAndDestroy(serializedDocument);
	}

void CSearchServerSubSession::UpdateCompleteL(const RMessage2& /*aMessage*/)
	{
	iIndexDb->UpdateCompleteL();	
	}


void CSearchServerSubSession::DeleteL(const RMessage2& aMessage)
	{
	// Sanity check
	if (!iIndexDb->IsOpen())
		{
		iSession->PanicClient(aMessage, EDatabaseNotOpen);
		return;
		}

	HBufC* docUid = HBufC::NewLC(aMessage.GetDesLength(0));

	TPtr ptr = docUid->Des();
	aMessage.ReadL(0, ptr);

	LOG_PLAYER_RECORD( CLogPlayerRecorder::LogDeleteL( reinterpret_cast<TUint>(this), *docUid ) );
	iIndexDb->DeleteDocumentsL(*docUid, this, aMessage);

	CleanupStack::PopAndDestroy(docUid);
	}

void CSearchServerSubSession::DeleteCompleteL(const RMessage2& /*aMessage*/)
	{
	iIndexDb->DeleteDocumentsCompleteL();	
	}

void CSearchServerSubSession::FlushCompleteL(const RMessage2& /*aMessage*/)
	{
	iIndexDb->FlushCompleteL();	
	}

void CSearchServerSubSession::ResetL(const RMessage2& aMessage)
	{
	// Sanity check
	if (!iIndexDb->IsOpen())
		{
		iSession->PanicClient(aMessage, EDatabaseNotOpen);
		return;
		}

	// Reset database
	LOG_PLAYER_RECORD( CLogPlayerRecorder::LogResetL( reinterpret_cast<TUint>(this) ) );
	iIndexDb->ResetL();
	
	// Complete the request
	aMessage.Complete(KErrNone);		
	}

void CSearchServerSubSession::FlushL(const RMessage2& aMessage)
    {
    // Sanity check
    if (!iIndexDb->IsOpen())
        {
        iSession->PanicClient(aMessage, EDatabaseNotOpen);
        return;
        }
    
    LOG_PLAYER_RECORD( CLogPlayerRecorder::LogFlushL( reinterpret_cast<TUint>(this) ) );
    iIndexDb->FlushL(this, aMessage);
    }


void CSearchServerSubSession::HandleAsyncronizerComplete(TCPixTaskType aType, TInt aError, const RMessage2& aMessage)
	{
	if (aError == KErrNone)
		{
		switch (aType)
			{
			case ECPixTaskTypeSearch:
				{
				TRAP(aError, SearchCompleteL(aMessage));
				break;
				}
			case ECPixTaskTypeGetDocument:
				{
				TRAP(aError, GetDocumentCompleteL(aMessage));
				break;
				}
			case ECPixTaskTypeAdd:
				{
				TRAP(aError, AddCompleteL(aMessage));
				break;
				}
			case ECPixTaskTypeUpdate:
				{
				TRAP(aError, UpdateCompleteL(aMessage));
				break;
				}
			case ECPixTaskTypeDelete:
				{
				TRAP(aError, DeleteCompleteL(aMessage));
				break;
				}
			case ECPixTaskTypeFlush:
				{
				TRAP(aError, FlushCompleteL(aMessage));
				break;
				}
			default:
				{
				// No need for completion code for the rest
				break;
				}
			}
		}
	
	// Complete the request
	aMessage.Complete(aError);		
	}

