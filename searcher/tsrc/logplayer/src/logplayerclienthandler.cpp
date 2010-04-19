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


//  Include Files
#include "LogPlayerClientHandler.h"

// Contants
// None

// ========================= MEMBER FUNCTIONS ==================================

// -----------------------------------------------------------------------------
// CLogPlayerClientHandler::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CLogPlayerClientHandler* CLogPlayerClientHandler::NewL()
    {
    CLogPlayerClientHandler* logPlayerManager = CLogPlayerClientHandler::NewLC();
    CleanupStack::Pop(logPlayerManager);
    return logPlayerManager;
    }

// -----------------------------------------------------------------------------
// CLogPlayerClientHandler::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CLogPlayerClientHandler* CLogPlayerClientHandler::NewLC()
    {
    CLogPlayerClientHandler* logPlayerManager = new (ELeave) CLogPlayerClientHandler();
    CleanupStack::PushL(logPlayerManager);
    logPlayerManager->ConstructL();
    return logPlayerManager;
    }

// -----------------------------------------------------------------------------
// CLogPlayerClientHandler::~CLogPlayerClientHandler()
// Destructor.
// -----------------------------------------------------------------------------
//
CLogPlayerClientHandler::~CLogPlayerClientHandler()
    {
    }

void CLogPlayerClientHandler::ResetL()
    {
    }
// -----------------------------------------------------------------------------
// CLogPlayerClientHandler::CLogPlayerClientHandler()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CLogPlayerClientHandler::CLogPlayerClientHandler()
    {
    }

// -----------------------------------------------------------------------------
// CLogPlayerClientHandler::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CLogPlayerClientHandler::ConstructL()
    {
    }
