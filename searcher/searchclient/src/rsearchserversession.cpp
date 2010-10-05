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
#include "cpixwatchdogcommon.h"
#include <centralrepository.h>

_LIT( KSemaphore,"Semaphore");

// FUNCTION PROTOTYPES
static TInt StartServer( const TDesC& aServerName , TUid aServerUid );
static TInt CreateServerProcess( const TDesC& aServerName , TUid aServerUid );

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
    //read the name and Uid of the search server
    TInt err = KErrNotReady;
    // get the watchdog repro 
    //TRAP_IGNORE is used to avoid the code scanner error
    CRepository* wdrepo = NULL;
    TRAP_IGNORE(wdrepo = CRepository::NewL( KWDrepoUidMenu ));
    if ( wdrepo )
        {
        HBufC* servername = NULL;
        TUid serveruid = {0};
        TBuf<KCenrepUidLength> temp;
        TInt64 value;
        TLex uidvalue;
        //read the searchserver UId
        if ( KErrNone == wdrepo->Get( KSearchServerUIDKey, temp ))
            {
            uidvalue.Assign(temp);
            if (KErrNone == uidvalue.Val( value,EHex ))
                serveruid.iUid = value;
            }
        //read the search server name
        if ( KErrNone == wdrepo->Get( KSearchServerNAMEKey, temp ))
            {
            //TRAP_IGNORE is used to avoid the code scanner error
            TRAP_IGNORE(servername = HBufC::NewL( temp.Length() ));
            TPtr ssname = servername->Des(); 
            ssname.Copy( temp );
            }
        // make sure the server is started before creating connection
        if ( servername )
           err = StartServer( *servername , serveruid );
    
        if ( KErrNone == err )
            {
            err = CreateSession(*servername, Version(), KDefaultMessageSlots);
            }
        delete servername;
        }
    delete wdrepo;
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
	if (iSizeList)
       {
       delete iSizeList;
       iSizeList = NULL;
       }
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

EXPORT_C void RSearchServerSubSession::SetQueryParserL(TInt aQueryParser)
	{
	TIpcArgs args(aQueryParser);

	// This call completes immediately, however the server will not
	// complete the request until later, so don't pass any local
	// descriptors as they will be out of scope by the time the server
	// attempts to read or write
	User::LeaveIfError( SendReceive(ESearchServerSetQueryParser, args) );
	}

