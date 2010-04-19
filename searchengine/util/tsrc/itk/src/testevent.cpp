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
#include <iostream>

#include "testevent.h"


namespace
{
    const char UNKNOWN[] = "(unknown)";
}


namespace Itk
{

    TestEvent::TestEvent(const char * expr,
              const char * file,
              size_t       line,
              const char * msg)
        : expr_(expr),
          file_(file),
          line_(line),
          msg_(msg == NULL ? "" : msg)
    {
        ;
    }


    TestEvent::TestEvent(const char * file,
              const char * msg)
        : expr_(""),
          file_(file),
          line_(0),
          msg_(msg)
    {
        ;
    }

    
    TestEvent::TestEvent(const char * contextName)
        : expr_(UNKNOWN),
          file_(UNKNOWN),
          line_(0),
          msg_(contextName)
          
    {
        ;
    }


    const std::string & TestEvent::expr() const
    {
        return expr_;
    }


    const std::string & TestEvent::file() const
    {
        return file_;
    }


    size_t TestEvent::line() const
    {
        return line_;
    }


    const std::string & TestEvent::msg() const
    {
        return msg_;
    }

}



namespace std
{
    std::ostream & operator<<(std::ostream         & os,
                              const Itk::TestEvent & e)
    {
        using namespace std;

        if (e.expr().length() > 0 && e.file().length() > 0)
            {
                // some sort of test failure
                os 
                    << "Expr "
                    << e.expr() 
                    << " failed at "
                    << e.file()
                    << "/"
                    << e.line()
                    << ": "
                    << e.msg();
            }
        else if (e.expr().length() == 0 && e.file().length() > 0)
            {
                // io capture message
                os
                    << "IO Capturing: "
                    << e.file()
                    << ": "
                    << e.msg();
            }
        else
            {
                // unknown failure
                os
                    << "Unknown failure in context "
                    << e.msg();
            }

        return os;
    }
}
