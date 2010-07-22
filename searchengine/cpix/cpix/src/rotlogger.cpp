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
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "cpixtools.h"

#include "cpixexc.h"

#include <string>

#include "cpixfstools.h"

#include "cpixexc.h"
#include "rotlogger.h"

#include "common/cpixlog.h"

namespace Cpix
{

    RotLogger * RotLogger::instance() 
    {
        if (instance_ == NULL)
            {
                instance_ = new RotLogger();
            }

        return instance_;
    }


    void RotLogger::shutdownAll()
    {
        delete instance_;
        instance_ = NULL;
    }


    void RotLogger::createLog(const char * logFileBase,
                              size_t       logSizeLimit,
                              size_t       logSizeCheckRecurrency)
    {
        using namespace std;

        if (logFileBase == NULL)
            {
                return;
            }

        logFileBase_ = logFileBase;
        logSizeLimit_ = logSizeLimit;
        logSizeCheckRecurrency_ = logSizeCheckRecurrency;

        string
            logFile = getLogFile(true); // current

        Cpt_EINTR_RETRY(logFd_,
                        open(logFile.c_str(),
                             O_CREAT | O_APPEND | O_WRONLY,
                             0666));
        if (logFd_ == -1)
            {
                THROW_CPIXEXC("Could not open file '%s'",
                              logFile.c_str());
            }
        Cpt::FileDescSentry
            fds(&logFd_);

        duplicatedStdOut_ = dup(STDOUT_FILENO);
        if (duplicatedStdOut_ == -1)
            {
                THROW_CPIXEXC("Cannot dup() STDOUT_FILENO");
            }
        duplicatedStdErr_ = dup(STDERR_FILENO);
        if (duplicatedStdErr_ == -1)
            {
                THROW_CPIXEXC("Cannot dup() STDERR_FILENO");
            }

        int
            result;
        Cpt_EINTR_RETRY(result,
                        close(STDOUT_FILENO));
        Cpt_EINTR_RETRY(result,
                        close(STDERR_FILENO));
            
        redirectStdOutErr();

        fds.release();

        isLoggingEnabled_ = true;
    }


    void RotLogger::rotateLog() throw()
    {
        using namespace std;

        Cpt::SyncRegion
            sr(mutex_);

        if (logFd_ == -1)
            {
                return;
            }

        bool
            doNothing = true; 
        ++logCount_;
        if (logCount_ == logSizeCheckRecurrency_)
            {
                logCount_ = 0;
                doNothing = Cpt::filesize(logFd_) <= logSizeLimit_;
            }

        if (doNothing)
            {
                return;
            }

        string
            logFile = getLogFile(false); // false: next
        int
            logFd;
        Cpt_EINTR_RETRY(logFd,
                        open(logFile.c_str(),
                             O_CREAT | O_TRUNC | O_WRONLY,
                             0666));
        if (logFd == -1)
            {
                logMsg(CPIX_LL_ERROR,
                       "Could not open next log: %s. CLOSING LOGS.\n",
                       logFile.c_str());
            }

        bool
            closedAll = closeFds();

        if (logFd != -1 && closedAll)
            {
                logFd_ = logFd;
                redirectStdOutErr();
            }
    }



    bool RotLogger::IsLoggingEnabled()
    {
        return isLoggingEnabled_;
    }
            


    void RotLogger::SetLoggingEnabled(bool b)
    {
        isLoggingEnabled_ = b;
    }



    RotLogger::RotLogger()
        : logSizeLimit_(0),
          logSizeCheckRecurrency_(0),
          logFd_(-1),
          curLogIdx_(-1),
          logCount_(0),
          duplicatedStdOut_(-1),
          duplicatedStdErr_(-1)
    {
        ;
    }



    RotLogger::~RotLogger()
    {
        if (logFd_ == -1)
            {
                return;
            }

        closeFds();
    }



    std::string RotLogger::getLogFile(bool current)
    {
        if (current)
            {
                curLogIdx_ = getMostRecentlyModifiedLogIdx();
            }
        else
            {
                ++curLogIdx_;
                curLogIdx_ %= LOGFILE_NUM;
            }

        std::string
            rv(logFileBase_);
        rv += LOG_SUFFIX;
        rv[logFileBase_.length() + LOGIDX_POS] 
            = static_cast<char>('0' + curLogIdx_);

        return rv;
    }
            
        
    int RotLogger::getMostRecentlyModifiedLogIdx()
    {
        int
            curLogIdx = 0;
        time_t
            mostRecentlyModified = 0;

        Cpt::auto_array<char>
            path(new char[logFileBase_.length()
                          + strlen(LOG_SUFFIX)
                          + 1]);
        strcpy(path.get(),
               logFileBase_.c_str());
        strcpy(path.get() + logFileBase_.length(),
               LOG_SUFFIX);
        int
            pos = logFileBase_.length() + LOGIDX_POS;
        for (int i = 0; i < LOGFILE_NUM; ++i)
            {
                *(path.get() + pos) = static_cast<char>('0' + i);
                time_t
                    modified = Cpt::filemodified(path.get());

                if (modified > mostRecentlyModified)
                    {
                        mostRecentlyModified = modified;
                        curLogIdx = i;
                    }
            }
        return curLogIdx;
    }


    void RotLogger::redirectStdOutErr()
    {
        int
            newStdOut = dup2(logFd_,
                             STDOUT_FILENO);
        if (newStdOut == -1 || newStdOut != STDOUT_FILENO)
            {
                ;
            }
        else
            {
                int
                    newStdErr = dup2(logFd_,
                                     STDERR_FILENO);
                if (newStdErr == -1 || newStdErr != STDERR_FILENO)
                    {
                        Cpt::FileDescSentry
                            fdsDummy(&newStdOut);
                    }
            }
    }


    bool RotLogger::closeFds()
    {
        int
            result;
        bool
            rv = true;

        result = fflush(stdout);
        rv = result != -1;

        result = fflush(stderr);
        rv &= result != -1;

        Cpt_EINTR_RETRY(result,
                        close(logFd_));
        rv &= result != -1;
        logFd_ = -1;

        Cpt_EINTR_RETRY(result,
                        close(STDOUT_FILENO));
        rv &= result != -1;

        Cpt_EINTR_RETRY(result,
                        close(STDERR_FILENO));
        rv &= result != -1;

        return rv;
    }



    RotLogger * RotLogger::instance_         = NULL;
    bool        RotLogger::isLoggingEnabled_ = false;

}

