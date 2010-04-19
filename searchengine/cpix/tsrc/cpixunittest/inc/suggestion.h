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

#ifndef SUGGESTION_H_
#define SUGGESTION_H_

#include <string>

#include "itk.h"
#include "cpixdoc.h"


class Suggestion
{
private:
    std::wstring  term_;
    std::wstring  frequency_;

    friend bool operator<(const Suggestion & left,
                          const Suggestion & right);

public:
    Suggestion(cpix_Document & doc,
               Itk::TestMgr  * testMgr);

        
    const std::wstring & term() const;
    const std::wstring & frequency() const;


    static void printSuggestions(cpix_Hits    * hits,
                                 Itk::TestMgr * testMgr);
    
};


bool operator<(const Suggestion & left,
               const Suggestion & right);


#endif
