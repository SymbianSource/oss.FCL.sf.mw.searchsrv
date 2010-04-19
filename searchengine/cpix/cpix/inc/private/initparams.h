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

#ifndef CPIX_INITPARAMS_H
#define CPIX_INITPARAMS_H


#include <string>

#include "cpixexc.h"

namespace Cpix
{

    // TODO move this utility to Cpt

    /**
     * Template base class for integer-type properties performing
     * default initialization and sanity checked property setting and
     * getting based on some declarative consts.
     *
     * template parameter: PROPTYPE the type of the property, some
     * integer type
     *
     * template parameter: PROPDEF has DEF, MIN and MAX members,
     * convertible to type PROPTYPE, defining the default, minimum and
     * maximum values for the property, respectively. It also has the
     * static const char [] typed NAME variable, for error reporting.
     *
     * POV class.
     */
    template<typename     PROPTYPE,
             typename     PROPDEF>
    class Property
    {
    private:
        //
        // private members
        //

        PROPTYPE      value_;


    public:
        //
        // public operators
        //

        /**
         * Gets the value stored by this property instance.
         */
        PROPTYPE get() const 
        {
            return value_;
        }


        /**
         * Sets the value stored by this property instance
         * performing sanity checks: value should be in the
         * range predefined by
         *
         * [PROPDEF::MIN, PROPDEF::MAX];
         */
        void set(PROPTYPE value)
        {
            if (value < PROPDEF::MIN)
                {
                    THROW_CPIXEXC("Value (%ld) too small for property %s",
                                  static_cast<long>(value),
                                  PROPDEF::NAME);
                }
            else if (value > PROPDEF::MAX)
                {
                    THROW_CPIXEXC("Value (%lu) too big for property %s",
                                  static_cast<long>(value),
                                  PROPDEF::NAME);
                }
                    
            value_ = value;
        }

                
        /**
         * Constructs this property and initializes its value
         * with the PROPDEF::DEF value.
         */
        Property()
            : value_(PROPDEF::DEF)
        {
            ;
        }
    };




    //
    //
    // Default, minimum and maximum values (inclusive) for parameters
    // that are set at runtime at initialization.
    //

#define TODO_DEFINEPROPERLY_DUMMY 1
            
    /**
     * Default, minimum and maximum values (inclusive) for
     * InitParams::logSizeLimit property.
     */
    struct LOGSIZELIMIT
    {
        enum 
            {
                DEF =  200 * 1024,
                MIN =         500,
                MAX = 1024 * 1024
            };
        static const char NAME[];
    };



    /**
     * Default, minimum and maximum values (inclusive) for
     * InitParams::logSizeCheckRecurrency property.
     */
    struct LOGSIZECHECKRECURRENCY
    {
        enum
            {
                DEF = 10,
                MIN = 1,
                MAX = 100
            };
        static const char NAME[];
    };

            
    /**
     * Default, minimum and maximum values (inclusive) for
     * InitParams::maxIdleSec property.
     */
    struct MAXIDLESEC
    {
        enum
            {
                DEF = 60,
                MIN = 5,
                MAX = 400
            };
        static const char NAME[];
    };


    /**
     * Default, minimum and maximum values (inclusive) for
     * InitParams::maxInsertBufSize property.
     */
    struct MAXINSERTBUFSIZE
    {
        enum
            {
                DEF = 32 * 1024,
                MIN =       512,
                MAX = 64 * 1024
            };
        static const char NAME[];
    };


    /**
     * Default, minimum and maximum values (inclusive) for
     * InitParams::insertMaxBufDocs property.
     */
    struct INSERTBUFMAXDOCS
    {
        enum
            {
                DEF = 10,
                MIN = 5,
                MAX = 100
            };
        static const char NAME[];
    };


    /**
     * Default, minimum and maximum values (inclusive) for
     * InitParams::idxJobQueueSize property.
     */
    struct IDXJOBQUEUESIZE
    {
        enum
            {
                DEF = 5,
                MIN = 3,
                MAX = 10
            };
        static const char NAME[];
    };


    /**
     * Default, minimum and maximum values (inclusive) for
     * InitParams::qryJobQueueSize property.
     */
    struct QRYJOBQUEUESIZE
    {
        enum
            {
                DEF = 5,
                MIN = 2,
                MAX = 10
            };
        static const char NAME[];
    };


    /**
     * Default, minimum and maximum values (inclusive) for
     * InitParams::idxThreadPriorityDelta.
     */
    struct IDXTHREADPRIORITYDELTA
    {
        enum
            {
                DEF =    0,
                MIN =   -20,
                MAX =    20
            };
        static const char NAME[];
    };
            

