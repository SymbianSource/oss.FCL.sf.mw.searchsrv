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

#include <time.h>
#include <stdio.h>

#include "common/cpixlog.h"
#include "rotlogger.h"

enum cpix_LogLevel GlobalLogLevel;

namespace
{

    /**
     * Format for date/time string rendering, as well as the size of
     * the required of buffer.
     */
    // separators and terminator:     1    1     1    1    1    1 =  6
    //                                |    |     |    |    |    |
    //                                v    v     v    v    v    v
    const char TimeStrFormat[] = "%02d/%02d/%04d\t%02d:%02d:%02d";
    //                             ^   ^    ^     ^    ^    ^
    //                             |   |    |     |    |    |
    // rendered integers lengths:  2   2    4     2    2    2     = 14
    //                                                            ----
    enum { TIMESTR_MAXSIZE =                                        20 };
    


    //
    // implementation detail
    // 
    void logMsg_(cpix_LogLevel   logLevel,
                 bool            produceDateTime,
                 const char    * format,
                 va_list         ap)
    {
        if (logLevel <= GlobalLogLevel
            && Cpix::RotLogger::IsLoggingEnabled())
            {
                if (produceDateTime)
                    {
                        const char
                            * timeStr = "(Unknown time)";
                        char
                            timeBuf[TIMESTR_MAXSIZE];

                        time_t
                            now;

                        if (time(&now) > 0)
                            {
                                struct tm
                                    timeDetails;
                                if (localtime_r(&now, &timeDetails) != NULL)
                                    {
                                        snprintf(timeBuf,
                                                 sizeof(timeBuf),
                                                 TimeStrFormat,
                                                 timeDetails.tm_mday,
                                                 timeDetails.tm_mon + 1,
                                                 timeDetails.tm_year + 1900,
                                                 timeDetails.tm_hour,
                                                 timeDetails.tm_min,
                                                 timeDetails.tm_sec);
                                        timeStr = timeBuf;
                                    }
                            }

                        printf(timeStr);
                    }
    
                switch (logLevel)
                    {
                    case CPIX_LL_ERROR:
                        printf(" E ");
                        break;
                    case CPIX_LL_WARNING:
                        printf(" W ");
                        break;
                    case CPIX_LL_TRACE:
                        printf(" T ");
                        break;
                    case CPIX_LL_DEBUG:
                        printf(" D ");
                        break;
                    default:
                        printf(" ? ");
                    };

                vprintf(format,
                        ap);

                printf("\r\n");

                fflush(stdout);
            }
    }
}


void logMsg(cpix_LogLevel     logLevel,
            const char      * format,
            ...)
{
    va_list
        args;
    va_start(args,
             format);
    
    logMsg_(logLevel,
            true,
            format,
            args);
    
    va_end(args);
}


void logDbgMsg(const char * format,
               ...)
{
    va_list
        args;
    va_start(args,
             format);
    
    logMsg_(CPIX_LL_DEBUG,
            true,
            format,
            args);
    
    va_end(args);
}



void logTestMsg(cpix_LogLevel  logLevel,
                const char   * format,
                ...)
{
    va_list
        args;
    va_start(args,
             format);
    
    logMsg_(logLevel,
            false,
            format,
            args);
    
    va_end(args);
}
