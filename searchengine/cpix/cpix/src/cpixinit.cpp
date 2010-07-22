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

#include "cpixerror.h"
#include "cpixinit.h"

#include "CLucene.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"

#ifdef _DEBUG
#include <memory>
#include "cpixdoc.h"
#endif

#include "idxdb.h"
#include "initparams.h"
#include "cpixutil.h"

#include "common/cpixlog.h"

//
//             cpix_InitParams related functions

cpix_InitParams * cpix_InitParams_create(cpix_Result * result)
{
    using namespace Cpix;

    cpix_InitParams
        * rv = NULL;

    rv = Create(result,
                CallCtor(rv));

    return rv;
}


void cpix_InitParams_destroy(cpix_InitParams * thisIp)
{
    using namespace Cpix;

    DestroyWrapper(thisIp);
}


const char * cpix_InitParams_getCpixDir(cpix_InitParams * thisIp)
{
    using namespace Cpix;

    return XlateExc(thisIp,
                    Caller(thisIp,
                               &InitParams::getCpixDir));
}


void cpix_InitParams_setCpixDir(cpix_InitParams * thisIp,
                                const char      * value)
{
    using namespace Cpix;

    XlateExc(thisIp,
             Caller(thisIp,
                        &InitParams::setCpixDir,
                        value));
}

const char * cpix_InitParams_getResourceDir(cpix_InitParams * thisIp)
{
    using namespace Cpix;

    return XlateExc(thisIp,
                    Caller(thisIp,
                               &InitParams::getResourceDir));
}


void cpix_InitParams_setResourceDir(cpix_InitParams * thisIp,
									const char      * value)
{
    using namespace Cpix;

    XlateExc(thisIp,
             Caller(thisIp,
                        &InitParams::setResourceDir,
                        value));
}


const char * cpix_InitParams_getLogFileBase(cpix_InitParams * thisIp)
{
    using namespace Cpix;

    return XlateExc(thisIp,
                    Caller(thisIp,
                               &InitParams::getLogFileBase));
}


void cpix_InitParams_setLogFileBase(cpix_InitParams * thisIp,
                                    const char      * value)
{
    using namespace Cpix;

    XlateExc(thisIp,
             Caller(thisIp,
                        &InitParams::setLogFileBase,
                        value));
}

    
size_t cpix_InitParams_getLogSizeLimit(cpix_InitParams * thisIp)
{
    using namespace Cpix;

    return XlateExc(thisIp,
                    Caller(thisIp,
                               &InitParams::getLogSizeLimit));
}


void cpix_InitParams_setLogSizeLimit(cpix_InitParams * thisIp,
                                     size_t            value)
{
    using namespace Cpix;

    XlateExc(thisIp,
             Caller(thisIp,
                        &InitParams::setLogSizeLimit,
                        value));
}



size_t cpix_InitParams_getLogSizeCheckRecurrency(cpix_InitParams * thisIp)
{
    using namespace Cpix;

    return XlateExc(thisIp,
                    Caller(thisIp,
                               &InitParams::getLogSizeCheckRecurrency));
}


void cpix_InitParams_setLogSizeCheckRecurrency(cpix_InitParams * thisIp,
                                               size_t            value)
{
    using namespace Cpix;

    XlateExc(thisIp,
             Caller(thisIp,
                        &InitParams::setLogSizeCheckRecurrency,
                        value));
}



size_t cpix_InitParams_getMaxIdleSec(cpix_InitParams * thisIp)
{
    using namespace Cpix;

    return XlateExc(thisIp,
                    Caller(thisIp,
                               &InitParams::getMaxIdleSec));
}


void cpix_InitParams_setMaxIdleSec(cpix_InitParams * thisIp,
                                   size_t            value)
{
    using namespace Cpix;

    XlateExc(thisIp,
             Caller(thisIp,
                        &InitParams::setMaxIdleSec,
                        value));
}



size_t cpix_InitParams_getMaxInsertBufferSize(cpix_InitParams * thisIp)
{
    using namespace Cpix;

    return XlateExc(thisIp,
                    Caller(thisIp,
                               &InitParams::getMaxInsertBufSize));
}


void cpix_InitParams_setMaxInsertBufferSize(cpix_InitParams * thisIp,
                                            size_t            value)
{
    using namespace Cpix;

    XlateExc(thisIp,
             Caller(thisIp,
                        &InitParams::setMaxInsertBufSize,
                        value));
}



size_t cpix_InitParams_getInsertBufMaxDocs(cpix_InitParams * thisIp)
{
    using namespace Cpix;

    return XlateExc(thisIp,
                    Caller(thisIp,
                               &InitParams::getInsertBufMaxDocs));
}


void cpix_InitParams_setInsertBufMaxDocs(cpix_InitParams * thisIp,
                                         size_t            value)
{
    using namespace Cpix;

    XlateExc(thisIp,
             Caller(thisIp,
                        &InitParams::setInsertBufMaxDocs,
                        value));
}



size_t cpix_InitParams_getIdxJobQueueSize(cpix_InitParams * thisIp)
{
    using namespace Cpix;

    return XlateExc(thisIp,
                    Caller(thisIp,
                               &InitParams::getIdxJobQueueSize));
}


