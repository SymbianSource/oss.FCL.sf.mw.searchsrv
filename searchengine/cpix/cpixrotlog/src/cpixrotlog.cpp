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
* Description: This executable tests the logfile rotationa and std out/err
*               redirecting capability of CPix - these features are to be
*               used in a context of a server/daemon, but this console
*               application tests for it
*
*/


#include <stdio.h>
#include <wchar.h>

#include <fstream>
#include <iostream>
#include <string>

#include <indevicecfg.h>

#include "cpixinit.h"
#include "cpixidxdb.h"

// This is a GCCE toolchain workaround needed when compiling with GCCE
// and using main() entry point
#ifdef __GCCE__
#include <staticlibinit_gcce.h>
#endif


#define LOGFILEBASE "c:\\Data\\cpixrotlog\\log"
#define LOGSIZELIMIT 500
#define LOGSIZECHECKRECURRENCY 10

#define SMS_TO_INDEX "c:\\Data\\cpixrotlog\\SMS_corpus.txt"

#define MAX_SMS_COUNT 100

cpix_FieldDesc SmsSchema[] = {

    // NOTE: in an actual SMS schema, you would probably not want to
    // store fields "to", "from" and "folder".

    {
        LTO_FIELD,                                     // name_
        cpix_STORE_YES | cpix_INDEX_UNTOKENIZED,      // cfg_
    },
    {
        LFROM_FIELD,                                   // name_
        cpix_STORE_YES | cpix_INDEX_UNTOKENIZED,      // cfg_
    },
    {
        LFOLDER_FIELD,                                 // name_
        cpix_STORE_YES | cpix_INDEX_UNTOKENIZED,      // cfg_
    },
    {
        LBODY_FIELD,                                   // name_
        cpix_STORE_NO | cpix_INDEX_TOKENIZED,         // cfg_
    },
    
};



SchemaId addSmsSchema(cpix_IdxDb * idxDb)
{
    return cpix_IdxDb_addSchema(idxDb,
                                SmsSchema,
                                sizeof(SmsSchema)/sizeof(cpix_FieldDesc));
}


void indexSmsMsg(cpix_IdxDb        * idxDb,
                 SchemaId            schemaId,
                 cpix_Analyzer     * analyzer,
                 const std::string & msgBody,
                 size_t              id)
{
    char
        docUid[16];
    snprintf(docUid,
             sizeof(docUid),
             "%d",
             id);
    wchar_t
        wDocUid[16];
    mbstowcs(wDocUid,
             docUid,
             sizeof(wDocUid)/sizeof(wchar_t) - 1);
    wDocUid[sizeof(wDocUid)/sizeof(wchar_t) - 1] = wchar_t(0);

    wchar_t
        excerpt[128];
    cpix_EPIState
        epiState;
    size_t
        maxWords = 6,
        bufSize = sizeof(excerpt) / sizeof(wchar_t);
    cpix_init_EPIState(&epiState);
    cpix_getExcerptOfText(excerpt,
                          msgBody.c_str(),
                          &maxWords,
                          &bufSize,
                          &epiState);

    wchar_t
        wMsgBody[256];
    mbstowcs(wMsgBody,
             msgBody.c_str(),
             sizeof(wMsgBody)/sizeof(wchar_t) - 1);
    wMsgBody[sizeof(wMsgBody)/sizeof(wchar_t) - 1] = wchar_t(0);
    
    wchar_t
        * fields[4];
    fields[0] = L"+3585553412"; // to
    fields[1] = L"+3585559078"; // from
    fields[2] = L"inbox";       // folder
    fields[3] = wMsgBody;       // body

    cpix_IdxDb_add2(idxDb,
                    schemaId,
                    wDocUid,                 // docUid,
                    SMSAPPCLASS,             // app class
                    excerpt,
                    NULL,                    // app id
                    (const wchar_t**)fields, // fields
                    analyzer);

    if (cpix_Failed(idxDb))
        {
            wchar_t
                report[1024];
            cpix_Error_report(idxDb->err_,
                              report,
                              sizeof(report)/sizeof(wchar_t));
            fwprintf(stderr,
                     L"Failed to index SMS %d (%S): %S\n",
                     id,
                     excerpt,
                     report);
            cpix_ClearError(idxDb);
        }
    else
        {
            fprintf(stdout,
                    "SMS %d (%S) indexed\n",
                    id,
                    excerpt);
            fprintf(stderr,
                    "FUNKY OUTPUT TO STD ERR\n");
        }
}


