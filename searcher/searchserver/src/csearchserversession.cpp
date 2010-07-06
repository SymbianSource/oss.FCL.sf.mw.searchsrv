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

#include "CSearchServerSession.h"
#include "CSearchServerSubSession.h"
#include "SearchServerCommon.h"
#include "CSearchServer.h"
#include "SearchServerHelper.h"
#include "Common.h"
#include "SearchServerTesting.h"
#include "SearchServerLogger.h"
#include "cpixidxdb.h" // cpix_IdxDb_undefineVolume
#include "CLogPlayerRecorder.h"

#include "indevicecfg.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "csearchserversessionTraces.h"
#endif



// ========================= MEMBER FUNCTIONS ==================================

// -----------------------------------------------------------------------------
// CSearchServerSession::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSearchServerSession* CSearchServerSession::NewL()
    {
    CSearchServerSession* self = CSearchServerSession::NewLC();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CSearchServerSession::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSearchServerSession* CSearchServerSession::NewLC()
    {
    CSearchServerSession* self = new (ELeave) CSearchServerSession();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CSearchServerSession::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSearchServerSession::ConstructL()
    {
    }

// -----------------------------------------------------------------------------
// CSearchServerSession::CSearchServerSession()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CSearchServerSession::CSearchServerSession()
    {
        OstTrace0( TRACE_NORMAL, CSEARCHSERVERSESSION_CSEARCHSERVERSESSION, "Creating search server session" );
        CPIXLOGSTRING("Creating search server session");
    }

// -----------------------------------------------------------------------------
// CSearchServerSession::~CSearchServerSession()
// Destructor.
// -----------------------------------------------------------------------------
//
CSearchServerSession::~CSearchServerSession()
    {
        OstTrace0( TRACE_NORMAL, DUP1_CSEARCHSERVERSESSION_CSEARCHSERVERSESSION, "Deleting search server session" );
        CPIXLOGSTRING("Deleting search server session");
    }

// -----------------------------------------------------------------------------
// CSearchServerSession::CreateL()
// Called by client/server framework after session has been successfully created
// -----------------------------------------------------------------------------
//
void CSearchServerSession::CreateL()
    {
    // Create new object index
    iCountersObjectIndex = CObjectIx::NewL();

    // Initialize the object container
    // using the object container index in the server.
    iContainer = ((CSearchServer*) Server())->NewContainerL();

    ((CSearchServer*) Server())->AddSession();
    }

// -----------------------------------------------------------------------------
// CSearchServerSession::SubSessionFromHandle()
// Utility to return the CSearchServerSubSession (subsession) object
// -----------------------------------------------------------------------------
//
CSearchServerSubSession* CSearchServerSession::SubSessionFromHandle(
        const RMessage2& aMessage, TInt aHandle)
    {
    CSearchServerSubSession* subsession =
            (CSearchServerSubSession*) iCountersObjectIndex->At(aHandle);
    if (subsession == NULL)
        {
        PanicClient(aMessage, EBadSubsessionHandle);
        }
    return subsession;
    }

// -----------------------------------------------------------------------------
// CSearchServerSession::ServiceL()
// Service request from client.
// -----------------------------------------------------------------------------
//
void CSearchServerSession::ServiceL(const RMessage2& aMessage)
    {
    // First check for session-relative requests
    switch (aMessage.Function())
        {
        case ESearchServerCreateSubSession:
            {
            NewSubSessionL(aMessage);
            return;
            }
        case ESearchServerCloseSession:
            {
            CloseSession(aMessage);
            return;
            }
        case ESearchServerDatabaseDefine:
            {
            DefineVolumeL(aMessage);
            return;
            }
        case ESearchServerDatabaseUnDefine:
            {
            UnDefineVolumeL(aMessage);
            return;
            }
        case ESearchServerStopHouseKeeping:
            {
            StopHouseKeeping(aMessage);
            return;
            }
        case ESearchServerContinueHouseKeeping:
            {
            ContinueHouseKeeping(aMessage);
            return;
            }
        case ESearchServerForceHouseKeeping:
            {
            ForceHouseKeeping(aMessage);
            return;
            }
            // Don't panic in default case!
        }

    // All other function codes must be subsession relative.
    // We need to find the appropriate server side subsession
    // i.e. the CCountSubSession object. 
    // The handle value is passed as the 4th aregument.
    CSearchServerSubSession* subsession = SubSessionFromHandle(aMessage,
            aMessage.Int3());
    switch (aMessage.Function())
        {
        case ESearchServerInitSubSession:
            aMessage.Complete(KErrNone);
            break;

        case ESearchServerCloseSubSession:
            DeleteCounter(aMessage.Int3());
            aMessage.Complete(KErrNone);
            break;

        case ESearchServerOpenDatabase:
            RECORDED_EXECUTION_BEGIN
            subsession->OpenDatabaseL(aMessage);
            RECORDED_EXECUTION_END("openDatabase")
            break;

        case ESearchServerSetAnalyzer:
            RECORDED_EXECUTION_BEGIN
            subsession->SetAnalyzerL(aMessage);
            RECORDED_EXECUTION_END("setAnalyzer")
            break;

        case ESearchServerSetQueryParser:
            RECORDED_EXECUTION_BEGIN
            subsession->SetQueryParserL(aMessage);
            RECORDED_EXECUTION_END("setQueryParser")
            break;

        case ESearchServerSearch:
            RECORDED_EXECUTION_BEGIN
            subsession->SearchL(aMessage);
            RECORDED_EXECUTION_END("search");
            break;

        case ESearchServerGetDocument:
            RECORDED_EXECUTION_BEGIN
            subsession->GetDocumentL(aMessage);
            RECORDED_EXECUTION_END("getDocument")
            break;

        case ESearchServerGetDocumentObject:
            RECORDED_EXECUTION_BEGIN
            subsession->GetDocumentObjectL(aMessage);
            RECORDED_EXECUTION_END("getDocumentObject")
            break;

        case ESearchServerCancelAll:
            RECORDED_EXECUTION_BEGIN
            subsession->CancelAll(aMessage);
            RECORDED_EXECUTION_END("cancelAll")
            break;

        case ESearchServerAdd:
            RECORDED_EXECUTION_BEGIN
            subsession->AddL(aMessage);
            RECORDED_EXECUTION_END("add")
            break;

        case ESearchServerUpdate:
            RECORDED_EXECUTION_BEGIN
            subsession->UpdateL(aMessage);
            RECORDED_EXECUTION_END("update")
            break;

        case ESearchServerDelete:
            RECORDED_EXECUTION_BEGIN
            subsession->DeleteL(aMessage);
            RECORDED_EXECUTION_END("delete")
            break;

        case ESearchServerReset:
            RECORDED_EXECUTION_BEGIN
            subsession->ResetL(aMessage);
            RECORDED_EXECUTION_END("reset")
            break;

        case ESearchServerFlush:
            RECORDED_EXECUTION_BEGIN
            subsession->FlushL(aMessage);
            RECORDED_EXECUTION_END("flush")
            break;

        default:
            {
            PanicClient(aMessage, EBadRequest);
            aMessage.Complete(KErrNone);
            }
        }

    }

// -----------------------------------------------------------------------------
// CSearchServerSession::NewSubSessionL()
// Creates new subsession
// -----------------------------------------------------------------------------
//
void CSearchServerSession::NewSubSessionL(const RMessage2& aMessage)
    {
    // make a new counter object
    CSearchServerSubSession* subsession = CSearchServerSubSession::NewL(this);

    // add the CCountSubSession object to 
    // this subsession's object container
    // to gererate a unique id
    iContainer->AddL(subsession);

    // Add the object to object index; this returns
    // a unique handle so that we can find the object
    // again laterit later.
    TInt handle = iCountersObjectIndex->AddL(subsession);

    // Write the handle value back to the client.
    // NB It's not obvious but the handle value must be passed
    // back as the 4th parameter (i.e. parameter number 3 on
    // a scale of 0 to 3). 
    // The arguments that are passed across are actually
    // set up by RSubSessionBase::DoCreateSubSession().
    // If you pass your own arguments into a call
    // to RSubSessionBase::CreateSubSession(), which calls DoCreateSubSession, 
    // then only the first three are picked up - the 4th is reserved for the
    // the subsession handle.
    TPckgBuf<TInt> handlePckg(handle);
    TRAPD(res,aMessage.WriteL(3,handlePckg));
    if (res != KErrNone)
        {
        iCountersObjectIndex->Remove(handle);
        PanicClient(aMessage, EBadDescriptor);
        return;
        }

    // We now have another "resource"
    iResourceCount++;

    // Complete the request
    aMessage.Complete(KErrNone);
    }

// -----------------------------------------------------------------------------
// CSearchServerSession::CloseSession()
// Called by client/server framework to close a session
// -----------------------------------------------------------------------------
//
void CSearchServerSession::CloseSession(const RMessage2& aMessage)
    {
    ((CSearchServer*) Server())->RemoveSession();

    // Deletes the object index.
    if (iCountersObjectIndex)
        {
        delete iCountersObjectIndex;
        iCountersObjectIndex = NULL;
        }

    ((CSearchServer*) Server())->RemoveContainer(iContainer);

    // Complete the request
    aMessage.Complete(KErrNone);
    }

// -----------------------------------------------------------------------------
// CSearchServerSession::DefineVolumeL()
// Defines a specific volume, with an optional path
// -----------------------------------------------------------------------------
//
void CSearchServerSession::DefineVolumeL(const RMessage2& aMessage)
    {
    HBufC* qBaseAppClass = HBufC::NewLC(aMessage.GetDesLength(0));
    TPtr qBaseAppClassPtr = qBaseAppClass->Des();
    aMessage.ReadL(0, qBaseAppClassPtr);

    HBufC* path = HBufC::NewLC(aMessage.GetDesLength(1));
    TPtr pathPtr = path->Des();
    aMessage.ReadL(1, pathPtr);

    DefineVolumeL(*qBaseAppClass, *path);

    CleanupStack::PopAndDestroy(path);
    CleanupStack::PopAndDestroy(qBaseAppClass);

    // Complete the request
    aMessage.Complete(KErrNone);
    }

// -----------------------------------------------------------------------------
// CSearchServerSession::DefineVolumeL()
// Defines a specific volume, with an optional path
// -----------------------------------------------------------------------------
//
void CSearchServerSession::DefineVolumeL(const TDesC& aQualifiedBaseAppClass,
        const TDesC& aIndexDbPath)
    {
    LOG_PLAYER_RECORD( CLogPlayerRecorder::LogDefineVolumeL(aQualifiedBaseAppClass, aIndexDbPath) );

    HBufC8* qualBaseAppClass = HBufC8::NewLC(aQualifiedBaseAppClass.Size()
            + 1);
    TPtr8 qualBaseAppClassPtr = qualBaseAppClass->Des();
    qualBaseAppClassPtr.Copy(aQualifiedBaseAppClass);

    HBufC8* path = HBufC8::NewLC(aIndexDbPath.Size() + 1);
    TPtr8 pathPtr = path->Des();
    pathPtr.Copy(aIndexDbPath);

    cpix_Result result;
    cpix_IdxDb_defineVolume(&result,
            reinterpret_cast<const char*> (qualBaseAppClassPtr.PtrZ()),
            reinterpret_cast<const char*> (pathPtr.PtrZ()));

    SearchServerHelper::CheckCpixErrorL(&result, KErrCannotDefineVolume);

    CleanupStack::PopAndDestroy(path);
    CleanupStack::PopAndDestroy(qualBaseAppClass);
    }

// -----------------------------------------------------------------------------
// CSearchServerSession::UnDefineVolumeL()
// Undefines a specific volume.
// -----------------------------------------------------------------------------
//
void CSearchServerSession::UnDefineVolumeL(const RMessage2& aMessage)
    {
    HBufC* qBaseAppClass = HBufC::NewLC(aMessage.GetDesLength(0));
    TPtr qBaseAppClassPtr = qBaseAppClass->Des();
    aMessage.ReadL(0, qBaseAppClassPtr);

    UnDefineVolumeL(*qBaseAppClass);

    CleanupStack::PopAndDestroy(qBaseAppClass);

    // Complete the request
    aMessage.Complete(KErrNone);
    }

// -----------------------------------------------------------------------------
// CSearchServerSession::StopHouseKeeping()
// Stop housekeeping
// -----------------------------------------------------------------------------
//
void CSearchServerSession::StopHouseKeeping(const RMessage2& aMessage)
    {
    CServer2* server = const_cast<CServer2*>( Server() );
    static_cast<CSearchServer*>( server )->StopHouseKeeping();
    
    // Complete the request
    aMessage.Complete(KErrNone);
    }

// -----------------------------------------------------------------------------
// CSearchServerSession::ContinueHouseKeeping()
// Continue housekeeping
// -----------------------------------------------------------------------------
//
void CSearchServerSession::ContinueHouseKeeping(const RMessage2& aMessage)
    {
    CServer2* server = const_cast<CServer2*>( Server() );
    static_cast<CSearchServer*>( server )->ContinueHouseKeeping();
    
    // Complete the request
    aMessage.Complete(KErrNone);
    }

// -----------------------------------------------------------------------------
// CSearchServerSession::ForceHouseKeeping()
// Force housekeeping
// -----------------------------------------------------------------------------
//
void CSearchServerSession::ForceHouseKeeping(const RMessage2& aMessage)
    {
    CServer2* server = const_cast<CServer2*>( Server() );
    static_cast<CSearchServer*>( server )->ForceHouseKeeping();
    
    // Complete the request
    aMessage.Complete(KErrNone);
    }

// -----------------------------------------------------------------------------
// CSearchServerSession::UnDefineVolumeL()
// Undefines a specific volume.
// -----------------------------------------------------------------------------
//
void CSearchServerSession::UnDefineVolumeL(const TDesC& aBaseAppClass)
    {
    HBufC8* baseAppClass = HBufC8::NewLC(aBaseAppClass.Size() + 1);
    TPtr8 baseAppClassPtr = baseAppClass->Des();
    baseAppClassPtr.Copy(aBaseAppClass);

    cpix_IdxDb_undefineVolume(
            reinterpret_cast<const char*> (baseAppClassPtr.PtrZ()));

    CleanupStack::PopAndDestroy(baseAppClass);
    }

// -----------------------------------------------------------------------------
// CSearchServerSession::NewSubSessionL()
// Delete the subsession object through its handle
// -----------------------------------------------------------------------------
//
void CSearchServerSession::DeleteCounter(TInt aHandle)
    {
    // This will delete the CCountSubSession object; the object is
    // reference counted, and removing the handle causes the object to be closed
    // [closing reduces the access count - the object is deleted if the access
    //  count reaches zero etc].
    iCountersObjectIndex->Remove(aHandle);
    // decrement resource count
    iResourceCount--;
    }

// Utility function to panic client
void CSearchServerSession::PanicClient(const RMessage2& aMessage, TInt aPanic) const
    {
    aMessage.Panic(KSearchServer, aPanic); // Note: this panics the client thread, not server
    }

// End of File

