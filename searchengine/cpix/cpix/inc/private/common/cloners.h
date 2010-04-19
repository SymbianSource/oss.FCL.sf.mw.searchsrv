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

#ifndef CPIX_CLONERS_H
#define CPIX_CLONERS_H

#include <memory>

#include "CLucene.h"

#include "cpixexc.h"

namespace Cpix
{

    /**
     * Clones a lucene document, binary fields are not supported.
     *
     * @param orig the original document to clone
     *
     * @return a new instance that has the same fields as the
     * original. Ownership passes to client.
     */
    std::auto_ptr<lucene::document::Document> 
    Clone(lucene::document::Document * orig);


    /**
     * A function type that can observe the name/value of a field.
     */
    typedef void FieldWatcher(const wchar_t * name,
                              const wchar_t * value,
                              int             config);


    /**
     * Clones a lucene document, binary fields are not
     * supported. Also, while it enumerates the fields, it calls a
     * callback functor for each field (name/value pair) encountered.
     *
     * Handy when you want a clone AND you also want to dig up some
     * info about the document content.
     *
     * @param orig the original document to clone
     *
     * @param fieldWatcher this functor is invoked for all field
     * enumerated. Templated parameter.
     *
     * @return a new instance that has the same fields as the
     * original. Ownership passes to client.
     * 
     */
    template<typename FIELDWATCHER>
    std::auto_ptr<lucene::document::Document>
    CloneWatchingFields(lucene::document::Document * orig,
                        FIELDWATCHER                 fieldWatcher);



}


/*********************************************************
 *
 *
 *  I M P L E M E N T A T I O N   O F   T E M P L A T E S 
 *
 */

namespace Cpix
{

    template<typename FIELDWATCHER>
    std::auto_ptr<lucene::document::Document>
    CloneWatchingFields(lucene::document::Document * orig,
                        FIELDWATCHER                 fieldWatcher)
    {
        using namespace std;
        using namespace lucene::document;

        auto_ptr<lucene::document::Document>
            rv(new lucene::document::Document);

        auto_ptr<DocumentFieldEnumeration>
            dfe(orig->fields());

        while (dfe->hasMoreElements())
            {
                lucene::document::Field
                    * field = dfe->nextElement();

                if (field->isBinary())
                    {
                        THROW_CPIXEXC("Binary fields are not supported");
                    }

                const wchar_t
                    * name = field->name(),
                    * value = field->stringValue();

                int
                    config = field->getConfig();

                fieldWatcher(name,
                             value,
                             config);

                auto_ptr<lucene::document::Field>
                    newField(new lucene::document::Field(name,
                                                         value,
                                                         config));

                rv->add(* newField.get());
                newField.release();
            }

        return rv;
    }


}


#endif

