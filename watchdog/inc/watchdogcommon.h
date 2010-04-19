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


#ifndef WATCHDOGCOMMON_H
#define WATCHDOGCOMMON_H

// INCLUDE FILES
#include <e32base.h>

#define MONITORING_DELAY 60000000 // Nano seconds to delay the monitored object

// CONSTANTS
_LIT(KHarvesterServer,"CPixHarvesterServer");
_LIT(KSearchServer,"CPixSearchServer");
_LIT( KSearchServerSemaphoreName, "CPixSearchServerSemaphore" );
_LIT( KHarvesterServerSemaphoreName, "CPixHarvesterServerSemaphore" );

const TUid KHServerUid3 = { 0x2001F6FB };
const TUid KSServerUid3 = { 0x2001F6F7 };

// DATA TYPES


#endif // WATCHDOGCOMMON_H
// End of file
