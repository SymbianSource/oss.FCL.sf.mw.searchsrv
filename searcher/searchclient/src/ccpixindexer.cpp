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
#include <S32MEM.H>

#include "SearchClient.pan"
#include "CCPixIndexer.h"
#include "MCPixIndexerObserver.h"
#include "SearchServerCommon.h"
#include "CSearchDocument.h"

// CCPixIndexer::NewL()
// Two-phased constructor.
EXPORT_C CCPixIndexer* CCPixIndexer::NewL(RSearchServerSession& aSearchServerSession)
	{
	CCPixIndexer* self = NewLC(aSearchServerSession);
	CleanupStack::Pop( self );
	return self;
	}

// CCPixIndexer::NewLC()
// Two-phased constructor.
EXPORT_C CCPixIndexer* CCPixIndexer::NewLC(RSearchServerSession& aSearchServerSession)
	{
	CCPixIndexer* self = new ( ELeave ) CCPixIndexer(aSearchServerSession);
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
	}

// CCPixIndexer::ConstructL()
// Symbian 2nd phase constructor can leave.
void CCPixIndexer::ConstructL()
	{
	User::LeaveIfError( iSubSession.Open(iSearchServerSession) );
	}

// CCPixIndexer::CCPixIndexer()
// C++ default constructor can NOT contain any code, that might leave.
CCPixIndexer::CCPixIndexer(RSearchServerSession& aSearchServerSession)
	: CActive( CActive::EPriorityStandard ),
	  iSearchServerSession(aSearchServerSession),
	  iState( EStateNone )
	{
	CActiveScheduler::Add( this );
	}

// CCPixIndexer::~CCPixIndexer()
// Destructor.
EXPORT_C CCPixIndexer::~CCPixIndexer()
	{
	Cancel(); // releases temporary values used in requests
	iSubSession.Close();
	
	delete iSerializedDocument;
	delete iBaseAppClass;
	delete iAnalyzer;
	delete iDocUidToDelete;
	}
 
// Helper function to serialize a document
void CCPixIndexer::SerializeDocumentL(const CSearchDocument& aDocument)
	{
	if (iSerializedDocument)
		{
		delete iSerializedDocument;
		iSerializedDocument = NULL;
		}
	iSerializedDocument = HBufC8::NewL(aDocument.Size());
	TPtr8 ptr = iSerializedDocument->Des();
	
	// Initialize a new stream
	RDesWriteStream stream;
	stream.Open(ptr);
	stream.PushL();

	// Externalize hits to the stream
	aDocument.ExternalizeL(stream);

	// Commit and destroy the stream
	stream.CommitL();
	CleanupStack::PopAndDestroy(&stream);	
	}

EXPORT_C TBool CCPixIndexer::IsDatabaseOpen() const
	{
	return iIsDatabaseOpen; 
	}

EXPORT_C const TDesC& CCPixIndexer::GetBaseAppClass() const
{
	if (iBaseAppClass)
		return *iBaseAppClass;
	else
		return KNullDesC;
}

EXPORT_C void CCPixIndexer::OpenDatabaseL(const TDesC& aBaseAppClass )
	{
	if (IsActive())
		{
		User::Leave(KErrInUse);
		}
	iIsDatabaseOpen = EFalse; // If opening database fails, this is the safe assumption
	delete iBaseAppClass; 
	iBaseAppClass = NULL; 
	iBaseAppClass = aBaseAppClass.AllocL(); 
	iSubSession.OpenDatabaseL(EFalse, *iBaseAppClass, KNullDesC); // Create if not found
	iIsDatabaseOpen = ETrue;
	}

EXPORT_C void CCPixIndexer::OpenDatabaseL(MCPixOpenDatabaseRequestObserver& aObserver, const TDesC& aBaseAppClass )
	{
	if (IsActive())
		User::Leave(KErrInUse);
	
	delete iBaseAppClass;
	iBaseAppClass = NULL; 
	iBaseAppClass = aBaseAppClass.AllocL(); 
	iIsDatabaseOpen = EFalse; // If opening database fails, this is the safe assumption

	iRequestObserver.iOpenDatabase = &aObserver;
	iState = EStateOpenDatabase;
	iSubSession.OpenDatabase(EFalse, *iBaseAppClass, KNullDesC, iStatus); // Create if not found
	SetActive();
	}

EXPORT_C void CCPixIndexer::SetAnalyzerL( const TDesC& aAnalyzer )
	{
	if ( !iIsDatabaseOpen ) 	User::Leave(KErrNotReady);
	if ( IsActive() ) 			User::Leave(KErrInUse);

	iSubSession.SetAnalyzerL( aAnalyzer ); 
	}

EXPORT_C void CCPixIndexer::SetAnalyzerL( MCPixSetAnalyzerRequestObserver& aObserver, const TDesC& aAnalyzer )
	{
	if ( !iIsDatabaseOpen ) 	User::Leave(KErrNotReady);
	if ( IsActive() ) 			User::Leave(KErrInUse);
	
	iAnalyzer = aAnalyzer.AllocL(); 

	iRequestObserver.iSetAnalyzer = &aObserver;
	iState = EStateSetAnalyzer; 
	iSubSession.SetAnalyzer( aAnalyzer, iStatus );
	SetActive(); 
	}


EXPORT_C void CCPixIndexer::AddL(const CSearchDocument& aDocument)
	{
	if ( !iIsDatabaseOpen ) 	User::Leave(KErrNotReady);
	if ( IsActive() ) 			User::Leave(KErrInUse);
	
	SerializeDocumentL(aDocument);
	
	iSubSession.AddL(*iSerializedDocument);
	}

