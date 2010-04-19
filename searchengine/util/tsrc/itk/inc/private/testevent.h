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

#ifndef ITK_TESTEVENT_H_
#define ITK_TESTEVENT_H_

#include <iosfwd>
#include <string>

namespace Itk
{

    /**
     * POV class to store test event related information, like
     * expression that failed (or asserted), source file, line and
     * possible custom message.
     */
    class TestEvent
    {
    private:
        //
        // private members
        //
        std::string   expr_;
        std::string   file_;
        size_t        line_;
        std::string   msg_;

    public:
        //
        // public operators
        //

        /**
         * TODO
         *
         * To report failed expects and asserts
         *
         * @param msg - may be NULL
         */
        TestEvent(const char * expr,
                  const char * file,
                  size_t       line,
                  const char * msg);

        /**
         * TODO
         *
         * To report io capture related messages
         */
        TestEvent(const char * file,
                  const char * msg);

        
        /**
         * TODO
         *
         * To report unknown failures
         */
        TestEvent(const char * contextName);


        const std::string & expr() const;

        const std::string & file() const;

        size_t line() const;

        const std::string & msg() const;
    };
}


namespace std
{
    std::ostream & operator<<(std::ostream         & os,
                              const Itk::TestEvent & e);
}

#endif // ITK_TESTEVENT_H_
