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
#include <e32math.h>
#include <e32debug.h>
#include <S32MEM.H>
#include "common.h"
#include "SearchServerLogger.h"
#include "SearchServerCommon.h"
#include "RSearchServerSession.h"
#include "CSearchDocument.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "rsearchserversessionTraces.h"
#endif



// FUNCTION PROTOTYPES
static TInt StartServer();
static TInt CreateServerProcess();

// RsearchServerSession::RsearchServerSession()
// C++ default constructor can NOT contain any code, that might leave.
EXPORT_C RSearchServerSession::RSearchServerSession(): RSessionBase() 
	{
	// No implementation required
	}

// -----------------------------------------------------------------------------
// RsearchServerSession::Connect()
// Connects to the server and create a session.
// -----------------------------------------------------------------------------
EXPORT_C TInt RSearchServerSession::Connect()
	{
	TInt err = StartServer();

	if ( KErrNone == err )
		{
		err = CreateSession(KSearchServerName, Version(), KDefaultMessageSlots);
		}
	return err;
	}

// -----------------------------------------------------------------------------
// A request to close the session.
// It makes a call to the server, which deletes the object container and object index
// for this session, before calling Close() on the base class.
// -----------------------------------------------------------------------------
EXPORT_C void RSearchServerSession::Close()
{
	SendReceive(ESearchServerCloseSession);
	RHandleBase::Close();
}

// -----------------------------------------------------------------------------
// RsearchServerSession::Version()
// Gets the version number.
// -----------------------------------------------------------------------------
//
EXPORT_C TVersion RSearchServerSession::Version() const
	{
	return (TVersion(KSearchServerMajorVersionNumber, KSearchServerMinorVersionNumber,
			KSearchServerBuildVersionNumber) );
	}

// -----------------------------------------------------------------------------
// RsearchServerSession::DefineVolumeL()
// Drops a qualified BaseAppClass
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RSearchServerSession::DefineVolume(const TDesC& aQualifiedBaseAppClass,
												 const TDesC& aIndexDbPath)
	{
	TIpcArgs args(&aQualifiedBaseAppClass, &aIndexDbPath);
	return SendReceive(ESearchServerDatabaseDefine, args);
	}

// -----------------------------------------------------------------------------
// RsearchServerSession::UnDefineVolumeL()
// Drops a qualified BaseAppClass
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RSearchServerSession::UnDefineVolume(const TDesC& aQualifiedBaseAppClass)
	{
	TIpcArgs args(&aQualifiedBaseAppClass);
	return SendReceive(ESearchServerDatabaseUnDefine, args);
	}

// -----------------------------------------------------------------------------
// RsearchServerSession::StopHouseKeeping()
// Stop housekeeping
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RSearchServerSession::StopHouseKeeping()
    {
    TInt err =  SendReceive(ESearchServerStopHouseKeeping, TIpcArgs());
	return err;
    }

// -----------------------------------------------------------------------------
// RsearchServerSession::ContinueHouseKeeping()
// Continue housekeeping
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RSearchServerSession::ContinueHouseKeeping()
    {
    return SendReceive(ESearchServerContinueHouseKeeping, TIpcArgs());
	}

// -----------------------------------------------------------------------------
// RsearchServerSession::ForceHouseKeeping()
// Force housekeeping
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RSearchServerSession::ForceHouseKeeping()
    {
    return SendReceive(ESearchServerForceHouseKeeping, TIpcArgs());
	}

//
//
// RSearchServerSubSession
//
//


RSearchServerSubSession::RSearchServerSubSession():
	iEstimatedResultsCountPckg(iEstimatedResultsCount), 
	iDocumentSizePckg(iDocumentSize)
	{
	}

TInt RSearchServerSubSession::Open(RSearchServerSession& aSession)
	{
	return CreateSubSession(aSession, ESearchServerCreateSubSession);
	}

void RSearchServerSubSession::Close()
	{
	RSubSessionBase::CloseSubSession(ESearchServerCloseSubSession);
	}

void RSearchServerSubSession::OpenDatabaseL(const TBool aOpenForSearch, const TDesC& aBaseAppClass, const TDesC& aDefaultSearchField)
	{
	TIpcArgs args(&aBaseAppClass, &aDefaultSearchField, aOpenForSearch);
	User::LeaveIfError( SendReceive(ESearchServerOpenDatabase, args) );
	}

void RSearchServerSubSession::OpenDatabase(const TBool aOpenForSearch, const TDesC& aBaseAppClass, const TDesC& aDefaultSearchField, TRequestStatus& aStatus)
	{
	TIpcArgs args(&aBaseAppClass, &aDefaultSearchField, aOpenForSearch);
	SendReceive(ESearchServerOpenDatabase, args, aStatus);
	}

