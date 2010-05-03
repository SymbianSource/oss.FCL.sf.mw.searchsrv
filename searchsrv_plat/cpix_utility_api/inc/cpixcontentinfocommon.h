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
* Description:  This component is to store the details of harvester plugins
*
*/

#ifndef CPIXCONTENTINFOCOMMON_H_
#define CPIXCONTENTINFOCOMMON_H_

//ContentInfo DB path
_LIT( KCIDBPath , "c:\\Private\\2001f6fb" );
//Content Info DB filename
_LIT(KContentInfoFileName,"cpixcontentinfo.sq");

//Table name in ContentInfo database
_LIT( KContentInfoTableName , "table1" );

//Max length for plugin display name
const TInt KMaxPluginNameLength(20);

//column name for content name column
_LIT( KContentName , "NAME" );

//column name for Indexing status column
_LIT( KIndexingStatus , "INS" );

//column name for Blacklist status column
_LIT( KBlacklistStatus , "BLS" );

//Primary key
_LIT( KPrimaryKey , "NAME" );

#endif /* CPIXCONTENTINFOCOMMON_H_ */
