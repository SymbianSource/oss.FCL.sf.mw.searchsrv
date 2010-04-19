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

#include "indevicecfg.h"

#include "cpixtools.h"

#include "ifieldfilter.h"
#include "filters/quadfilter.h"

#include "document.h"
#include "cpixdoc.h"
#include "common/cpixlog.h"

namespace Cpix
{

    bool QuadFilter::process(Cpix::Document* doc)
    {
        using namespace std;
        using namespace Cpt;

        bool
            rv = true;

        const wchar_t
            * gpsLatStr = doc->get(LLATITUDE_FIELD),
            * gpsLongStr = doc->get(LLONGITUDE_FIELD);

        if (gpsLatStr != NULL && gpsLongStr != NULL)
            {
                double
                    gpsLat,
                    gpsLong;

                const char
                    * result = NULL;

                result = wconvert(&gpsLat,
                                  gpsLatStr);

                if (result == NULL)
                    {
                        result = wconvert(&gpsLong,
                                          gpsLongStr);

                        if (result == NULL)
                            {

                                process(doc,
                                        gpsLat,
                                        gpsLong);
                            }
                        else
                            {
                                emitWarning(doc,
                                            "converting GPS longitude failed");
                            }
                    }
                else
                    {
                        emitWarning(doc,
                                    "converting GPS latitude failed");
                    }
            }
        else
            {
                emitWarning(doc,
                            "missing GPS lat/long field(s)");
            }

        return rv;
    }


    void QuadFilter::emitWarning(Cpix::Document * doc,
                                 const char                 * msg)
    {
        const wchar_t
            * docUid = doc->get(LCPIX_DOCUID_FIELD);

        logMsg(CPIX_LL_ERROR,
               "Quad filter could not create field '%S' for %S: %s\n",
               QNR_FIELD,
               docUid,
               msg);
    }
    

    void QuadFilter::process(Cpix::Document * doc,
                             double           gpsLat,
                             double           gpsLong)
    {
        using namespace Cpt;

        QNr
            qnr(gpsLat,
                gpsLong);

        // QNr field is a space delimited enumaration of string
        // representation of QNr-s, starting from the smallest one and
        // going to ever bigger ones (parents).
        //
        // A QNR stringified value is a "z(0)z(1)...z(n-1)" string
        // where z(i) is y(i)x(i) (two digits), where the pair (x(i),
        // y(i)) identifie the child quad on the ith level in the quad
        // tree. So, for a qnr value, the following value should be
        // created and put into the index for the qnr field:
        //
        //     "z(0)...z(15) z(0)...z(14)...z(0)z(1) z(0)"
        //
        // The length of this string value for QNR_FIELD is always the
        // same:     sum of word lengths that are 1, ..., 16 chars long
        //                   |
        //                   |        space delimiters between words
        //                   |          |
        //                   |          |  zero terminator
        //                   |          |    |
        //                   V          V    V
        static const size_t
            qnrFieldLength = 8*(1+16) + 15 + 1;
        wstring
            qnrFieldValue;
        qnrFieldValue.reserve(qnrFieldLength);

        for (int i = 0; i < Cpt::QNr::MAX_LEVEL; ++i)
            {
                if (i > 0)
                    {
                        qnrFieldValue += ' ';
                    }

                qnrFieldValue += qnr.toWString();
                qnr = qnr.getParentQNr();
            }
        
        // TODO finally: STORE_NO
        doc->add( new Cpix::Field(QNR_FIELD,
                                  qnrFieldValue.c_str(),
                                  cpix_STORE_YES | cpix_INDEX_TOKENIZED | cpix_AGGREGATE_NO));
    }

    QuadFilter::~QuadFilter()
    {
        ;
    }

}