EXPORT_C void RSearchServerSubSession::SetAnalyzerL(const TDesC& aAnalyzer)
	{
	// Message arguments for the server
	TIpcArgs args(&aAnalyzer);

	// This call completes immediately, however the server will not
	// complete the request until later, so don't pass any local
	// descriptors as they will be out of scope by the time the server
	// attempts to read or write
	User::LeaveIfError( SendReceive(ESearchServerSetAnalyzer, args) ); 
	}

EXPORT_C void RSearchServerSubSession::SetAnalyzer(const TDesC& aAnalyzer, TRequestStatus& aStatus)
	{
	// Message arguments for the server
	TIpcArgs args(&aAnalyzer);

	// This call completes immediately, however the server will not
	// complete the request until later, so don't pass any local
	// descriptors as they will be out of scope by the time the server
	// attempts to read or write
	SendReceive(ESearchServerSetAnalyzer, args, aStatus);
	}


// RSearchServerSubSession::Search()
EXPORT_C void RSearchServerSubSession::SearchL(const TDesC& aSearchTerms)
	{
	OstTraceFunctionEntry0( RSEARCHSERVERSUBSESSION_SEARCHL_ENTRY );
	PERFORMANCE_LOG_START("RSearchServerSubSession::SearchL");
	
	iEstimatedResultsCount = 0;

	// Message arguments for the server
	TIpcArgs args(&aSearchTerms, &iEstimatedResultsCountPckg);

	// This call completes immediately, however the server will not
	// complete the request until later, so don't pass any local
	// descriptors as they will be out of scope by the time the server
	// attempts to read or write
	User::LeaveIfError( SendReceive(ESearchServerSearch, args) );
	OstTraceFunctionExit0( RSEARCHSERVERSUBSESSION_SEARCHL_EXIT );
	}

EXPORT_C void RSearchServerSubSession::Search(const TDesC& aSearchTerms, TRequestStatus& aStatus)
	{
	OstTraceFunctionEntry0( RSEARCHSERVERSUBSESSION_SEARCH_ENTRY );
	PERFORMANCE_LOG_START("RSearchServerSubSession::SearchL");
	
	iEstimatedResultsCount = 0;

	// Message arguments for the server
	TIpcArgs args(&aSearchTerms, &iEstimatedResultsCountPckg);

	// This call completes immediately, however the server will not
	// complete the request until later, so don't pass any local
	// descriptors as they will be out of scope by the time the server
	// attempts to read or write
	SendReceive(ESearchServerSearch, args, aStatus);
	OstTraceFunctionExit0( RSEARCHSERVERSUBSESSION_SEARCH_EXIT );
	}

// RSearchServerSubSession::GetEstimatedDocumentCount()
TInt RSearchServerSubSession::GetEstimatedDocumentCount()
	{
	return iEstimatedResultsCount;
	}

// RSearchServerSubSession::GetDocument()
EXPORT_C CSearchDocument* RSearchServerSubSession::GetDocumentL(TInt aIndex)
	{
	OstTraceFunctionEntry0( RSEARCHSERVERSUBSESSION_GETDOCUMENTL_ENTRY );
	PERFORMANCE_LOG_START("RSearchServerSubSession::GetDocumentL");
	
	iDocumentSize = 0;

	// Message arguments for the server
	TIpcArgs args(aIndex, &iDocumentSizePckg);

	User::LeaveIfError( SendReceive(ESearchServerGetDocument, args ) );	
	
	return GetDocumentObjectL(); 
	}

EXPORT_C void RSearchServerSubSession::GetDocument(TInt aIndex, TRequestStatus& aStatus)
	{
	OstTraceFunctionEntry0( RSEARCHSERVERSUBSESSION_GETDOCUMENT_ENTRY );
	PERFORMANCE_LOG_START("RSearchServerSubSession::GetDocument");

	iDocumentSize = 0;

	// Message arguments for the server
	TIpcArgs args(aIndex, &iDocumentSizePckg);

	SendReceive(ESearchServerGetDocument, args, aStatus );	
	OstTraceFunctionExit0( RSEARCHSERVERSUBSESSION_GETDOCUMENT_EXIT );
	}


EXPORT_C CSearchDocument* RSearchServerSubSession::GetDocumentObjectL()
	{
	OstTraceFunctionEntry0( RSEARCHSERVERSUBSESSION_GETDOCUMENTOBJECTL_ENTRY );
	PERFORMANCE_LOG_START("CCPixSearcher::GetDocumentObjectL");
	
	CSearchDocument* document = NULL;
	if (iDocumentSize>0)
		{
		HBufC8* buf = HBufC8::NewLC(iDocumentSize);
		TPtr8 ptr = buf->Des();
		User::LeaveIfError(SendReceive(ESearchServerGetDocumentObject, TIpcArgs(&ptr)));

		RDesReadStream stream;
		stream.Open(ptr);
		stream.PushL();
		document = CSearchDocument::NewLC(stream);
		CleanupStack::Pop(document);
		CleanupStack::PopAndDestroy(&stream);
		CleanupStack::PopAndDestroy(buf);
		}
	
	OstTraceFunctionExit0( RSEARCHSERVERSUBSESSION_GETDOCUMENTOBJECTL_EXIT );
	return document;
	}

