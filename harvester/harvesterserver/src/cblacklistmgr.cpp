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

#include "CBlacklistMgr.h"
#include <HarvesterServerLogger.h>

// -----------------------------------------------------------------------------
// CBlacklistMgr::NewL()
// -----------------------------------------------------------------------------
//
CBlacklistMgr* CBlacklistMgr::NewL()
    {
    CPIXLOGSTRING("CBlacklistMgr::NewL(): Entered");
    CBlacklistMgr* instance = CBlacklistMgr::NewLC();
    CleanupStack::Pop( instance );
    CPIXLOGSTRING("CBlacklistMgr::NewL(): Exit");
    return instance;
    }

// -----------------------------------------------------------------------------
// CBlacklistMgr::NewLC()
// -----------------------------------------------------------------------------
//
CBlacklistMgr* CBlacklistMgr::NewLC()
    {
    CBlacklistMgr* instance = new (ELeave) CBlacklistMgr();
    CleanupStack::PushL( instance );
    instance->ConstructL();
    return instance;
    }

// -----------------------------------------------------------------------------
// CBlacklistMgr::CBlacklistMgr()
// -----------------------------------------------------------------------------
//
CBlacklistMgr::CBlacklistMgr()
    {
    //Do the necessary initialisation
    }

// -----------------------------------------------------------------------------
// CBlacklistMgr::~CBlacklistMgr
// -----------------------------------------------------------------------------
//
CBlacklistMgr::~CBlacklistMgr()
    {
    delete iBlacklistDb;
    }

// -----------------------------------------------------------------------------
// CBlacklistMgr::ConstructL()
// -----------------------------------------------------------------------------
//
void CBlacklistMgr::ConstructL()
    {
    CPIXLOGSTRING("CBlacklistMgr::ConstructL(): Entered");    
    iBlacklistDb = CBlacklistDb::NewL();    
    CPIXLOGSTRING("CBlacklistMgr::ConstructL(): Exit");    
    }

// -----------------------------------------------------------------------------
// CBlacklistMgr::AddL()
// -----------------------------------------------------------------------------
//
TInt CBlacklistMgr::AddL( TUid aPluginUid , TInt aVersion )
    {
    //Add the item record to database
    CPIXLOGSTRING3("CBlacklistMgr::AddL(): Uid = %x and Version = %d" , aPluginUid.iUid , aVersion);
    //Check if the record with given plugin uid is already available in database or not
    //If available just update version number in the same record
    //If there is no record found in database with given uid, add new record with given details    
    TInt err = KErrNone;
    const TBool isfound = iBlacklistDb->FindL( aPluginUid.iUid );
    
    if (isfound)
        {
        err = iBlacklistDb->UpdateL( aPluginUid.iUid , aVersion );
        }
    else
        {
        err = iBlacklistDb->AddL( aPluginUid.iUid , aVersion );
        }
      
    CPIXLOGSTRING("CBlacklistMgr::AddL(): Exit");
    return err;
    }

// -----------------------------------------------------------------------------
// CBlacklistMgr::RemoveL()
// -----------------------------------------------------------------------------
//
void CBlacklistMgr::Remove( TUid aPluginUid )
    {
    CPIXLOGSTRING2("CBlacklistMgr::RemoveL(): Uid = %x " , aPluginUid.iUid );
    //Remove the item record to database
    iBlacklistDb->Remove( aPluginUid.iUid );
    
    CPIXLOGSTRING("CBlacklistMgr::RemoveL(): Exit");    
    }

// -----------------------------------------------------------------------------
// CBlacklistMgr::iSAvailableL()
// -----------------------------------------------------------------------------
//
TBool CBlacklistMgr::FindL( TUid aPluginUid , TInt aVersion )
    {
    CPIXLOGSTRING3("CBlacklistMgr::FindL(): Uid = %x and Version = %d" , aPluginUid.iUid , aVersion);
    //Check if the item is available in database
    TBool found = iBlacklistDb->FindWithVersionL( aPluginUid.iUid , aVersion );
    
    if(found)
        {
        CPIXLOGSTRING("UID is Black listed");
        }
    else
        {
        CPIXLOGSTRING("UID is not Black listed");
        }
    return found;
    }
