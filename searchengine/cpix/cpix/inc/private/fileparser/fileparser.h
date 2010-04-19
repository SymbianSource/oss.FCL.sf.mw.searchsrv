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

#ifndef CPIX_FILEPARSER_H_
#define CPIX_FILEPARSER_H_

#include "ifieldfilter.h"
namespace Cpix
{

    // Removes the standard fields from the Document doc.
    // These fields are appclass, mimetype and excerpt.
    void removeStandardFields(Cpix::Document * doc);
    
    void GenericFileProcessor(Cpix::Document* doc,const char * path);

    class FileParser : public IFieldFilter
    {
    public:
        //
        // public operations
        //
        
        virtual bool process(Cpix::Document * doc);
        

        virtual ~FileParser();
    };

}

#endif // CPIX_FILEPARSER_H_
