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

#ifndef __CLIENTSERVERCOMMON_H__
#define __CLIENTSERVERCOMMON_H__

// INCLUDE FILES
#include <e32base.h>

/** @addtogroup Common */
/*\@{*/

/**
 * @file 
 * @ingroup Common
 * @brief Constants and codes associated with CPix client server communication
 */


// CONSTANTS
_LIT( KSearchServerName,"CPixSearchServer" ); // Server name
_LIT( KSearchServerSemaphoreName, "CPixSearchServerSemaphore" );

// The server version. A version must be specified when
// creating a session with the server.
const TUint KSearchServerMajorVersionNumber=1;
const TUint KSearchServerMinorVersionNumber=0;
const TUint KSearchServerBuildVersionNumber=0;

// DATA TYPES
/** 
 * @brief Opcodes used in message passing between client and server
 * 
 * CSearchServer is CPolicyServer and uses these values to define security policies
 * for messages. If these values are changed then CSearchServer security policy
 * values must be updated accordingly.
 */
enum TSearchServerRequest
{
	// Session related requests
	ESearchServerCreateSubSession = 1,
	ESearchServerCloseSession,
	
	// Sub-session related requests
	ESearchServerInitSubSession,
	ESearchServerCloseSubSession,
	ESearchServerOpenDatabase,
    ESearchServerSearch,
    ESearchServerGetDocument,
    ESearchServerGetDocumentObject,
    ESearchServerAdd,
    ESearchServerUpdate,
    ESearchServerDelete,
    ESearchServerReset,
    ESearchServerFlush,
    ESearchServerCancelAll,
    // Define the Index Database
    ESearchServerDatabaseDefine,
    // Undefine the Index Database
    ESearchServerDatabaseUnDefine,

    // Log player related commands
    ESearchServerStopHouseKeeping,
    ESearchServerContinueHouseKeeping,
    ESearchServerForceHouseKeeping,

    // Set analyzer. This must be latest message. Following messages
    // are not supported by server
    ESearchServerSetAnalyzer,
    ESearchServerSetQueryParser
};

/** 
 * @brief Opcodes used by server to indicate which asynchronous service has completed
 */
enum TSearchServerRqstComplete
{
    ESearchServerSearchComplete = 1
};

/*\@}*/ // group common

#endif // __CLIENTSERVERCOMMON_H__

// End of file

