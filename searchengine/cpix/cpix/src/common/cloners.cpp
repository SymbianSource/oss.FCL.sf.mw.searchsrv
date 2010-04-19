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

#include "common/cloners.h"


namespace
{
    void DummyFieldWatcher(const wchar_t * /* name */,
                           const wchar_t * /* value */,
                           int             /* config */)
    {
        ;
    }
}



namespace Cpix
{


    std::auto_ptr<lucene::document::Document> 
    Clone(lucene::document::Document * orig)
    {
        return CloneWatchingFields(orig,
                                   &DummyFieldWatcher);
    }



}
