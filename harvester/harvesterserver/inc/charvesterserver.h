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

#ifndef CHARVESTERSERVER_H
#define CHARVESTERSERVER_H

// INCLUDE FILES
#include <e32base.h>
#include "HarvesterServer.pan"

// FORWARD DECLARATIONS
class CIndexingManager;

// CLASS DECLARATION
/**
*  CHarvesterServer
*/
class CHarvesterServer : public CServer2
{
public : // Constructors and destructors

    /**
    * NewL.
    * Two-phased constructor.
    * @return Pointer to created CHarvesterServer object.
    */
    static CHarvesterServer* NewL();

    /**
    * NewLC.
    * Two-phased constructor.
    * @return Pointer to created CHarvesterServer object.
    */
    static CHarvesterServer* NewLC();

    /**
    * ~CHarvesterServer.
    * Destructor.
    */
    virtual ~CHarvesterServer();

public: // New functions

    /**
    * ThreadFunction.
    * Main function for the server thread.
    * @return Error code.
    */
    static void ThreadFunction();
    
    /**
     * IndexingManager
     * @return Current instance of CIndexingManager
     */
    CIndexingManager& IndexingManager() { return *iIndexingManager; }

private: // Constructors and destructors

    /**
    * CHarvesterServer.
    * C++ default constructor.
    * @param aPriority priority for this thread.
    */
    CHarvesterServer( TInt aPriority );

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
                             THarvesterServerPanic aReason );

    /**
    * PanicServer.
    * Panics the server.
    * @param aPanic The panic code.
    */
    static void PanicServer( THarvesterServerPanic aPanic );

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

private: // Data
    
	/*
	 * iGlue is Symbian wrapper for CPix OpenC search API.
	 */
	//CSearchGlue* iGlue;
	
    /**
     * iIndexingManager, the indexing manager object
     */
    CIndexingManager* iIndexingManager;
};

#endif // CHARVESTERSERVER_H

// End of File