// RSearchServerSubSession::CancelSearch()
// Cancels outstanding search from server
EXPORT_C void RSearchServerSubSession::CancelAll() const
	{
	SendReceive(ESearchServerCancelAll, TIpcArgs());
	}

// Adds the serialized document to the index
EXPORT_C void RSearchServerSubSession::Add(const TDesC8& aSerializedDocument, TRequestStatus& aStatus)
	{
	TIpcArgs args(&aSerializedDocument);	
	SendReceive(ESearchServerAdd, args, aStatus);
	}

// Updates the serialized document in the index
EXPORT_C void RSearchServerSubSession::Update(const TDesC8& aSerializedDocument, TRequestStatus& aStatus)
	{
	TIpcArgs args(&aSerializedDocument);	
	SendReceive(ESearchServerUpdate, args, aStatus);
	}

// Deletes the document uid in the index
EXPORT_C void RSearchServerSubSession::Delete(const TDesC& aDocUid, TRequestStatus& aStatus)
	{
	TIpcArgs args(&aDocUid);	
	SendReceive(ESearchServerDelete, args, aStatus);
	}

// Resets all of the documents in the index
EXPORT_C void RSearchServerSubSession::Reset(TRequestStatus& aStatus)
	{
	SendReceive( ESearchServerReset, aStatus );
	}

// Adds the serialized document to the index
EXPORT_C void RSearchServerSubSession::AddL(const TDesC8& aSerializedDocument)
	{
	TIpcArgs args( &aSerializedDocument );
	User::LeaveIfError( SendReceive(ESearchServerAdd, args) );
	}

// Updates the serialized document in the index
EXPORT_C void RSearchServerSubSession::UpdateL(const TDesC8& aSerializedDocument)
	{
	TIpcArgs args(&aSerializedDocument);
	OstTrace0( TRACE_NORMAL, RSEARCHSERVERSUBSESSION_UPDATEL, "RSearchServerSubSession::UpdateL(): sending the request to server" );
	CPIXLOGSTRING( "RSearchServerSubSession::UpdateL(): sending the request to server" );
	User::LeaveIfError( SendReceive(ESearchServerUpdate, args) );
	OstTrace0( TRACE_NORMAL, DUP1_RSEARCHSERVERSUBSESSION_UPDATEL, "RSearchServerSubSession::UpdateL(): sent the request to server success" );
	CPIXLOGSTRING( "RSearchServerSubSession::UpdateL(): sent the request to server success" );
	}

// Deletes the document uid in the index
EXPORT_C void RSearchServerSubSession::DeleteL(const TDesC& aDocUid)
	{
	TIpcArgs args(&aDocUid);	
	User::LeaveIfError( SendReceive(ESearchServerDelete, args) );
	}

// Resets all of the documents in the index
EXPORT_C void RSearchServerSubSession::ResetL()
	{
	User::LeaveIfError( SendReceive(ESearchServerReset) );
	}

// Issues CPix flush
EXPORT_C void RSearchServerSubSession::FlushL()
    {
    User::LeaveIfError( SendReceive(ESearchServerFlush) );
    }

// Issues CPix flush
EXPORT_C void RSearchServerSubSession::Flush(TRequestStatus& aStatus)
    {
    SendReceive(ESearchServerFlush, aStatus);
    }


// StartServer()
// Starts the server if it is not already running
static TInt StartServer()
	{
	TInt result;

	TFindServer findsearchServer(KSearchServerName);
	TFullName name;

	result = findsearchServer.Next(name);
	if (result == KErrNone)
		{
		// Server already running
		return KErrNone;
		}

	RSemaphore semaphore;
	result = semaphore.CreateGlobal(KSearchServerSemaphoreName, 0);
	if (result != KErrNone)
		{
		return result;
		}

	result = CreateServerProcess();
	if (result != KErrNone)
		{
		return result;
		}

	semaphore.Wait();
	semaphore.Close();

	return KErrNone;
	}

// CreateServerProcess()
// Creates a server process
static TInt CreateServerProcess()
	{
	TInt result;

	const TUidType serverUid( KNullUid, KNullUid, KServerUid3);

	RProcess server;

	result = server.Create(KSearchServerFilename, KNullDesC, serverUid);
	if (result != KErrNone)
		{
		return result;
		}

	server.Resume();
	server.Close();
	return KErrNone;
	}

// End of File

