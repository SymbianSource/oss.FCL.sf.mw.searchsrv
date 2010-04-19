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

#include <wchar.h>
#include <string.h>

#include "CLucene.h"

#include "indevicecfg.h"

#include "document.h"

#include "fileparser/fileparser.h" // removeStandardFields()

#include <iostream>


namespace Cpix
{
    bool isUnrecognizedFile(const char * )
    {
        return true;
    }



    void processUnrecognizedFile(Cpix::Document * doc,
                                 const char     * )
    {
        doc->setAppClass( UNKFILEAPPCLASS );  
    }
    
}
