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

#ifndef CPIX_CPIXEXC_H
#define CPIX_CPIXEXC_H

#include <exception>
#include <string>

/**
 * CPix internal exception class.
 */
class CpixExc /* OBS : public std::exception */
{
private:
    const wchar_t   * wWhat_;
    std::string       file_;
    size_t            line_;

public:
    /**
     * Constructs an exception class with the given message.
     *
     * @param wWhat the message, may be NULL. Its ownership is not
     * transferred. It is in fact a printf-like format string,
     * possibly followed by arguments.
     */
    CpixExc(const wchar_t * wWhat,
            const char    * file,
            size_t          line,
            ...);

    /**
     * Constructs an exception class with the given message.
     *
     * @param what the message, may be NULL. Its ownership is not
     * transferred. It is in fact a printf-like format string,
     * possibly followed by arguments.
     */
    CpixExc(const char    * what,
            const char    * file,
            size_t          line,
            ...);

    /**
     * Destroys this exception instance.
     */
    ~CpixExc();

    CpixExc(const CpixExc & that);

    CpixExc & operator=(const CpixExc & that);

    /**
     * Returns the specifics of the exception - string still owned
     * by this exception class.
     */
    const wchar_t * wWhat() const throw ();

    const char * file() const;

    size_t line() const;


private:
    void freewWhat();
};


#define THROW_CPIXEXC(m, args...) throw CpixExc(m, __FILE__, __LINE__, ##args)
#define PL_ERROR "Program Logic Error "


#endif /* CPIX_CPIXEXC_H */
