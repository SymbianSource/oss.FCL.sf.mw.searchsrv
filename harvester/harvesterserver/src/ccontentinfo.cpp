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

#include "ccontentinfo.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "ccontentinfoTraces.h"
#endif


// -----------------------------------------------------------------------------
// CContentInfo::NewL()
// -----------------------------------------------------------------------------
//
CContentInfo* CContentInfo::NewL()
    {
    OstTraceFunctionEntry0( CCONTENTINFO_NEWL_ENTRY );    
    CContentInfo* instance = CContentInfo::NewLC();
    CleanupStack::Pop( instance );
    OstTraceFunctionExit0( CCONTENTINFO_NEWL_EXIT );
    return instance;
    }

// -----------------------------------------------------------------------------
// CContentInfo::NewLC()
// -----------------------------------------------------------------------------
//
CContentInfo* CContentInfo::NewLC()
    {
    OstTraceFunctionEntry0( CCONTENTINFO_NEWLC_ENTRY );
    CContentInfo* instance = new (ELeave) CContentInfo();
    CleanupStack::PushL( instance );
    OstTraceFunctionExit0( CCONTENTINFO_NEWLC_EXIT );
    return instance;
    }
// -----------------------------------------------------------------------------
// CContentInfo::CContentInfo()
// -----------------------------------------------------------------------------
//
CContentInfo::CContentInfo()
    {
    //Do necessary initialization here
    }
// -----------------------------------------------------------------------------
// CContentInfo::~CContentInfo()
// -----------------------------------------------------------------------------
//
CContentInfo::~CContentInfo()
    {
    delete iContentName;
    }
// -----------------------------------------------------------------------------
// CContentInfo::GetNameL()
// -----------------------------------------------------------------------------
//
HBufC* CContentInfo::GetNameL()
    {
    OstTraceFunctionEntry0( CCONTENTINFO_GETNAMEL_ENTRY );
    HBufC* contentname = NULL;    
    if ( iContentName->Length() )        
        {        
        contentname = iContentName->AllocL();
        }    
    OstTraceFunctionExit0( CCONTENTINFO_GETNAMEL_EXIT );
    return contentname;
    }

// -----------------------------------------------------------------------------
// CContentInfo::GetIndexStatusL()
// -----------------------------------------------------------------------------
//
TInt CContentInfo::GetIndexStatus()
    {
    return iIndexStatus;
    }

// -----------------------------------------------------------------------------
// CContentInfo::GetBlacklistStatusL()
// -----------------------------------------------------------------------------
//
TInt CContentInfo::GetBlacklistStatus()
    {
    return iBlacklistStatus;
    }

// -----------------------------------------------------------------------------
// CContentInfo::SetNameL()
// -----------------------------------------------------------------------------
//
void CContentInfo::SetNameL( const TDesC& aName )
    {
    OstTraceFunctionEntry0( CCONTENTINFO_SETNAMEL_ENTRY );
    if ( iContentName )
        {
        delete iContentName;
        iContentName = NULL;
        }
    
    iContentName = HBufC::NewL( aName.Length() );
    iContentName->Des().Copy( aName );
    OstTraceFunctionExit0( CCONTENTINFO_SETNAMEL_EXIT );
    }

// -----------------------------------------------------------------------------
// CContentInfo::SetIndexStatusL()
// -----------------------------------------------------------------------------
//
void CContentInfo::SetIndexStatus( const TInt aIndexStatus )
    {
    iIndexStatus = aIndexStatus;
    }

// -----------------------------------------------------------------------------
// CContentInfo::SetBlacklistStatusL()
// -----------------------------------------------------------------------------
//
void CContentInfo::SetBlacklistStatus( const TBool aBlacklistStatus )
    {
    iBlacklistStatus = aBlacklistStatus;
    }
//EOF
