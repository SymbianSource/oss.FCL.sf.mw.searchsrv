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

#include "CHarvesterServer.h"
#include "CHarvesterServerSession.h"
#include "CIndexingManager.h"
#include "HarvesterServerCommons.h"

// ========================= MEMBER FUNCTIONS ==================================

// -----------------------------------------------------------------------------
// CHarvesterServer::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CHarvesterServer* CHarvesterServer::NewL()
	{
	CHarvesterServer* HarvesterServer = CHarvesterServer::NewLC();
	CleanupStack::Pop(HarvesterServer);
	return HarvesterServer;
	}

// -----------------------------------------------------------------------------
// CHarvesterServer::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CHarvesterServer* CHarvesterServer::NewLC()
	{
	CHarvesterServer* HarvesterServer = new ( ELeave ) CHarvesterServer( EPriorityNormal );
	CleanupStack::PushL(HarvesterServer);
	HarvesterServer->ConstructL();
	return HarvesterServer;
	}

// -----------------------------------------------------------------------------
// CHarvesterServer::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CHarvesterServer::ConstructL()
	{
	iIndexingManager = CIndexingManager::NewL();
	StartL(KHarvesterServerName);
	}

// -----------------------------------------------------------------------------
// CHarvesterServer::CHarvesterServer()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CHarvesterServer::CHarvesterServer(TInt aPriority) :
	CServer2(aPriority)
	{
	// Implementation not required
	}

// -----------------------------------------------------------------------------
// CHarvesterServer::~CHarvesterServer()
// Destructor.
// -----------------------------------------------------------------------------
//
CHarvesterServer::~CHarvesterServer()
	{
	delete iIndexingManager;
	}

// -----------------------------------------------------------------------------
// CHarvesterServer::NewSessionL()
// Creates a time server session.
// -----------------------------------------------------------------------------
//
CSession2* CHarvesterServer::NewSessionL(const TVersion& aVersion,
		const RMessage2& /*aMessage*/) const
	{
	// Check we are the right version
	if ( !User::QueryVersionSupported(TVersion(KHarvesterServerMajorVersionNumber,
			KHarvesterServerMinorVersionNumber, KHarvesterServerBuildVersionNumber), aVersion) )
		{
		User::Leave(KErrNotSupported);
		}

	// Make new session
	return CHarvesterServerSession::NewL(/**iGlue*/);
	}

// -----------------------------------------------------------------------------
// CHarvesterServer::PanicClient()
// Panics the client.
// -----------------------------------------------------------------------------
//
void CHarvesterServer::PanicClient(const RMessage2& aMessage,
		THarvesterServerPanic aPanic)
	{
	aMessage.Panic(KHarvesterServer, aPanic);
	}

// -----------------------------------------------------------------------------
// CHarvesterServer::PanicServer()
// Panics the server.
// -----------------------------------------------------------------------------
//
void CHarvesterServer::PanicServer(THarvesterServerPanic aPanic)
	{
	User::Panic(KHarvesterServer, aPanic);
	}

// -----------------------------------------------------------------------------
// CHarvesterServer::ThreadFunctionL()
// Second stage startup for the server thread.
// -----------------------------------------------------------------------------
//
void CHarvesterServer::ThreadFunctionL()
	{
    //QCoreApplication installs one ActiveScheduler.  So removing the Active scheduler install
    //from ThreadFunctionL
    
	// Construct active scheduler
	//CActiveScheduler* activeScheduler = new ( ELeave ) CActiveScheduler;
	//CleanupStack::PushL(activeScheduler);

	// Install active scheduler
	//CActiveScheduler::Install(activeScheduler);	
		
	// Construct server
	CHarvesterServer* server = CHarvesterServer::NewLC();
	
	// Set thread priority
	RProcess().SetPriority(EPriorityBackground);
	
    // Rename the thread.
    User::RenameThread(KHarvesterServerName);        
    RProcess::Rendezvous(KErrNone);           

	// Signal client thread if started by such
	RSemaphore semaphore;
	TInt err = semaphore.OpenGlobal(KHarvesterServerSemaphoreName);
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
	//CleanupStack::PopAndDestroy( activeScheduler );
	}

// -----------------------------------------------------------------------------
// CHarvesterServer::ThreadFunction()
// Main function for the server thread.
// -----------------------------------------------------------------------------
//
void CHarvesterServer::ThreadFunction()
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
//TInt E32Main()
//	{
//	CHarvesterServer::ThreadFunction();
//	return KErrNone;
//	}

// End of File