EXPORT_C void RSearchServerSubSession::SetQueryParser(TInt aQueryParser, TRequestStatus& aStatus)
	{
	TIpcArgs args(aQueryParser);
	
	SendReceive(ESearchServerSetQueryParser, args, aStatus);
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
	OstTrace1( TRACE_NORMAL, RSEARCHSERVERSUBSESSION_SEARCHL, "RSearchServerSubSession::SearchL::sync;iEstimatedResultsCount=%d", iEstimatedResultsCount );
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

	// RSearchServerSubSession::GetBatchDocumentL()
EXPORT_C CSearchDocument** RSearchServerSubSession::GetBatchDocumentL(TInt aIndex, TInt& aReturnDoc, TInt aCount)
	{
	PERFORMANCE_LOG_START("RSearchServerSubSession::GetBatchDocumentL");
	
	if (!aCount) return NULL;
	
	iDocumentSize = 0;
	iReqCount = aCount;
	
	if (iSizeList)
	    {
	    delete iSizeList;
	    iSizeList = NULL;
	    }
	iSizeList = STATIC_CAST(TInt *, User::AllocZL(iReqCount * sizeof(TInt)));
    TPtr8 blob((TUint8*)iSizeList, iReqCount * sizeof(TInt));
	// Message arguments for the server
	TIpcArgs args(aIndex, iReqCount, &blob);
	iDocSizeArray.Reset();
	User::LeaveIfError( SendReceive(ESearchServerGetBatchDocument, args ) );	
	
	return GetBatchDocumentObjectL( aReturnDoc ); 
	}
	
EXPORT_C void RSearchServerSubSession::GetBatchDocument(TInt aIndex, TRequestStatus& aStatus, TInt aCount)
	{
	PERFORMANCE_LOG_START("RSearchServerSubSession::GetBatchDocument");

	iDocumentSize = 0;
	iReqCount = aCount;
	    
    if (iSizeList)
        {
        delete iSizeList;
        iSizeList = NULL;
        }
    TRAP_IGNORE(iSizeList = STATIC_CAST(TInt *, User::AllocZL(iReqCount * sizeof(TInt))));
    //iDocSizeArray
    TPtr8 blob((TUint8*)iSizeList, iReqCount * sizeof(TInt));
	    
    // Message arguments for the server
    TIpcArgs args(aIndex, aCount, &blob);
    
	SendReceive(ESearchServerGetBatchDocument, args, aStatus );
	}
	
EXPORT_C CSearchDocument** RSearchServerSubSession::GetBatchDocumentObjectL( TInt& aRetCount )
	{
	PERFORMANCE_LOG_START("CCPixSearcher::GetBatchDocumentObjectL");
	CSearchDocument** document = NULL;
	TInt i ,totalsize = 0;
	for ( i = 0; i< iReqCount; i++ )
        {
        if( *(iSizeList+i) )
            {
            totalsize += *(iSizeList+i);
            }
        else break;
        }   
	aRetCount = i;
	
	if ( aRetCount> 0 && totalsize )
	    {	    
	    //document = (CSearchDocument**)malloc ( sizeof(CSearchDocument*) * (i-1));
	    document = STATIC_CAST(CSearchDocument**, User::AllocL( aRetCount * sizeof(CSearchDocument*)));
	    HBufC8* buf = HBufC8::NewLC(totalsize +2 );
        TPtr8 ptr = buf->Des();
        User::LeaveIfError(SendReceive(ESearchServerGetBatchDocumentObject, TIpcArgs(&ptr)));	    
	    TInt startpos = 0;
	    TInt endpos = 0;
	    for ( TInt arrCount= 0; arrCount < aRetCount ; arrCount++)
                {
                endpos = *(iSizeList + arrCount) -4;
                //endpos = startpos + iDocSizeArray[arrCount];
                TPtrC8 tempptr = ptr.Mid( startpos , endpos );
                startpos += endpos;	        
                RDesReadStream stream;
                stream.Open(tempptr);
                stream.PushL();
                document[arrCount] = CSearchDocument::NewL(stream);
                CleanupStack::PopAndDestroy(&stream);
                }
        CleanupStack::PopAndDestroy(buf);	    
	    }		
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
static TInt StartServer( const TDesC& aServerName , TUid aServerUid )
	{
	TInt result;

	TFindServer findsearchServer(aServerName);
	TFullName name;

	result = findsearchServer.Next(name);
	if (result == KErrNone)
		{
		// Server already running
		return KErrNone;
		}

	RSemaphore semaphore;
	HBufC* semaphorename = HBufC16::NewL( aServerName.Length() + 10 );
    TPtr semaphoreptr = semaphorename->Des();
    semaphoreptr.Copy( aServerName );
    semaphoreptr.Append( KSemaphore );
    
	result = semaphore.CreateGlobal(*semaphorename, 0);
	
	delete semaphorename;
	
	if (result != KErrNone)
		{
		return result;
		}

	result = CreateServerProcess( aServerName,aServerUid );
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
static TInt CreateServerProcess( const TDesC& aServerName , TUid aServerUid )
	{
	TInt result;

	const TUidType serverUid( KNullUid, KNullUid, aServerUid);

	RProcess server;

	result = server.Create(aServerName, KNullDesC, serverUid);
	if (result != KErrNone)
		{
		return result;
		}

	server.Resume();
	server.Close();
	return KErrNone;
	}

// End of File

