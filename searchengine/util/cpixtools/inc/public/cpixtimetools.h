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
#ifndef CPIXTIMETOOLS_H_
#define CPIXTIMETOOLS_H_

#include <sys/time.h>

#include <exception>

namespace Cpt
{


    class TimeExc : public std::exception
    {
    public:
        virtual const char * what() const throw();
    };


    
    /**
     * Utility function to get the secs and microseconds since the
     * epoc. It throws TimeExc on failure.
     */
    void GetTimeOfDay(struct timeval & tv);



    /**
     * Utility function to get the secs since the epoc. It throws
     * TimeExc on failure.
     */
    long GetTimeOfDay();



    /**
     * Utility class for measuring the elapsed time, in ??? seconds
     * since its construction.
     */
    class StopperWatch
    {
    private:
        struct timeval    timestamp_;

    public:

        /**
         * Constructs taking a timestamp. Throws exception if fails
         * taking timestamp.
         */
        StopperWatch();

        /**
         * @returns the elapcsed milliseconds since the construction
         * of this instance.
         */
        long elapsedMSecs() const;


        /**
         * Resets this stopper watch, calls to elapsedMSecs() will
         * return time elapsed since "now".
         */
        void reset();
    };
}



#endif