EXPORT_C void CCPixIndexer::AddL(MCPixIndexingRequestObserver& aObserver, const CSearchDocument& aDocument)
	{
	if ( !iIsDatabaseOpen ) 	User::Leave(KErrNotReady);
	if ( IsActive() ) 			User::Leave(KErrInUse);
	
	SerializeDocumentL(aDocument);
	
	iRequestObserver.iIndexing = &aObserver;
	iState = EStateAdd;	
	iSubSession.Add(*iSerializedDocument, iStatus);
	SetActive();
	}

EXPORT_C void CCPixIndexer::UpdateL(const CSearchDocument& aDocument)
	{
	if ( !iIsDatabaseOpen ) 	User::Leave(KErrNotReady);
	if ( IsActive() ) 			User::Leave(KErrInUse);

	SerializeDocumentL(aDocument);
	iSubSession.UpdateL(*iSerializedDocument);
	}

EXPORT_C void CCPixIndexer::UpdateL(MCPixIndexingRequestObserver& aObserver, const CSearchDocument& aDocument)
	{
	if ( !iIsDatabaseOpen ) 	User::Leave(KErrNotReady);
	if ( IsActive() ) 			User::Leave(KErrInUse);
		
	SerializeDocumentL(aDocument);
	
	iRequestObserver.iIndexing = &aObserver;
	iState = EStateUpdate;
	iSubSession.Update(*iSerializedDocument, iStatus);
	SetActive();
	}

EXPORT_C void CCPixIndexer::DeleteL(const TDesC& aDocUid)
	{
	if ( !iIsDatabaseOpen ) 	User::Leave(KErrNotReady);
	if ( IsActive() ) 			User::Leave(KErrInUse);
	
	iSubSession.DeleteL(aDocUid);
	}

EXPORT_C void CCPixIndexer::DeleteL(MCPixIndexingRequestObserver& aObserver, const TDesC& aDocUid)
	{
	if ( !iIsDatabaseOpen ) 	User::Leave(KErrNotReady);
	if ( IsActive() ) 			User::Leave(KErrInUse);

	delete iDocUidToDelete;
	iDocUidToDelete = NULL;

	iDocUidToDelete = aDocUid.AllocL();

	iRequestObserver.iIndexing = &aObserver;
	iState = EStateDelete;
	iSubSession.Delete(*iDocUidToDelete, iStatus);
	SetActive();
	}


EXPORT_C void CCPixIndexer::ResetL()
	{
	if ( !iIsDatabaseOpen ) 	User::Leave(KErrNotReady);
	if ( IsActive() ) 			User::Leave(KErrInUse);

	iSubSession.ResetL();
	}

EXPORT_C void CCPixIndexer::ResetL(MCPixIndexingRequestObserver& aObserver)
	{
	if ( !iIsDatabaseOpen ) 	User::Leave(KErrNotReady);
	if ( IsActive() ) 			User::Leave(KErrInUse);

	iRequestObserver.iIndexing = &aObserver;
	iState = EStateReset;
	iSubSession.Reset(iStatus);
	SetActive();
	}

EXPORT_C void CCPixIndexer::FlushL()
    {
    if ( !iIsDatabaseOpen )     User::Leave(KErrNotReady);
    if ( IsActive() )           User::Leave(KErrInUse);
    
    iSubSession.FlushL();
    }

EXPORT_C void CCPixIndexer::FlushL(MCPixIndexingRequestObserver& aObserver)
    {
    if ( !iIsDatabaseOpen )     User::Leave(KErrNotReady);
    if ( IsActive() )           User::Leave(KErrInUse);
    
    iRequestObserver.iIndexing = &aObserver;
    iState = EStateFlush; 
    iSubSession.Flush(iStatus);
    SetActive();
    }

// CCPixIndexer::RunL()
// Invoked to handle responses from the server.
void CCPixIndexer::RunL()
	{
	TState oldState = iState;
	iState = EStateNone; 
	
	TRequestObserver observer = iRequestObserver; 
	iRequestObserver.iAny = NULL;
	
	if (iSerializedDocument)
		{
		delete iSerializedDocument;
		iSerializedDocument = NULL;
		}

	switch (oldState) 
		{
		case EStateOpenDatabase: 
			iIsDatabaseOpen = (iStatus.Int() == KErrNone);
			if ( observer.iOpenDatabase ) 
				observer.iOpenDatabase->HandleOpenDatabaseResultL(iStatus.Int());
			break;
		case EStateSetAnalyzer:
			delete iAnalyzer; iAnalyzer = NULL; 
			if ( observer.iSetAnalyzer ) 
				observer.iSetAnalyzer->HandleSetAnalyzerResultL(iStatus.Int());
			break;
		case EStateDelete: 
			delete iDocUidToDelete;
			iDocUidToDelete = NULL;
			// fallthrough
		case EStateAdd: 		// fallthrough
		case EStateUpdate: 		// fallthrough
		case EStateReset:
		case EStateFlush:
			if ( observer.iIndexing ) 
				observer.iIndexing->HandleIndexingResultL(iStatus.Int());
			break;
		}

	}

// CCPixIndexer::DoCancel()
// Cancels any outstanding operation.
void CCPixIndexer::DoCancel()
	{
	iSubSession.CancelAll();
	}

TInt CCPixIndexer::RunError(TInt /* aError */)
	{
	return KErrNone; // Don't panic
	}

// End of File
