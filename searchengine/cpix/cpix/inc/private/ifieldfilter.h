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

#ifndef CPIX_IFIELDFILTER_H_
#define CPIX_IFIELDFILTER_H_

#include <wchar.h>

#include <map>
#include <string>

#include "CLucene.h"

#include "cpixsynctools.h"

#include "cpixidxdb.h"

// Forward declarations
namespace Cpix {
	class Document; 
}

namespace Cpix
{

    /**
     * Internal interface that abstracts the notion of field
     * filters. Field filters process values given in field
     * declarations (cpix_FieldDesc) and produce final values for
     * fields to be indexed. Field filters are identified by
     * identifiers, see cpix_FieldFilterId and cpix_FieldDesc
     * types.
     */
    class IFieldFilter
    {
    public:
        //
        // public operations
        //

        /**
         * Processes a field (name/value pair) and produces actual
         * fields (to index and/or store) on the document just before
         * it is given to clucene for addition to the
         * index. Implementations of the IFieldFilter must be
         * stateless, i.e. no state change may occur to an instance of
         * IFieldFilter accross different calls to process. (This is
         * to enable pooling such instances instead of creating and
         * destroying them all the time. If state is required, then
         * bool isStateless() const; method must be added to this
         * interface.)
         *
         * @param doc the document to add the created fields to. A
         * process call may create multiple fields on the
         * document, and remove others. Must not be NULL.
         *
         * @return true if the document was processed okay and it can
         * go to the index now, false otherwise (bail out)
         *
         */
        virtual bool process(Cpix::Document * doc) = 0;
        

        /**
         * Destructor
         */
        virtual ~IFieldFilter() = 0;
    };


    
    /**
     * Singleton class (with static methods for help) that is (a)
     * factory of field filters based on their IDs (b) manages a pool
     * of field filter instances in order to reduce
     * creation/destruction cycles.
     */
    class FieldFilterPool
    {
        //
        // private members
        //
        typedef std::map<std::wstring, IFieldFilter*> FilterMap;
        FilterMap                filters_;

        /**
         * Protects the member "filters_" as it can be possibly
         * accessed by multiple threads.
         */
        Cpt::Mutex               mutex_;

        static FieldFilterPool * instance_;

    public:
        //
        // public operators
        //
        /**
         * Gets a filter instance based on a given filter id. May fail
         * with CpixExc thrown. Thread-safe.
         */
        static IFieldFilter * getFieldFilter(const wchar_t * ffid);

        /**
         * Releases a filter instance. Thread-safe.
         */
        static void releaseFieldFilter(IFieldFilter * ff);

        /**
         * Initializes the singleton. Not thread-safe (and it should
         * not need to be).
         */
        static void init();

        /**
         * Destroys all filter instances in this pool and releases the
         * pool instance itself. Not thread-safe (and it should not
         * be).
         */
        static void cleanup();


    private:
        //
        // private methods
        //

        /**
         * Ctor
         */
        FieldFilterPool();


        /**
         * Singleton retrieval.
         */
        static FieldFilterPool * instance();


        /**
         * Thread-safe.
         */
        IFieldFilter * getFieldFilter_(const wchar_t * ffid);

        /**
         * Thread-safe.
         */
        void releaseFieldFilter_(IFieldFilter * ff);

        /**
         * Thread-safe.
         */
        void cleanup_();

        IFieldFilter * createFieldFilter(const wchar_t * ffid);
    };


    /**
     * Utility class to guard the lifetime of field filters.
     */
    class FieldFilterSentry
    {
    private:
        //
        // private members
        //

        IFieldFilter    * fieldFilter_;

    public:
        //
        // public operators
        //
        
        /**
         * Ctor - guarding on IFieldFilter instance.
         */
        explicit FieldFilterSentry(IFieldFilter * ff);


        /**
         * Dtor - makes sure the field filter instance is properly
         * release to the pool it was taken from.
         */
        ~FieldFilterSentry();


        /**
         * @return the stored pointer
         */
        IFieldFilter * get();
    };


}


#endif // CPIX_IFIELDFILTER_H_
