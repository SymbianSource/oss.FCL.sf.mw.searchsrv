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


// INCLUDE FILES
#include <e32svr.h>
#include <S32MEM.H>

#include "CHarvesterServerSession.h"
#include "HarvesterServerCommons.h"
#include "CHarvesterServer.h"
#include <CSearchDocument.h>

// ========================= MEMBER FUNCTIONS ==================================

// -----------------------------------------------------------------------------
// CHarvesterServerSession::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CHarvesterServerSession* CHarvesterServerSession::NewL()
	{
	CHarvesterServerSession* self = CHarvesterServerSession::NewLC();
	CleanupStack::Pop(self);
	return self;
	}

// -----------------------------------------------------------------------------
// CHarvesterServerSession::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CHarvesterServerSession* CHarvesterServerSession::NewLC()
	{
	CHarvesterServerSession* self = new ( ELeave ) CHarvesterServerSession();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

// -----------------------------------------------------------------------------
// CHarvesterServerSession::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CHarvesterServerSession::ConstructL()
	{        
	}

// -----------------------------------------------------------------------------
// CHarvesterServerSession::CHarvesterServerSession()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CHarvesterServerSession::CHarvesterServerSession()
	{
	}

// -----------------------------------------------------------------------------
// CHarvesterServerSession::~CHarvesterServerSession()
// Destructor.
// -----------------------------------------------------------------------------
//
CHarvesterServerSession::~CHarvesterServerSession()
	{
	}

// -----------------------------------------------------------------------------
// CHarvesterServerSession::ServiceL()
// Service request from client.
// -----------------------------------------------------------------------------
//
void CHarvesterServerSession::ServiceL(const RMessage2& aMessage)
	{
	switch (aMessage.Function())
		{
		// TODO: HarvesterServer API

		default:
			PanicClient(aMessage, EBadRequest);
		}
	
	aMessage.Complete(KErrNone);
	}

// Utility function to panic client
void CHarvesterServerSession::PanicClient(const RMessage2& aMessage, TInt aPanic) const
	{
	aMessage.Panic(KHarvesterServer, aPanic); // Note: this panics the client thread, not server
	}

// End of File

