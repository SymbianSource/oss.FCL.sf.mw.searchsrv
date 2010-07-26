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

#include "SearchServerConfiguration.h"
#include "CSearchServer.h"
#include "SearchServerCommon.h"
#include "CSearchServerSession.h"
#include "CCPixIdxDb.h"
#include "CHouseKeepingHandler.h"
#include "SearchServerLogger.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "csearchserverTraces.h"
#endif


// Server security policy

// iRangeCount of TPolicy
const TUint KServerPolicyRangeCount = 6;

// Search command related element index
const TUint8 KSearchElementIndex = 0;

// Indexing command related element index
const TUint8 KIndexingElementIndex = 1;

// iRanges of TPolicy 
const TInt KServerPolicyRanges[KServerPolicyRangeCount] =
{
        0,                              // Subsession creation related messages
        ESearchServerSearch,            // Search related messages
        ESearchServerAdd,               // Indexing related messages
        ESearchServerCancelAll,         // Cancellation
        ESearchServerDatabaseDefine,    // Database define
        ESearchServerSetQueryParser+1      // Unsupported messages 
};

// iElementsIndex of TPolicy
const TUint8 KServerPolicyElementIndex[KServerPolicyRangeCount] =
{
        CPolicyServer::EAlwaysPass,
        KSearchElementIndex,
        KIndexingElementIndex,
        CPolicyServer::EAlwaysPass,
        KIndexingElementIndex,
        CPolicyServer::ENotSupported
};

// iElements of TPolicy
const CPolicyServer::TPolicyElement KServerPolicyElements[] =
{
        {_INIT_SECURITY_POLICY_C1(ECapabilityReadUserData), CPolicyServer::EFailClient },
        {_INIT_SECURITY_POLICY_C1(ECapabilityWriteUserData), CPolicyServer::EFailClient }
};

// TPolicy for policy server
const CPolicyServer::TPolicy KServerPolicy =
{
        CPolicyServer::EAlwaysPass, // Connects always pass
        KServerPolicyRangeCount,
        KServerPolicyRanges,
        KServerPolicyElementIndex,
        KServerPolicyElements
};


// ========================= MEMBER FUNCTIONS ==================================

// -----------------------------------------------------------------------------
// CSearchServer::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSearchServer* CSearchServer::NewL()
	{
	CSearchServer* searchServer = CSearchServer::NewLC();
	CleanupStack::Pop(searchServer);
	return searchServer;
	}

// -----------------------------------------------------------------------------
// CSearchServer::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSearchServer* CSearchServer::NewLC()
	{
	CSearchServer* searchServer = new ( ELeave ) CSearchServer( CActive::EPriorityStandard );
	CleanupStack::PushL(searchServer);
	searchServer->ConstructL();
	return searchServer;
	}

// -----------------------------------------------------------------------------
// CSearchServer::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSearchServer::ConstructL()
	{
	OstTraceFunctionEntry0( CSEARCHSERVER_CONSTRUCTL_ENTRY );
	
	CPIXLOGSTRING("Creating search server");
	RProcess process;
    process.SetPriority( EPriorityBackground );
    process.Close();
	// Create new container index
	iContainerIndex = CObjectConIx::NewL();
	
	// Create new house keeping handler
	iHouseKeepingHandler = CHouseKeepingHandler::NewL();
	iHouseKeepingHandler->StartL();
	
	// Initialize the OpenC search
	CCPixIdxDb::InitializeL();
	StartL(KSearchServerName);
	OstTraceFunctionExit0( CSEARCHSERVER_CONSTRUCTL_EXIT );
	}

// -----------------------------------------------------------------------------
// CSearchServer::CSearchServer()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CSearchServer::CSearchServer(TInt aPriority) :
	CPolicyServer(aPriority, KServerPolicy),
	iState( EUp ), 
	iSessionCount( 0 ) 
	{
	// Implementation not required
	}

// -----------------------------------------------------------------------------
// CSearchServer::~CSearchServer()
// Destructor.
// -----------------------------------------------------------------------------
//
CSearchServer::~CSearchServer()
	{
	OstTrace0( TRACE_NORMAL, CSEARCHSERVER_CSEARCHSERVER, "Deleting search server" );
	CPIXLOGSTRING("Deleting search server");
	
	// Delete the container index
	delete iContainerIndex;
	
	// Delete house keeping handler
	delete iHouseKeepingHandler;

	// Uninitialize the CPix API
	CCPixIdxDb::Shutdown();
	}

// -----------------------------------------------------------------------------
// CSearchServer::AddSessionL()
// -----------------------------------------------------------------------------
//
void CSearchServer::AddSession()
	{
	iSessionCount++; 
	}

// -----------------------------------------------------------------------------
// CSearchServer::RemoveSessionL()
// -----------------------------------------------------------------------------
//
void CSearchServer::RemoveSession()
	{
	iSessionCount--;
	if ( iState == EShuttingDown )
		{
		CActiveScheduler::Stop(); 
		}
	}

