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

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <glib.h>

#include <exception>

#include "cpixsyncpool.h"

#include "cpixerror.h"
#include "cpixthreadlocaldata.h"
#include "common/cpixlog.h"
#include "initparams.h"

#include "CLucene.h"


wchar_t ET_STD_CPP_EXC_NAME[]      = L"Std C++ exception";
wchar_t ET_CPIX_EXC_NAME[]         = L"CPix exception";
wchar_t ET_CLUCENE_EXC_NAME[]      = L"CLucene exception";
wchar_t ET_UNKNOWN_EXC_NAME[]      = L"Unknown exception";
wchar_t ET_CPTSYNTAX_EXC_NAME[]    = L"Syntax exception";
wchar_t ET_CPT_EXC_NAME[]          = L"Cpt exception";
wchar_t ET_OS_EXC_NAME[]           = L"OS exception";

const wchar_t * GetErrorTypeName(cpix_ErrorType errorType)
    {
        wchar_t
            * rv = ET_UNKNOWN_EXC_NAME;

        if (errorType & ET_STD_CPP_EXC)
            {
                rv = ET_STD_CPP_EXC_NAME;
            }
        else if (errorType & ET_CPIX_EXC)
            {
                rv = ET_CPIX_EXC_NAME;
            }
        else if (errorType & ET_CLUCENE_EXC)
            {
                rv = ET_CLUCENE_EXC_NAME;
            }
        else if (errorType == ET_CPTSYNTAX_EXC)
            {
                rv = ET_CPTSYNTAX_EXC_NAME;
            }
        else if (errorType & ET_CPT_EXC)
            {
                rv = ET_CPT_EXC_NAME;
            }
        else if (errorType & ET_OS_EXC)
            {
                rv = ET_OS_EXC_NAME;
            }

        return rv;
    }



/**
 * This class is used for relaying error messages for asynchronous
 * calls. It derives from cpix_Error, and thus IS-A a cpix_Error, and
 * can be used with the rest of the API.
 *
 * Rationale: an error status / message must be available as long as
 * the client does not collect the results of an async call (or has
 * cancelled it).
 */
struct ErrorInfo : public cpix_Error
{
private:
    //
    // private members
    //
    wchar_t      buf_[Cpix::ERRORMSG_BUFSIZE];


public:
    //
    // public operators
    //

    /**
     * Sets the error type (type_ in parent class) to the given type
     * and the message (msg_ in parent class) to NULL.
     *
     * @param errorType the error type to set
     */
    void setInfo(cpix_ErrorType errorType)
    {
        setInfo(errorType,
                static_cast<const wchar_t*>(NULL));
    }


    /**
     * Sets the error type (type_ in parent class) to the given type
     * and message (msg_ in parent class) to the given message.
     *
     *
     * @param errorType the error type to set
     *
     * @param msg the detailed message of the error
     */
    void setInfo(cpix_ErrorType   errorType,
                 const wchar_t  * msg)
    {
        type_ = errorType;
        if (msg == NULL)
            {
                msg_ = NULL;
            }
        else
            {
                wcsncpy(buf_,
                        msg,
                        sizeof(buf_)/sizeof(wchar_t) - 1);
                buf_[sizeof(buf_)/sizeof(wchar_t) - 1] 
                    = wchar_t(0);
                msg_ = buf_;
            }
        impl_ = this;
    }

    
    /**
     * Same as overloaded method, but for char instead of wchar_t.
     */
    void setInfo(cpix_ErrorType     errorType,
                 const char       * msg)
    {
        type_ = errorType;
        if (msg == NULL)
            {
                msg_ = NULL;
            }
        else
            {
                mbstowcs(buf_,
                         msg,
                         sizeof(buf_)/sizeof(wchar_t) - 1);
                buf_[sizeof(buf_)/sizeof(wchar_t) - 1] 
                    = wchar_t(0);
                msg_ = buf_;
            }
        impl_ = this;
    }


    /**
     * Constructor (default). In fact, it initializes the public
     * members of the super class (which is a C struct part of the
     * public API).
     */
    ErrorInfo()
    {
        type_ = ET_UNKNOWN_EXC;
        msg_ = NULL;
        impl_ = this;
    }
};



/**
 * This error info instance is used with ALL synchronous API calls,
 * but never with asynchronous API calls.
 *
 * NOTE: This C++ instance has in practice a problem getting
 * constructed properly with the ARM rvctc 2.2 compiler, but it should
 * not matter - an error info should be set up (setInfo) before use.
 */
ErrorInfo SyncErrorInfo;


/**
 * A singleton pool of ErrorInfo objects that are used only with
 * asynchronous calls, never with synchronous calls.
 *
 * TODO as a debug measure, set a max limit, and warn/assert when it
 * is exceeded, to pinpoint leaking error infos.
 */
