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

#include "indevicecfg.h"

#include "initparams.h"


namespace Cpix
{

    const char LOGSIZELIMIT::NAME[]           = "logSizeLimit";
    const char LOGSIZECHECKRECURRENCY::NAME[] = "logSizeCheckRecurrency";
    const char MAXIDLESEC::NAME[]             = "maxIdleSec";
    const char MAXINSERTBUFSIZE::NAME[]       = "maxInsertBufSize";
    const char INSERTBUFMAXDOCS::NAME[]       = "insertBufMaxDocs";
    const char IDXJOBQUEUESIZE::NAME[]        = "idxJobQueueSize";
    const char QRYJOBQUEUESIZE::NAME[]        = "qryJobQueueSize";
    const char IDXTHREADPRIORITYDELTA::NAME[] = "idxThreadPriorityDelta";
    const char QRYTHREADPRIORITYDELTA::NAME[] = "qryThreadPriorityDelta";
    const char CLHITSPAGESIZE::NAME[]         = "clHitsPageSize";


    InitParams::InitParams()
        : cpixDir_(DEFAULT_CPIX_DIR),
          resourceDir_(DEFAULT_RESOURCE_DIR)
    {
        setenv("LUCENE_LOCK_DIR_ENV_1",DEFAULT_CLUCENE_LOCK_DIR,1);
    }


    const char * InitParams::getCpixDir() const
    {
        return cpixDir_.c_str();
    }

    void InitParams::setCpixDir(const char * value)
    {
        if (value == NULL
            || strlen(value) == 0)
            {
                THROW_CPIXEXC("Value for property cpixDir cannot be NULL or empty string");
            }

        cpixDir_ = value;
    }

    const char * InitParams::getResourceDir() const
    {
        return resourceDir_.c_str();
    }

    void InitParams::setResourceDir(const char * value)
    {
        if (value == NULL
            || strlen(value) == 0)
            {
                THROW_CPIXEXC("Value for property resourceDir cannot be NULL or empty string");
            }

        resourceDir_ = value;
    }

    const char * InitParams::getCluceneLockDir() const
    {
        return getenv("LUCENE_LOCK_DIR_ENV_1");
    }

    void InitParams::setCluceneLockDir(const char * value)
    {
        if (value == NULL
            || strlen(value) == 0)
            {
                THROW_CPIXEXC("Value for property clucene lock dir cannot be NULL or empty string");
            }

        setenv("LUCENE_LOCK_DIR_ENV_1",value,1);
    }

    const char * InitParams::getLogFileBase() const
    {
        return logFileBase_.length() == 0 ? NULL : logFileBase_.c_str();
    }


    void InitParams::setLogFileBase(const char * logFileBase)
    {
        logFileBase_ = logFileBase == NULL ? "" : logFileBase;
    }

        
    size_t InitParams::getLogSizeLimit() const
    {
        return logSizeLimit_.get();
    }

    void InitParams::setLogSizeLimit(size_t value)
    {
        logSizeLimit_.set(value);
    }


    size_t InitParams::getLogSizeCheckRecurrency() const
    {
        return logSizeCheckRecurrency_.get();
    }

    void InitParams::setLogSizeCheckRecurrency(size_t value)
    {
        logSizeCheckRecurrency_.set(value);
    }

        
    size_t InitParams::getMaxIdleSec() const
    {
        return maxIdleSec_.get();
    }

    void InitParams::setMaxIdleSec(size_t value)
    {
        maxIdleSec_.set(value);
    }


    size_t InitParams::getMaxInsertBufSize() const
    {
        return maxInsertBufSize_.get();
    }

    void InitParams::setMaxInsertBufSize(size_t value)
    {
        maxInsertBufSize_.set(value);
    }


    size_t InitParams::getInsertBufMaxDocs() const
    {
        return insertBufMaxDocs_.get();
    }

    void InitParams::setInsertBufMaxDocs(size_t value)
    {
        insertBufMaxDocs_.set(value);
    }


    size_t InitParams::getIdxJobQueueSize() const
    {
        return idxJobQueueSize_.get();
    }

    void InitParams::setIdxJobQueueSize(size_t value)
    {
        idxJobQueueSize_.set(value);
    }

 
    size_t InitParams::getQryJobQueueSize() const
    {
        return qryJobQueueSize_.get();
    }

    void InitParams::setQryJobQueueSize(size_t value)
    {
        qryJobQueueSize_.set(value);
    }

    
    int InitParams::getIdxThreadPriorityDelta() const
    {
        return idxThreadPriorityDelta_.get();
    }


    void InitParams::setIdxThreadPriorityDelta(int value)
    {
        idxThreadPriorityDelta_.set(value);
    }


    int InitParams::getQryThreadPriorityDelta() const
    {
        return qryThreadPriorityDelta_.get();
    }


    void InitParams::setQryThreadPriorityDelta(int value)
    {
        qryThreadPriorityDelta_.set(value);
    }


    size_t InitParams::getClHitsPageSize() const
    {
        return clHitsPageSize_.get();
    }

    void InitParams::setClHitsPageSize(size_t value)
    {
        clHitsPageSize_.set(value);
    }
    

}
