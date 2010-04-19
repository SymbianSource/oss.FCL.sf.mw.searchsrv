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

#ifndef CPIX_CPIXDOCUMENTCONSUMER_H
#define CPIX_CPIXDOCUMENTCONSUMER_H

#include <stdint.h>

#include "CLucene.h"


namespace Cpix
{

    /**
     * This base class is used by LuceneHits to be able to fetch a
     * number of documents in one go.
     *
     * Rationale: with multithreading:
     *
     * (a) every document fetch operation may need to recommit search
     *     due to indexing going on
     *
     * (b) even if no dependant index has been changed, just
     *     collecting all the indexes and lock on to them is
     *     time-consuming
     *
     * So it makes sense to fetch not only one document (that is being
     * requested by the user), but a number of them in the same go,
     * while all indexes have been locked and brought into a state
     * where they can be used for recommit / fetching document.
     *
     * The range [ beingIndex(), endIndex() ) defines the indexes for
     * hits document that are requested. If the entity fetching the
     * documents cannot fetch a hit document at a certain index, then
     * depending on type of failure, we must take different actions -
     * cf fetchDocument method.
     */
    class DocumentConsumer
    {
    protected:
        //
        // protected operations
        //

        /**
         * The first index of a hit document that is being requested.
         */
        virtual int32_t beginIndex() const = 0;


        /**
         * The index of the last hit document that is being requested
         * plus one.
         */
        virtual int32_t endIndex() const = 0;


        /**
         * A document was fetched for an index so that 
         *
         *    beginIndex() <= index < endIndex()
         *
         * and now the consumer can have it.
         */
        virtual void useDocument(int32_t                      index,
                                 lucene::document::Document * document) = 0;


        /**
         * In the case of a document fetch failure, one must report
         * the reason (the important ones).
         *
         * @param index the index of the hit document that failed to
         *        get fetched
         *
         * @param clErrorNumber - the error number as defined in
         *        clucene sources: debug/error.h
         */
        virtual void failedDocument(int32_t        index,
                                    int            clErrorNumber) = 0;


    public:
        //
        // public operations
        //
        
        /**
         * Fetches the required documents in a loop. Based on the type
         * of failure it may continue fetching the next required
         * document (e.g. document was deleted), or may stop the loop
         * (index out of bounds).
         *
         * All an IIdxDb implementation needs to do is
         *
         * (a) make sure to have a valid hits object to use
         *
         * (b) make sure it is not modified while this function runs
         *
         */
        void fetchDocuments(lucene::search::Hits * hits);


        /**
         * Destructor.
         */
        virtual ~DocumentConsumer() = 0;


    private:
        //
        // private methods
        //

    };

}


#endif // CPIX_CPIXDOCUMENTCONSUMER_H
