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


#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "cpixexc.h"
#include "initparams.h"

static const wchar_t EMPTY_MSG[]       
= L"";

static const wchar_t LFAILED_CTOR_MSG[] 
= L"(Failed to alloc exception message)";

CpixExc::CpixExc(const wchar_t * wWhat,
                 const char    * file,
                 size_t          line,
                 ...)
    : wWhat_(EMPTY_MSG),
      file_(file),
      line_(line)
{
    if (wWhat != NULL)
        {
            wWhat_ = LFAILED_CTOR_MSG;

            wchar_t
                tmpBuf[Cpix::EXCMSG_BUFSIZE];

            va_list
                args;
            va_start(args,
                     line);
            int
                result = vsnwprintf(tmpBuf,
                                    sizeof(tmpBuf)/sizeof(wchar_t),
                                    wWhat,
                                    args);
            va_end(args);

            if (result > 0)
                {
                    wWhat_ = wcsdup(tmpBuf);
                    if (wWhat_ == NULL)
                        {
                            wWhat_ = LFAILED_CTOR_MSG;
                        }
                }
        }
}


CpixExc::CpixExc(const char * what,
                 const char * file,
                 size_t       line,
                 ...)
    : wWhat_(EMPTY_MSG),
      file_(file),
      line_(line)
{
    if (what != NULL)
        {
            wWhat_ = LFAILED_CTOR_MSG;

            char
                tmpBuf[Cpix::EXCMSG_BUFSIZE];

            va_list
                args;
            va_start(args,
                     line);
            int
                result = vsnprintf(tmpBuf,
                                   sizeof(tmpBuf)/sizeof(char),
                                   what,
                                   args);
            va_end(args);
            
            if (result > 0)
                {
                    size_t
                        bufSize = strlen(tmpBuf) + 1;
                    wWhat_ = (wchar_t*)malloc(bufSize * sizeof(wchar_t));
                    if (wWhat_ != NULL)
                        {
                            mbstowcs(const_cast<wchar_t*>(wWhat_),
                                     tmpBuf,
                                     bufSize);
                            const_cast<wchar_t*>(wWhat_)[bufSize-1] = L'\0';
                        }
                    else
                        {
                            wWhat_ = LFAILED_CTOR_MSG;
                        }
                }
        }
}



CpixExc::~CpixExc()
{
    freewWhat();
}

    
CpixExc::CpixExc(const CpixExc & that)
    : wWhat_(EMPTY_MSG),
      line_(0)
{
    if (that.wWhat_ == EMPTY_MSG
        || that.wWhat_ == LFAILED_CTOR_MSG)
        {
            wWhat_ = that.wWhat_;
        }
    else
        {
            wWhat_ = wcsdup(that.wWhat_);
            if (wWhat_ == NULL)
                {
                    wWhat_ = LFAILED_CTOR_MSG;
                }
        }

    file_ = that.file_;
    line_ = that.line_;
}
    

CpixExc & CpixExc::operator=(const CpixExc & that)
{
    if (this != &that)
        {
            const wchar_t
                * newwWhat = NULL;

            if (that.wWhat_ == EMPTY_MSG
                || that.wWhat_ == LFAILED_CTOR_MSG)
                {
                    newwWhat = that.wWhat_;
                }
            else
                {
                    newwWhat = wcsdup(that.wWhat_);
                    if (newwWhat == NULL)
                        {
                            newwWhat = LFAILED_CTOR_MSG;
                        }
                }

            freewWhat();
            wWhat_ = newwWhat;

            file_ = that.file_;
            line_ = that.line_;
        }

    return *this;
}
    

const wchar_t * CpixExc::wWhat() const throw()
{
    return wWhat_;
}


const char * CpixExc::file() const
{
    return file_.c_str();
}

size_t CpixExc::line() const
{
    return line_;
}



void CpixExc::freewWhat()
{
    if (wWhat_ != NULL
        && wWhat_ != EMPTY_MSG
        && wWhat_ != LFAILED_CTOR_MSG)
        {
            free(const_cast<wchar_t*>(wWhat_));
        }
    wWhat_ = NULL;
}

