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

#ifndef HARVESTERSERVER_PAN
#define HARVESTERSERVER_PAN

/** Panic Category */
_LIT( KHarvesterServer, "CHarvesterServer" );

/** HarvesterServer panic codes */
enum THarvesterServerPanic
    {
    EBadRequest = 1,
    EBadDescriptor = 2,
    ESrvCreateServer = 3,
    EMainSchedulerError = 4,
    ECreateTrapCleanup = 5,
    ESrvSessCreateTimer = 6,
    EReqAlreadyPending = 7              // A request is already pending
    };

#endif // HARVESTERSERVER_PAN

// End of File
