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

#ifndef __CSEARCHSERVER_H__
#define __CSEARCHSERVER_H__

// INCLUDE FILES
#include <e32base.h>
#include "SearchServer.pan"

class CHouseKeepingHandler;

// CLASS DECLARATION
/**
*  CSearchServer
*/
class CSearchServer : public CPolicyServer
{
public : // Constructors and destructors

    /**
    * NewL.
    * Two-phased constructor.
    * @return Pointer to created CSearchServer object.
    */
    static CSearchServer* NewL();

    /**
    * NewLC.
    * Two-phased constructor.
    * @return Pointer to created CSearchServer object.
    */
    static CSearchServer* NewLC();

    /**
    * ~CSearchServer.
    * Destructor.
    */
    virtual ~CSearchServer();

public: // New functions

    /**
    * ThreadFunction.
    * Main function for the server thread.
    * @return Error code.
    */
    static void ThreadFunction();
    
	/**
	 * NewContainerL.
	 * Returns an object container, and guaranteed 
	 * to produce object containers with unique
	 * ids within the server. 	 
	 * Called by a new session to create a container
	 * Containers created by this method MUST be release 
	 * with RemoveContainer method. 
	 */ 
	CObjectCon* NewContainerL();
	
	/**
	 * RemoveContainer.
	 * Removes a container created by NewContainerL
	 */ 
	void RemoveContainer(CObjectCon* aContainer);
		
	/**
	 * HandleHeartBeatL.
	 * From MHeartBeatObserver
	 * Called by the CHeartBeatTimer
	 */
	void HandleHeartBeatL();
	
	/**
	 * Stop housekeeping
	 */
	void StopHouseKeeping();

	/**
	 * Continue housekeeping
	 */
	void ContinueHouseKeeping();

	/**
	 * Force housekeeping
	 */
	void ForceHouseKeeping();
 
public: // For CSearchServerSession 

	/**
	 * Increments session count by one
	 */
	void AddSession(); 
	
	/**
	 * Reduces session count by one. May cause the server 
	 * shutdown, if server is preparing for one. 
	 */
	void RemoveSession(); 

	/**
	 * Starts shutting down
	 */
	void ShutDown(); 

private: // Constructors and destructors

    /**
    * CSearchServer.
    * C++ default constructor.
    * @param aPriority priority for this thread.
    */
    CSearchServer( TInt aPriority );

    /**
    * ConstructL.
    * 2nd phase constructor.
    */
    void ConstructL();

private: // New methods

    /**
    * PanicClient.
    * Panics the client.
    * @param aMessage The message channel to the client.
    * @param aReason The reason code for the panic.
    */
    static void PanicClient( const RMessage2& aMessage,
                             TSearchServerPanic aReason );

    /**
    * PanicServer.
    * Panics the server.
    * @param aPanic The panic code.
    */
    static void PanicServer( TSearchServerPanic aPanic );

    /**
    * ThreadFunctionL.
    * Second stage startup for the server thread.
    */
    static void ThreadFunctionL();

private: // Functions from base classes

    /**
    * From CServer, NewSessionL.
    * Creates a time server session.
    * @param aVersion The client version.
    * @param aMessage Message from client.
    * @return Pointer to new session.
    */
    CSession2* NewSessionL(const TVersion& aVersion, const RMessage2& aMessage) const;

private: // Internal state declaration
	
	enum TState 
		{
		EUp, 
		EShuttingDown
		};
    
private: // Data
	
	// The server has an object container index that
	// creates an object container for each session.
	CObjectConIx* iContainerIndex; 
	
	TState iState; 
	
	TInt iSessionCount; 
	
	// House keeping handler
	CHouseKeepingHandler* iHouseKeepingHandler;
};

#endif // __CSEARCHSERVER_H__

// End of File