void cpix_InitParams_setIdxJobQueueSize(cpix_InitParams * thisIp,
                                        size_t            value)
{
    using namespace Cpix;

    XlateExc(thisIp,
             Caller(thisIp,
                        &InitParams::setIdxJobQueueSize,
                        value));
}



size_t cpix_InitParams_getQryJobQueueSize(cpix_InitParams * thisIp)
{
    using namespace Cpix;

    return XlateExc(thisIp,
                    Caller(thisIp,
                               &InitParams::getQryJobQueueSize));
}


void cpix_InitParams_setQryJobQueueSize(cpix_InitParams * thisIp,
                                        size_t            value)
{
    using namespace Cpix;

    XlateExc(thisIp,
             Caller(thisIp,
                        &InitParams::setQryJobQueueSize,
                        value));
}


int cpix_InitParams_getIdxThreadPriorityDelta(cpix_InitParams * thisIp)
{
    using namespace Cpix;

    return XlateExc(thisIp,
                    Caller(thisIp,
                               &InitParams::getIdxThreadPriorityDelta));
}


void cpix_InitParams_setIdxThreadPriorityDelta(cpix_InitParams * thisIp,
                                               int            value)
{
    using namespace Cpix;

    XlateExc(thisIp,
             Caller(thisIp,
                        &InitParams::setIdxThreadPriorityDelta,
                        value));
}


int cpix_InitParams_getQryThreadPriorityDelta(cpix_InitParams * thisIp)
{
    using namespace Cpix;

    return XlateExc(thisIp,
                    Caller(thisIp,
                               &InitParams::getQryThreadPriorityDelta));
}

void cpix_InitParams_setQryThreadPriorityDelta(cpix_InitParams * thisIp,
                                               int            value)
{
    using namespace Cpix;

    XlateExc(thisIp,
             Caller(thisIp,
                        &InitParams::setQryThreadPriorityDelta,
                        value));
}


size_t cpix_InitParams_getClHitsPageSize(cpix_InitParams * thisIp)
{
    using namespace Cpix;

    return XlateExc(thisIp,
                    Caller(thisIp,
                               &InitParams::getClHitsPageSize));
}


void cpix_InitParams_setClHitsPageSize(cpix_InitParams * thisIp,
                                       size_t            value)
{
    using namespace Cpix;

    XlateExc(thisIp,
             Caller(thisIp,
                        &InitParams::setClHitsPageSize,
                        value));
}


const char * cpix_InitParams_getCluceneLockDir(cpix_InitParams * thisIp)
{
    using namespace Cpix;

    return XlateExc(thisIp,
                    Caller(thisIp,
                               &InitParams::getCluceneLockDir));
}


void cpix_InitParams_setCluceneLockDir(cpix_InitParams * thisIp,
                                const char      * value)
{
    using namespace Cpix;

    XlateExc(thisIp,
             Caller(thisIp,
                        &InitParams::setCluceneLockDir,
                        value));
}





//
//
//             CPIX LIBRARY FUNCTIONS
//




void cpix_init(cpix_Result     * result,
               cpix_InitParams * initParams)
{
    using namespace Cpix;

    XlateExc(result,
             CallFreeFunc(&IIdxDb::init,
                          Cast2Native(initParams)));

}


void cpix_shutdown()
{
    Cpix::IdxDb::shutdownAll();
}



const char * cpix_version()
{
    // TODO IMPLEMENT THIS PROPERLY
    //
    // o figure out what SVN command line client to use, see
    // http://cruisecontrolrb.thoughtworks.com/documentation/getting_started
    //
    // o do custom makefile steps, see
    // http://developer.symbian.com/forum/thread.jspa?messageID=56999
    // check also S60 examples, for example
    // S60_3rd_FP2_SDK_v1.1\S60CppExamples\AddressBook\group\Icons_scalable_dc.mk
    // S60_3rd_FP2_SDK_v1.1\S60CppExamples\AddressBook\group\AddressBook.mmp
    //
    // o point custom build step that gets the current revision and
    //   the version number from package file and creates a header
    //   with the proper C string literals
    return "CPix v0.1.0 rev??? (TODO)";
}



void cpix_doHousekeeping()
{
    using namespace Cpix;

    cpix_Result
        result;

    XlateExc(&result,
             CallFreeFunc(&IIdxDb::doHousekeepingOnAll));
}


cpix_LogLevel cpix_setLogLevel(cpix_LogLevel logLevel)
{
    cpix_LogLevel
        rv = GlobalLogLevel;

    GlobalLogLevel = logLevel;

    return rv;
}


void cpix_dbgForceLogs()
{
    using namespace Cpix;

    cpix_Result
        result;

    XlateExc(&result,
             CallFreeFunc(&RotLogger::SetLoggingEnabled,
                          true));
}


void cpix_dbgDumpState()
{
    using namespace Cpix;

    cpix_Result
        result;

    XlateExc(&result,
             CallFreeFunc(&IIdxDb::dbgDumpAllState));
}


#ifdef DBG_MEM_USAGE
uint32_t cpix_dbgPeakDynMemUsage()
{
    return lucene::debug::PeakDynMemUsage;
}


void cpix_dbgResetDynMemUsage()
{
    lucene::debug::PeakDynMemUsage = 0;
}
#endif



