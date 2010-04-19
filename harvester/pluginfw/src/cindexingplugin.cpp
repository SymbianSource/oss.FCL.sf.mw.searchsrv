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

#include "CIndexingPlugin.h"
#include <ecom.h>

GLDEF_C TInt E32Dll()
	{
	return(KErrNone);
	}

EXPORT_C CIndexingPlugin* CIndexingPlugin::NewL(const TUid aUid)
	{
	TAny* indexingPlugin = REComSession::CreateImplementationL(
			aUid, _FOFF( CIndexingPlugin, iDtor_ID_Key ) );
	return (reinterpret_cast<CIndexingPlugin*>(indexingPlugin));
	}

// -----------------------------------------------------------------------------
// CIndexingPlugin::ListImplementationsL
// -----------------------------------------------------------------------------
//
EXPORT_C void CIndexingPlugin::ListImplementationsL(RImplInfoPtrArray& aImplInfoArray)
	{
	REComSession::ListImplementationsL(
			KIndexingPluginInterfaceUid, aImplInfoArray );
	}
	
// -----------------------------------------------------------------------------
// CIndexingPlugin::~CIndexingPlugin
// -----------------------------------------------------------------------------
//
EXPORT_C CIndexingPlugin::~CIndexingPlugin()
	{
	REComSession::DestroyedImplementation( iDtor_ID_Key );
	}


// -----------------------------------------------------------------------------
// CIndexingPlugin::SetObserver
// -----------------------------------------------------------------------------
//
EXPORT_C void CIndexingPlugin::SetObserver(MIndexingService& aObserver)
	{
	iObserver = &aObserver;
	}

// -----------------------------------------------------------------------------
// CIndexingPlugin::SetSearchSession
// -----------------------------------------------------------------------------
//
EXPORT_C void CIndexingPlugin::SetSearchSession(RSearchServerSession& aSearchSession)
	{
	iSearchSession = aSearchSession;
	}

// -----------------------------------------------------------------------------
// CIndexingPlugin::Flush
// -----------------------------------------------------------------------------
//
EXPORT_C void CIndexingPlugin::Flush(CCPixIndexer& aIndexer)
	{
	TRAP_IGNORE( aIndexer.FlushL() );
	}
