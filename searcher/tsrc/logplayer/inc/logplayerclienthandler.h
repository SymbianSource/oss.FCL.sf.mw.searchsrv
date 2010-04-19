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


#ifndef LOGPLAYERCLIENTHANDLER_H
#define LOGPLAYERCLIENTHANDLER_H

#include "CCPixIndexer.h"
#include "CCPixSearcher.h"
#include "CSearchDocument.h"
#include "RSearchServersession.h"

//  Include Files
#include <e32base.h>

class CLogPlayerClientHandler : public CBase
{
public : // Constructors and destructors

    /**
    * NewL.
    * Two-phased constructor.
    * @return Pointer to created CLogPlayerClientHandler object.
    */
    static CLogPlayerClientHandler* NewL();

    /**
    * NewLC.
    * Two-phased constructor.
    * @return Pointer to created CLogPlayerClientHandler object.
    */
    static CLogPlayerClientHandler* NewLC();

    /**
    * Destructor.
    */
    virtual ~CLogPlayerClientHandler();

public: // New functions
    
    void ResetL();
    
    //  None
    
private: // New functions
    
    // None
    
private: // Constructors and destructors

    /**
    * C++ default constructor.
    */
    CLogPlayerClientHandler();

    /**
    * ConstructL.
    * 2nd phase constructor.
    */
    void ConstructL();
    
private: // Data
    
    CCPixIndexer* iIndexer;
    CCPixSearcher* iSearcher;
    
}
;
#endif  // LOGPLAYERCLIENTHANDLER_H

