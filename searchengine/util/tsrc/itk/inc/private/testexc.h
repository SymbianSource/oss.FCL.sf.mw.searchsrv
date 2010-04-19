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

#ifndef ITK_TESTEXC_H_
#define ITK_TESTEXC_H_

#include <exception>

#include "testevent.h"

namespace Itk
{

    /**
     * Exception class to controll the execution of test case
     * (hierarchies), namely to fall back to the closest Context on
     * failed assertion. In a sense, used for controll flow -
     * horribile dictu!
     */
    class TestExc : public std::exception
    {
    private:
        //
        // private members
        //

    public:
        //
        // public operators
        //


        virtual const char * what() const throw();
    };
}

#endif // ITK_TESTEXC_H_
