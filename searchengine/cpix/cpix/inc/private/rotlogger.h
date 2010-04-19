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

#ifndef CPIX_ROTLOGGER_H
#define CPIX_ROTLOGGER_H

#include "cpixsynctools.h"

// TODO turn it tunable (initparams.h - cpixinit.h)
#define LOGFILE_NUM   5
#define LOG_SUFFIX    "_N_log.txt"
//                      ^
//                      |
//                    +-+
//                    |
#define LOGIDX_POS    1


namespace Cpix
{


    /**
     * Class implementing the rotating logger functionality -
     * redirecting std out and err to log files. This is a singleton
     * class.
     */
    class RotLogger
    {
    private:
        //
        // private
        //
        std::string         logFileBase_;
        size_t              logSizeLimit_;
        size_t              logSizeCheckRecurrency_;
        int logFd_;
        int curLogIdx_;
        size_t              logCount_;
        

        static RotLogger  * instance_;
        static bool         isLoggingEnabled_;

        int                 duplicatedStdOut_;
        int                 duplicatedStdErr_;

        Cpt::Mutex          mutex_;

    public:
        //
        // public operations
        //

        /**
         * Not thread safe (and it should not need to be), as it does
         * not modify the pointer once it is created, which should
         * happen anyway during init (by a call to createLog()).
         */
        static RotLogger * instance();


        /**
         * Not thread safe (and it should not need to be), as it is
         * invoked during library shutdown.
         */
        static void shutdownAll();


        /**
         * Not thread safe (and it should not need to be), as it is
         * invoked during library init.
         *
         * @param logFileBase the path and prefix of the logfiles to
         * which the changing (rotating) suffix is appended. May be
         * NULL, in which case logging is not redirected (not
         * enabled).
         *
         * @param logSizeLimit an approximate limit to log file sizes,
         * after which logs should be redirected to the next rotating
         * log file. Ignored if logFileBase was NULL.
         *
         * @param logSizeCheckRecurrency how frequently the size of
         * the currently used logfile is to be checked. The more
         * frequent, the more accurately the parameter logSizeLimit
         * will be observed, but at a perfomance cost. Ignored if
         * logFileBase was NULL.
         */
        void createLog(const char * logFileBase,
                       size_t       logSizeLimit,
                       size_t       logSizeCheckRecurrency);


        /**
         * Thread-safe.
         */
        void rotateLog() throw();


        /**
         * @returns true if logging is enabled (logFileBase given to
         * createLog() method was NOT NULL and initialization
         * succeeded).
         */
        static bool IsLoggingEnabled();


        /**
         * Enables / disables logging to STDOUT.
         *
         * Initially, logging is enabled / disabled based on whether
         * we do log redirection or not. If there is no redirection of
         * STDOUT to logfile, then there is no logging enabled to
         * STDOUT either. However, some test code may need STDOUT
         * logging and not the redirected logging.
         */
        static void SetLoggingEnabled(bool b);


    private:
        //
        // private methods
        //
        RotLogger();
        ~RotLogger();


        std::string getLogFile(bool current);
            
        
        int getMostRecentlyModifiedLogIdx();


        void redirectStdOutErr();


        /**
         * @returns if closing all descriptors have succeeded
         */
        bool closeFds();


    };




}

#endif
