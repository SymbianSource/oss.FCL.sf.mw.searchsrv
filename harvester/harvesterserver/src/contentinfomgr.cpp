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

#include "contentinfomgr.h"
#include "ccotentinfodb.h"
#include "ccontentinfo.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "contentinfomgrTraces.h"
#endif


// -----------------------------------------------------------------------------
// CContentInfoMgr::NewL()
// -----------------------------------------------------------------------------
//
CContentInfoMgr* CContentInfoMgr::NewL()
    {
    OstTraceFunctionEntry0( CCONTENTINFOMGR_NEWL_ENTRY );
    CContentInfoMgr* instance = CContentInfoMgr::NewLC();
    CleanupStack::Pop( instance );
    OstTraceFunctionExit0( CCONTENTINFOMGR_NEWL_EXIT );
    return instance;
    }

// -----------------------------------------------------------------------------
// CContentInfoMgr::NewLC()
// -----------------------------------------------------------------------------
//
CContentInfoMgr* CContentInfoMgr::NewLC()
    {
    OstTraceFunctionEntry0( CCONTENTINFOMGR_NEWLC_ENTRY );
    CContentInfoMgr* instance = new (ELeave) CContentInfoMgr();
    CleanupStack::PushL( instance );
    instance->ConstructL();
    OstTraceFunctionExit0( CCONTENTINFOMGR_NEWLC_EXIT );
    return instance;
    }

// -----------------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------------
//
CContentInfoMgr::CContentInfoMgr()
    {
    //Do the necessary initialisation
    }

// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
//
CContentInfoMgr::~CContentInfoMgr()
    {
    delete iContentInfoDb;
    }

// -----------------------------------------------------------------------------
// CContentInfoMgr::ConstructL
// -----------------------------------------------------------------------------
//
void CContentInfoMgr::ConstructL()
    { 
    OstTraceFunctionEntry0( CCONTENTINFOMGR_CONSTRUCTL_ENTRY );
    iContentInfoDb = CContentInfoDb::NewL();
    OstTraceFunctionExit0( CCONTENTINFOMGR_CONSTRUCTL_EXIT );
    }

// -----------------------------------------------------------------------------
// CContentInfoMgr::AddL
// -----------------------------------------------------------------------------
//
TInt CContentInfoMgr::AddL( CContentInfo* aContentInfo )
    {
    return iContentInfoDb->AddL( aContentInfo );
    }

// -----------------------------------------------------------------------------
// CContentInfoMgr::UpdateBlacklistStatusL
// -----------------------------------------------------------------------------
//
TInt CContentInfoMgr::UpdateBlacklistStatusL( const TDesC& aContentName , TInt aBlacklistStatus )
    {
    return iContentInfoDb->UpdateBlacklistStatusL( aContentName, aBlacklistStatus );
    }

// -----------------------------------------------------------------------------
// CContentInfoMgr::UpdatePluginIndexStatusL
// -----------------------------------------------------------------------------
//
TInt CContentInfoMgr::UpdatePluginIndexStatusL( const TDesC& aContentName , TInt aIndexStatus )
    {
    return iContentInfoDb->UpdatePluginIndexStatusL( aContentName, aIndexStatus );
    }

// -----------------------------------------------------------------------------
// CContentInfoMgr::FindL
// -----------------------------------------------------------------------------
//
TBool CContentInfoMgr::FindL( const TDesC& aContentName )
    {
    return iContentInfoDb->FindL( aContentName );
    }

// -----------------------------------------------------------------------------
// CContentInfoMgr::Remove
// -----------------------------------------------------------------------------
//
void CContentInfoMgr::RemoveL( const TDesC& aContentName )
    {
    iContentInfoDb->RemoveL( aContentName );
    }

// -----------------------------------------------------------------------------
// CContentInfoMgr::Remove
// -----------------------------------------------------------------------------
//
void CContentInfoMgr::ResetL( )
    {
    iContentInfoDb->ResetDatabaseL();
    }

// -----------------------------------------------------------------------------
// CContentInfoMgr::GetContentCountL
// -----------------------------------------------------------------------------
//
TInt CContentInfoMgr::GetContentCountL( )
    {
    return iContentInfoDb->GetContentCountL();
    }
//EOF
