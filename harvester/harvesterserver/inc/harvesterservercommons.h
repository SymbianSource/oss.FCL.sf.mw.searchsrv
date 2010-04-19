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

#ifndef HARVESTERSERVERCOMMONS_H
#define HARVESTERSERVERCOMMONS_H

// INCLUDE FILES
#include <e32base.h>

// CONSTANTS
_LIT( KHarvesterServerName,"CPixHarvesterServer" ); // Server name
_LIT( KHarvesterServerSemaphoreName, "CPixHarvesterServerSemaphore" );

// The server version. A version must be specified when
// creating a session with the server.
const TUint KHarvesterServerMajorVersionNumber=1;
const TUint KHarvesterServerMinorVersionNumber=0;
const TUint KHarvesterServerBuildVersionNumber=0;

// DATA TYPES
/** 
 * @brief Opcodes used in message passing between client and server
 */
enum THarvesterServerRequest
{
	EHarvesterServerGetHarvesterCount = 1,
    EHarvesterServerGetHarvesterStatus,
};

#endif // HARVESTERSERVERCOMMONS_H
// End of file