void indexSms(cpix_IdxDb     * idxDb,
              SchemaId         schemaId,
              cpix_Analyzer  * analyzer,
              const char     * path)
{
    using namespace std;

    int
        count = 0;

    ifstream
        ifs(path);

    if (ifs.is_open())
        {
            string
                line;

            size_t
                i = 0;

            while (getline(ifs, line) && count < MAX_SMS_COUNT)
                {
                    indexSmsMsg(idxDb,
                                schemaId,
                                analyzer,
                                line,
                                i);
                    ++i;
                    ++count;
                }
        }
}


void indexSmsCorpus()
{
    wchar_t
        report[512];

    cpix_Result
        result;

    cpix_Analyzer
        * analyzer = cpix_CreateSimpleAnalyzer(&result);

    if (cpix_Succeeded(&result))
        {
            cpix_Result
                result;

            cpix_IdxDb_defineVolume(&result,
                                    SMS_QBASEAPPCLASS,
                                    NULL);

            if (cpix_Succeeded(&result))
                {
                    cpix_IdxDb
                        * idxDb = cpix_IdxDb_openDb(&result,
                                                    SMS_QBASEAPPCLASS,
                                                    cpix_IDX_CREATE);
                    if (cpix_Succeeded(&result))
                        {
                            SchemaId
                                schemaId = addSmsSchema(idxDb);
                    
                            if (cpix_Succeeded(idxDb))
                                {
                                    indexSms(idxDb,
                                             schemaId,
                                             analyzer,
                                             SMS_TO_INDEX);
                                }
                            else
                                {
                                    cpix_Error_report(result.err_,
                                                      report,
                                                      sizeof(report)/sizeof(wchar_t));
                                    fwprintf(stderr,
                                             L"Failed to create-db IdxDb: %S\n",
                                             report);
                                    cpix_ClearError(&result);
                                }

                            cpix_IdxDb_releaseDb(idxDb);
                        }
                    else
                        {
                            cpix_Error_report(result.err_,
                                              report,
                                              sizeof(report)/sizeof(wchar_t));
                            fwprintf(stderr,
                                     L"Failed to create-db IdxDb: %S\n",
                                     report);
                            cpix_ClearError(&result);
                        }                    
                }
            else
                {
                    printf("Could not define volume %s\n",
                           SMS_QBASEAPPCLASS);
                }
            cpix_Analyzer_destroy(analyzer);
        }
    else
        {
            cpix_Error_report(result.err_,
                              report,
                              sizeof(report)/sizeof(wchar_t));
            fwprintf(stderr,
                     L"Failed to create analyzer: %S\n",
                     report);
            cpix_ClearError(&result);
        }
    
}


int main(void)
{
    int
        rv = 0;

    printf("Indexing %d SMS messages from SMS corpus.\n",
           MAX_SMS_COUNT);
    printf("After that, exiting.\n");
    printf("THERE WILL BE NO MORE CONSOLE OUTPUT AFTER THIS LINE - WAIT!\n");


    cpix_Result
        result;

    cpix_InitParams
        * initParams = cpix_InitParams_create(&result);

    if (cpix_Failed(&result))
        {
            printf("Failed to create cpix_InitParams\n");
            return -1;
        }


    cpix_InitParams_setLogFileBase(initParams,
                                   LOGFILEBASE);

    if (cpix_Failed(initParams))
        {
            printf("Failed to set log file base to %s\n",
                   LOGFILEBASE);
            return -1;
        }

    cpix_InitParams_setLogSizeLimit(initParams,
                                    LOGSIZELIMIT);

    if (cpix_Failed(initParams))
        {
            wchar_t
                buffer[128];
            cpix_Error_report(initParams->err_,
                              buffer,
                              sizeof(buffer));
            printf("Failed to set log size limit to %d: %S\n",
                   LOGSIZELIMIT,
                   buffer);
            return -1;
        }

    cpix_InitParams_setLogSizeCheckRecurrency(initParams,
                                              LOGSIZECHECKRECURRENCY);

    if (cpix_Failed(initParams))
        {
            printf("Failed to set log size check recurrency %d\n",
                   LOGSIZECHECKRECURRENCY);
            return -1;
        }

    cpix_init(&result,
              initParams);

    if (cpix_Failed(&result))
        {
            wchar_t
                buffer[128];
            cpix_Error_report(result.err_,
                              buffer,
                              sizeof(buffer));
            printf("Failed to initialize CPix: %S\n",
                   buffer);
            return -1;
        }

    cpix_InitParams_destroy(initParams);

    cpix_setLogLevel(CPIX_LL_DEBUG);

    indexSmsCorpus();

    cpix_shutdown();

    return rv;
}
