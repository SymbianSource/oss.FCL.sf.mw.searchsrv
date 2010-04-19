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

#include "documentconsumer.h"

#include "common/cpixlog.h"

namespace Cpix
{
    void DocumentConsumer::fetchDocuments(lucene::search::Hits * hits)
    {
        bool
            goOn = true;

        for (int32_t idx = beginIndex();
             goOn && idx < endIndex();
             ++idx)
            {
                try
                    {
                        lucene::document::Document
                            * doc = &(hits->doc(idx));

                        useDocument(idx,
                                    doc);
                    }
                catch (LuceneError & clErr)
                    {
                        int
                            errNum = clErr.number();

                        failedDocument(idx,
                                       errNum);

                        // if the document was not loaded because it
                        // was deleted, the next document might still
                        // be loadable
                        goOn = (errNum == CL_ERR_DeletedDocument);

                        logDbgMsg("fetchDocuments() Failed to load doc %d: %d/%S",
                                  idx,
                                  errNum,
                                  clErr.twhat());
                    }
                catch (...)
                    {
                        failedDocument(idx,
                                       CL_ERR_UNKNOWN);

                        goOn = false;

                        logDbgMsg("fetchDocuments() Failed to load doc %d.",
                                  idx);
                    }
            }
    }



    DocumentConsumer::~DocumentConsumer()
    {
        ;
    }

}