class ErrorInfoPool : public Cpt::SyncPool<ErrorInfo>
{
private:
    //
    // private members
    //

    static ErrorInfoPool      * instance_;

public:
    //
    // public operators
    //
    ErrorInfoPool()
        : Cpt::SyncPool<ErrorInfo>(Cpix::MIN_ERRORINFO_COUNT)
    {
        ;
    }


    static ErrorInfoPool * instance()
    {
        if (instance_ == NULL)
            {
                instance_  = new ErrorInfoPool;
            }

        return instance_;
    }
    
    static void shutdown()
    {
        delete instance_;
        instance_ = NULL; 
    }
    
};

namespace Cpix {

    void InitErrorInfoPool()
    {
        ErrorInfoPool::instance();
    }


    void ShutdownErrorInfoPool()
    {
        ErrorInfoPool::shutdown();
    }
}



ErrorInfoPool * ErrorInfoPool::instance_ = NULL;



ErrorInfo * AcquireErrorInfo()
{
    bool
        isSlaveThread = false;

    void
        * key = pthread_getspecific(Cpix::ThreadLocalDataKey);

    if (key == NULL)
        {
            logMsg(CPIX_LL_ERROR,
                   "No ThreadLocalData in this thread (cpix_init() not called?)");
        }
    else
        {
            Cpix::ThreadLocalData
                * tld = reinterpret_cast<Cpix::ThreadLocalData*>(key);
            isSlaveThread = tld->isSlaveThread();
        }
            
    ErrorInfo
        * rv = NULL;

    if (isSlaveThread)
        {
            // for slave threads (async api), we must use an error
            // info instance from the pool that noone else is using
            rv = ErrorInfoPool::instance()->acquire();
        }
    else
        {
            // for master thread (sync api), we must use the global
            // instance
            rv = & SyncErrorInfo;
        }

    return rv;
}


void ReleaseErrorInfo(ErrorInfo * errorInfo)
{
    if (errorInfo != &SyncErrorInfo)
        {
            ErrorInfoPool::instance()->release(errorInfo);
        }
    else
        {
            ; // SyncErrorInfo is static, nothing to release about it
        }
}



cpix_Error * CreateError(cpix_ErrorType   errorType,
                         const wchar_t  * msg)
{
    ErrorInfo
        * errorInfo = NULL;

    try 
        {
            errorInfo = AcquireErrorInfo();
            errorInfo->setInfo(errorType,
                               msg);
        }
    catch (...)
        {
            logMsg(CPIX_LL_ERROR,
                   "!!! Failed to create error struct for type %d ...",
                   errorType);
            logMsg(CPIX_LL_ERROR,
                   "!!! ... (cont'd) msg: %S. Will report CPIX error.",
                   msg == NULL ? L"(null)" : msg);
            
            errorInfo = & SyncErrorInfo;
            errorInfo->setInfo(ET_CPIX_EXC);
        }

    cpix_Error
        * rv = static_cast<cpix_Error*>(errorInfo);

    return rv;
}



cpix_Error * CreateError(cpix_ErrorType    errorType,
                         const char      * msg)
{
    ErrorInfo
        * errorInfo = NULL;

    try
        {
            errorInfo = AcquireErrorInfo();
            errorInfo->setInfo(errorType,
                               msg);
        }
    catch (...)
        {
            logMsg(CPIX_LL_ERROR,
                   "!!! Failed to create error struct for type %d ...",
                   errorType);
            logMsg(CPIX_LL_ERROR,
                   "!!! ... (cont'd) msg: %s. Will report CPIX error.",
                   msg == NULL ? "(null)" : msg);

            errorInfo = & SyncErrorInfo;
            errorInfo->setInfo(ET_CPIX_EXC);
        }

    cpix_Error
        * rv = static_cast<cpix_Error*>(errorInfo);

    return rv;
}



void cpix_Error_report(cpix_Error    * thisError,
                       wchar_t       * target,
                       size_t          targetLength)
{

    if (thisError != NULL)
        {
            const wchar_t
                * errorTypeName = GetErrorTypeName(thisError->type_);
            const wchar_t
                * detail = thisError->msg_;

            if (detail == NULL)
                {
                    detail = L"No details";
                }

            snwprintf(target,
                      targetLength,
                      L"Error type: %S. %S\n",
                      errorTypeName,
                      detail);
        }
    else
        {
		if (target)
            target = 0;
        }
}



cpix_Error * cpix_Error_destroy(cpix_Error * thisError)
{
    if (thisError != NULL)
        {
            ReleaseErrorInfo(reinterpret_cast<ErrorInfo*>(thisError->impl_));
        }

    return NULL;
}

