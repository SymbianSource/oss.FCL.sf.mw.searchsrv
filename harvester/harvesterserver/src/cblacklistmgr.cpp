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
#include "CBlacklistDb.h"
#include <HarvesterServerLogger.h>
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cblacklistmgrTraces.h"
#endif


// -----------------------------------------------------------------------------
// CBlacklistMgr::NewL()
// -----------------------------------------------------------------------------
//
CBlacklistMgr* CBlacklistMgr::NewL()
    {
    OstTraceFunctionEntry0( CBLACKLISTMGR_NEWL_ENTRY );
    CPIXLOGSTRING("CBlacklistMgr::NewL(): Entered");
    CBlacklistMgr* instance = CBlacklistMgr::NewLC();
    CleanupStack::Pop( instance );
    CPIXLOGSTRING("CBlacklistMgr::NewL(): Exit");
    OstTraceFunctionExit0( CBLACKLISTMGR_NEWL_EXIT );
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
    OstTraceFunctionEntry0( CBLACKLISTMGR_CONSTRUCTL_ENTRY );
    CPIXLOGSTRING("CBlacklistMgr::ConstructL(): Entered");    
    iBlacklistDb = CBlacklistDb::NewL();    
    CPIXLOGSTRING("CBlacklistMgr::ConstructL(): Exit");    
    OstTraceFunctionExit0( CBLACKLISTMGR_CONSTRUCTL_EXIT );
    }

// -----------------------------------------------------------------------------
// CBlacklistMgr::AddL()
// -----------------------------------------------------------------------------
//
TInt CBlacklistMgr::AddL( TUid aPluginUid , TInt aVersion )
    {
    OstTraceFunctionEntry0( CBLACKLISTMGR_ADDL_ENTRY );
    //Add the item record to database
    OstTraceExt2( TRACE_NORMAL, CBLACKLISTMGR_ADDL, "CBlacklistMgr::AddL;Uid=%x;Version=%d", aPluginUid.iUid, aVersion );
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
    OstTraceFunctionExit0( CBLACKLISTMGR_ADDL_EXIT );
    return err;
    }

// -----------------------------------------------------------------------------
// CBlacklistMgr::RemoveL()
// -----------------------------------------------------------------------------
//
void CBlacklistMgr::Remove( TUid aPluginUid )
    {
    OstTraceFunctionEntry0( CBLACKLISTMGR_REMOVE_ENTRY );
    OstTrace1( TRACE_NORMAL, CBLACKLISTMGR_REMOVE, "CBlacklistMgr::Remove;Uid=%x", aPluginUid.iUid );
    CPIXLOGSTRING2("CBlacklistMgr::RemoveL(): Uid = %x " , aPluginUid.iUid );
    //Remove the item record to database
    iBlacklistDb->Remove( aPluginUid.iUid );
    
    CPIXLOGSTRING("CBlacklistMgr::RemoveL(): Exit");    
    OstTraceFunctionExit0( CBLACKLISTMGR_REMOVE_EXIT );
    }

// -----------------------------------------------------------------------------
// CBlacklistMgr::FindL()
// -----------------------------------------------------------------------------
//
TBool CBlacklistMgr::FindL( TUid aPluginUid , TInt aVersion )
    {
    OstTraceExt2( TRACE_NORMAL, CBLACKLISTMGR_FINDL, "CBlacklistMgr::FindL;Uid=%x;Version=%d", aPluginUid.iUid, aVersion );
    CPIXLOGSTRING3("CBlacklistMgr::FindL(): Uid = %x and Version = %d" , aPluginUid.iUid , aVersion);
    //Check if the item is available in database
    TBool found = iBlacklistDb->FindWithVersionL( aPluginUid.iUid , aVersion );
    
    if(found)
        {
        OstTrace0( TRACE_NORMAL, DUP1_CBLACKLISTMGR_FINDL, "UID is Black listed" );
        CPIXLOGSTRING("UID is Black listed");
        }
    else
        {
        OstTrace0( TRACE_NORMAL, DUP2_CBLACKLISTMGR_FINDL, "UID is not Black listed" );
        CPIXLOGSTRING("UID is not Black listed");
        }
    return found;
    }

// -----------------------------------------------------------------------------
// CBlacklistMgr::AddtoDontloadListL()
// -----------------------------------------------------------------------------
//
TInt CBlacklistMgr::AddtoDontloadListL( TUid aPluginUid )
    {
    OstTraceFunctionEntry0( CBLACKLISTMGR_ADDTOUNLOADLISTL_ENTRY );
    OstTrace1( TRACE_NORMAL, CBLACKLISTMGR_ADDTODONTLOADLISTL, "CBlacklistMgr::AddtoDontloadListL;Uid=%x", aPluginUid.iUid );
    CPIXLOGSTRING2("CBlacklistMgr::AddtoDontloadListL(): Uid = %x " , aPluginUid.iUid );
    //Check if the record with given plugin uid is already available in database or not
    //If available just ignore the addition
    //If there is no record found in database with given uid, add new record with given uid    
    TInt err = KErrNone;
    
    if( !(iBlacklistDb->FindInDontloadListL( aPluginUid.iUid )) )    
        {
        err = iBlacklistDb->AddtoDontloadListL( aPluginUid.iUid );
        }
      
    CPIXLOGSTRING("CBlacklistMgr::AddtoDontloadListL(): Exit");    
    OstTraceFunctionExit0( CBLACKLISTMGR_ADDTODONTLOADLISTL_EXIT );
    return err;
    }

// -----------------------------------------------------------------------------
// CBlacklistMgr::RemoveFromDontloadListL()
// -----------------------------------------------------------------------------
//
void CBlacklistMgr::RemoveFromDontloadListL( TUid aPluginUid )
    {
    OstTraceFunctionEntry0( CBLACKLISTMGR_REMOVEFROMUNLOADLISTL_ENTRY );
    OstTrace1( TRACE_NORMAL, CBLACKLISTMGR_REMOVEFROMUNLOADLISTL, "CBlacklistMgr::RemoveFromDontloadListL;Uid=%x", aPluginUid.iUid );
    CPIXLOGSTRING2("CBlacklistMgr::RemoveFromDontloadListL(): Uid = %x " , aPluginUid.iUid );
    //Remove the item record to database
    iBlacklistDb->RemoveFromDontloadListL( aPluginUid.iUid );
    
    CPIXLOGSTRING("CBlacklistMgr::RemoveFromDontloadListL(): Exit");  
    OstTraceFunctionExit0( CBLACKLISTMGR_REMOVEFROMDONTLOADLISTL_EXIT );
    }

// -----------------------------------------------------------------------------
// CBlacklistMgr::FindfromDontloadListL()
// -----------------------------------------------------------------------------
//
TBool CBlacklistMgr::FindInDontloadListL(TUid aPluginUid )
    {
    CPIXLOGSTRING2("CBlacklistMgr::FindInDontloadListL(): Uid = %x " , aPluginUid.iUid );
    OstTrace1( TRACE_NORMAL, CBLACKLISTMGR_FINDINDONTLOADLISTL, "CBlacklistMgr::FindInDontloadListL;Uid=%x", aPluginUid.iUid );
    return ( iBlacklistDb->FindInDontloadListL( aPluginUid.iUid ) );    
    }
