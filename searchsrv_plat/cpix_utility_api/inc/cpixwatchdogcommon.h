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
/**
 * @file
 * @ingroup Common
 * @brief Provides watchdog common definitions which mentions the keys used for configured harvester and search servers. 
 * The central repository key for search UI application is also covered here. 
 */

#ifndef CPIXWATCHDOGCOMMON_H_
#define CPIXWATCHDOGCOMMON_H_

//Length of uid string in cenrep
const TInt KCenrepUidLength = 256;
//Uid of watchdog cetral repository database
const TUid KWDrepoUidMenu = {0x20029ab8};
const TUint32 KHarvesterServerKey = 0x1;
const TUint32 KSearchServerKey = 0x2;
const TUint32 KHarvesterServerUIDKey = 0x3;
const TUint32 KHarvesterServerNAMEKey = 0x4;
const TUint32 KSearchServerUIDKey = 0x5;
const TUint32 KSearchServerNAMEKey = 0x6;
const TUint32 KSearchUiUIDKey = 0x7;


#endif /*CPIXWATCHDOGCOMMON_H_*/