    /**
     * Default, minimum and maximum values (inclusive) for
     * InitParams::qryThreadPriorityDelta.
     */
    struct QRYTHREADPRIORITYDELTA
    {
        enum
            {
                DEF =    0,
                MIN =   -20,
                MAX =    20
            };
        static const char NAME[];
    };


    /**
     * Default, minimum and maximum values (inclusive) for
     * InitParams::clHitsPageSize property.
     */
    struct CLHITSPAGESIZE
    {
        enum
            {
                DEF = 20,
                MIN = 5,
                MAX = 500
            };
        static const char NAME[];
    };



    //
    //
    // Values for parameters that are not modified at runtime.
    //
    enum Consts
        {
            /**
             * Exceptions from lower components (C++ runtime, STL,
             * CLucene, CpixTools, Cpix itself) are rendered and
             * stored in a message buffer of this size.
             */
            ERRORMSG_BUFSIZE      = 256,
            
            
            /**
             * The pool of ErrorInfo instances should keep this many
             * ErrorInfo instances around for reuse.
             */
            MIN_ERRORINFO_COUNT   = 2,
            
            
            /**
             * Temporary buffer size in different locations.
             */
            EXCMSG_BUFSIZE        = 256,


            /**
             * Index instances forcibly removed by UndefineVolume
             * cannot be discarded immediately due to potential active
             * jobs still using it, so it is discarded after a number
             * of housekeep rounds. How many housekeep rounds later,
             * is defined by this enum here.
             */
            DISCARD_SAFETY_PERIOD = 2
        };



    /**
     * This class is a POV (with sanity checking property setters)
     * that stores all arguments required to start Cpix core.
     *
     * For the semantics of these properties, please refer to
     * cpixinit.h, to cpix_InitParams struct and its "methods".
     */
    class InitParams
    {
    private:
        //
        // private members
        //

        // for cpixreg.txt and automatic index paths
        std::string                        cpixDir_;


        // log related parameters
        std::string                        logFileBase_;

        Property<size_t, 
                 LOGSIZELIMIT>             logSizeLimit_;

        Property<size_t, 
                 LOGSIZECHECKRECURRENCY>   logSizeCheckRecurrency_;


        // an IdxDb instance can be idle up to this point until it is
        // flushed / and released (transparently)
        Property<size_t,
                 MAXIDLESEC>               maxIdleSec_;


        // Cpix::IdxDb internals
        Property<size_t,
                 MAXINSERTBUFSIZE>         maxInsertBufSize_;

        Property<size_t,
                 INSERTBUFMAXDOCS>         insertBufMaxDocs_;
        // TODO add & implement limit for deletes


        // jobqueue item sizes
        Property<size_t,
                 IDXJOBQUEUESIZE>         idxJobQueueSize_;

        Property<size_t,
                 QRYJOBQUEUESIZE>         qryJobQueueSize_;
        
        // thread prioritie deltas for the slave (worker threads,
        // async API)
        Property<int,
                 IDXTHREADPRIORITYDELTA>  idxThreadPriorityDelta_;

        Property<int,
                 QRYTHREADPRIORITYDELTA>  qryThreadPriorityDelta_;


        // page size for clucene hits (wrapper object)
        Property<size_t,
                 CLHITSPAGESIZE>          clHitsPageSize_;

    public:
        //
        // public operators
        //

        /**
         * Initializes this instance with the default values for each
         * parameters (property).
         */
        InitParams();
        

        const char * getCpixDir() const;
        void setCpixDir(const char * value);


        const char * getLogFileBase() const;
        void setLogFileBase(const char * value);

        
        size_t getLogSizeLimit() const;
        void setLogSizeLimit(size_t value);


        size_t getLogSizeCheckRecurrency() const;
        void setLogSizeCheckRecurrency(size_t value);

        
        size_t getMaxIdleSec() const;
        void setMaxIdleSec(size_t value);


        size_t getMaxInsertBufSize() const;
        void setMaxInsertBufSize(size_t value);


        size_t getInsertBufMaxDocs() const;
        void setInsertBufMaxDocs(size_t value);


        size_t getIdxJobQueueSize() const;
        void setIdxJobQueueSize(size_t value);

 
        size_t getQryJobQueueSize() const;
        void setQryJobQueueSize(size_t value);

        int getIdxThreadPriorityDelta() const;
        void setIdxThreadPriorityDelta(int value);

        int getQryThreadPriorityDelta() const;
        void setQryThreadPriorityDelta(int value);

        size_t getClHitsPageSize() const;
        void setClHitsPageSize(size_t value);
        
        const char * getCluceneLockDir() const;
        void setCluceneLockDir(const char * value);
    };




}


#endif // CPIX_INITPARAMS_H
