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
* Description: Main application class
*
*/

#include "itk.h"
#include "testexc.h"

namespace Itk
{

    void itk_expect(TestMgr     * testMgr,
                    bool          succeeded,
                    const char  * expr,
                    const char  * file,
                    size_t        line,
                    const char  * format,
                    ...)
    {
        char
            msg[256];

        va_list
            args;
        va_start(args,
                 format);

        vsnprintf(msg,
                  sizeof(msg),
                  format,
                  args);

        va_end(args);

        testMgr->expecting(succeeded,
                           expr,
                           file,
                           line,
                           msg);
    }


    void itk_assert(TestMgr     * testMgr,
                    bool          succeeded,
                    const char  * expr,
                    const char  * file,
                    size_t        line,
                    const char  * format,
                    ...)
    {
        char
            msg[256];

        va_list
            args;
        va_start(args,
                 format);

        vsnprintf(msg,
                  sizeof(msg),
                  format,
                  args);

        va_end(args);

        testMgr->asserting(succeeded,
                           expr,
                           file,
                           line,
                           msg);

        if (!succeeded)
            {
                throw TestExc();
            } 
    }


    void itk_msg(TestMgr     * testMgr,
                 const char  * file,
                 size_t        line,
                 const char  * format,
                 ...)
    {
        char
            msg[256];

        va_list
            args;
        va_start(args,
                 format);

        vsnprintf(msg,
                  sizeof(msg),
                  format,
                  args);

        va_end(args);

        testMgr->msg(file,
                     line,
                     msg);
    }


    void itk_dbgMsg(TestMgr     * testMgr,
                    const char  * format,
                    ...)
    {
        char
            msg[256];

        va_list
            args;
        va_start(args,
                 format);

        vsnprintf(msg,
                  sizeof(msg),
                  format,
                  args);

        va_end(args);

        testMgr->dbgMsg(msg);
    }


    void itk_report(TestMgr     * testMgr,
                    const char  * name,
                    const char  * format,
                    ...)
    {
        char
            msg[256];

        va_list
            args;
        va_start(args,
                 format);

        vsnprintf(msg,
                  sizeof(msg),
                  format,
                  args);

        va_end(args);

        testMgr->report(name,
                        msg);
    }



    void getTimestamp(Timestamp * timestamp)
    {
        g_get_current_time(timestamp);
    }


    long getElapsedMs(Timestamp * later,
                      Timestamp * earlier)
    {
        long
            millis = ((later->tv_sec - earlier->tv_sec)*1000) 
            + ((later->tv_usec - earlier->tv_usec)/1000);

        return millis;
    }
    
}