// -----------------------------------------------------------------------------
// CSearchServer::ShutDownL()
// -----------------------------------------------------------------------------
//
void CSearchServer::ShutDown()
	{
	iState = EShuttingDown; 
	if ( iSessionCount == 0 )
		{
		CActiveScheduler::Stop(); 
		}
	}

// -----------------------------------------------------------------------------
// CSearchServer::NewContainerL()
// Create new container index for sessions
// -----------------------------------------------------------------------------
//
CObjectCon* CSearchServer::NewContainerL()
{
	return iContainerIndex->CreateL();
}

// -----------------------------------------------------------------------------
// CSearchServer::NewContainerL()
// Create new container index for sessions
// -----------------------------------------------------------------------------
//
void CSearchServer::RemoveContainer(CObjectCon* aContainer)
{
	iContainerIndex->Remove( aContainer );
}

// -----------------------------------------------------------------------------
// CSearchServer::NewSessionL()
// Creates a time server session.
// -----------------------------------------------------------------------------
//
CSession2* CSearchServer::NewSessionL(const TVersion& aVersion,
		const RMessage2& /*aMessage*/) const
	{
	// Check we are the right version
	if ( !User::QueryVersionSupported(TVersion(KSearchServerMajorVersionNumber,
			KSearchServerMinorVersionNumber, KSearchServerBuildVersionNumber), aVersion) )
		{
		User::Leave(KErrNotSupported);
		}

	// Make new session
	return CSearchServerSession::NewL();
	}

// -----------------------------------------------------------------------------
// CSearchServer::StopHouseKeeping()
// Stop housekeeping
// -----------------------------------------------------------------------------
//
void CSearchServer::StopHouseKeeping()
    {
    iHouseKeepingHandler->StopHouseKeeping();
    }

// -----------------------------------------------------------------------------
// CSearchServer::ContinueHouseKeeping()
// Continue housekeeping
// -----------------------------------------------------------------------------
//
void CSearchServer::ContinueHouseKeeping()
    {
    iHouseKeepingHandler->ContinueHouseKeeping();
    }

// -----------------------------------------------------------------------------
// CSearchServer::ForceHouseKeeping()
// Force housekeeping
// -----------------------------------------------------------------------------
//
void CSearchServer::ForceHouseKeeping()
    {
    iHouseKeepingHandler->ForceHouseKeeping();
    }

// -----------------------------------------------------------------------------
// CSearchServer::PanicClient()
// Panics the client.
// -----------------------------------------------------------------------------
//
void CSearchServer::PanicClient(const RMessage2& aMessage,
		TSearchServerPanic aPanic)
	{
	aMessage.Panic(KSearchServer, aPanic);
	}

// -----------------------------------------------------------------------------
// CSearchServer::PanicServer()
// Panics the server.
// -----------------------------------------------------------------------------
//
void CSearchServer::PanicServer(TSearchServerPanic aPanic)
	{
	User::Panic(KSearchServer, aPanic);
	}

// -----------------------------------------------------------------------------
// CSearchServer::ThreadFunctionL()
// Second stage startup for the server thread.
// -----------------------------------------------------------------------------
//
void CSearchServer::ThreadFunctionL()
	{
	// Construct active scheduler
	CActiveScheduler* activeScheduler = new ( ELeave ) CActiveScheduler;
	CleanupStack::PushL(activeScheduler);

	// Install active scheduler
	CActiveScheduler::Install(activeScheduler);

	// Construct server
	CSearchServer* server = CSearchServer::NewLC();
	
    // Rename the thread.
    User::RenameThread(KSearchServerName);        
    RProcess::Rendezvous(KErrNone);           

	// Signal client thread if started by such
	RSemaphore semaphore;
	TInt err = semaphore.OpenGlobal(KSearchServerSemaphoreName);
	if (KErrNone == err)
		{
		// Started by client
		semaphore.Signal();
		semaphore.Close();
		}
	else
		{
		if (KErrNotFound == err)
			{
			// Started by S60 starter
			}
		else
			{
			// Unknown error, leave
			User::Leave(err);
			}
		}

	// Start handling requests
	CActiveScheduler::Start();
	
	// Cleanup
	CleanupStack::PopAndDestroy( server );
	CleanupStack::PopAndDestroy( activeScheduler );
	}

// -----------------------------------------------------------------------------
// CSearchServer::ThreadFunction()
// Main function for the server thread.
// -----------------------------------------------------------------------------
//
void CSearchServer::ThreadFunction()
	{
    __UHEAP_MARK;
	CTrapCleanup* cleanupStack = CTrapCleanup::New();
	
	if ( !cleanupStack )
		{
		PanicServer(ECreateTrapCleanup);
		}

	TRAPD( err, ThreadFunctionL() );
	if ( err )
		{
		PanicServer(ESrvCreateServer);
		}

	delete cleanupStack;
    __UHEAP_MARKEND;    
	}

// -----------------------------------------------------------------------------
// E32Main()
// Provides the API for the operating system to start the executable.
// Returns the address of the function to be called.
// -----------------------------------------------------------------------------
//
TInt E32Main()
	{
	CSearchServer::ThreadFunction();
	return KErrNone;
	}

// End of File
