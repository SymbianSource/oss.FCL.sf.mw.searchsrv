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

#include "cpixtimetools.h"



namespace Cpt
{

    /****************************************************
     *
     *  TimeExc
     *
     */
    const char * TimeExc::what() const throw()
    {
        return "Could not get timestamp (gettimeofday)";
    }



    /****************************************************
     *
     *  GetTimeOfDay
     *
     */
    void GetTimeOfDay(struct timeval & tv)
    {
        int
            result = gettimeofday(&tv,
                                  NULL);

        if (result != 0)
            {
                throw TimeExc();
            }
    }



    long GetTimeOfDay()
    {
        struct timeval
            now;

        GetTimeOfDay(now);

        return now.tv_sec;
    }



    /****************************************************
     *
     *  StopperWatch
     *
     */
    StopperWatch::StopperWatch()
    {
        reset();
    }


    long StopperWatch::elapsedMSecs() const
    {
        enum 
        {
            MSECS_IN_SEC  = 1000,
            USECS_IN_MSEC = 1000
        };

        struct timeval
            now;

        GetTimeOfDay(now);

        long
            rv = (now.tv_sec - timestamp_.tv_sec) * MSECS_IN_SEC
            + (now.tv_usec - timestamp_.tv_usec) / USECS_IN_MSEC;

        return rv;
    }


    void StopperWatch::reset()
    {
        GetTimeOfDay(timestamp_);
    }
    
}
