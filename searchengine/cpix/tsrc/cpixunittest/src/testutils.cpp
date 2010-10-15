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

#include <dirent.h>
#include <errno.h>
#include <wchar.h>
#include <stddef.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include "cpixidxdb.h"

#include "itk.h"

#include "config.h"



#include "testutils.h"

#include "std_log_result.h"

std::wstring GetItemId(int itemIndex)
{
    using namespace std;

    wostringstream 
        id; 

    if (itemIndex == 0)
        {
            id << (wchar_t)'a';
        }
    else
        {
            // smallest first 
            while (itemIndex > 0) {
                id << (wchar_t)('a'+(itemIndex%10));
                itemIndex/=10; 
            }
        }

    return id.str(); 
}


int GetItemIndex(const wchar_t * itemId)
{
    int 
        ret = 0, 
        exp = 1;

    size_t
        length = wcslen(itemId);

    for (size_t i = 0; i < length; ++i) {
        if ( itemId[i] >= 'a' && itemId[i] < 'a'+10) {
            ret += (itemId[i]-'a') * exp; 
            exp*=10; 
        } else {
            // TODO: Panic
            return -1; 
        }
    }

    return ret; 
}



void PrintHit(cpix_Document * doc,
              Itk::TestMgr * testMgr)
{
    using namespace std;

    const wchar_t
        * value = cpix_Document_getFieldValue(doc,
                                              LCPIX_DOCUID_FIELD);

    // path-case: if it looks like path starting with a drive letter
    // and a colon-separator, then get rid of the drive letter to be
    // able to pass test cases both on the phone and the emulator
    wstring
        docUid(value);
    if (docUid.length() > 2
        && docUid[1] == L':')
        {
            docUid[0] = L'!';
        }

    ITK_EXPECT(testMgr, value != NULL, "DOCUID field not found");

    fprintf(stdout,
            "DOC (%S): ",
            docUid.c_str());
    
    value = cpix_Document_getFieldValue(doc,
                                        LCPIX_EXCERPT_FIELD);
    
    ITK_EXPECT(testMgr, value != NULL, "EXCERPT field not found");
    
    fprintf(stdout,
            "%S\n",
            value);
}


void PrintHits(cpix_Hits    * hits,
               Itk::TestMgr * testMgr)
{
    CustomPrintHits(hits,
                    testMgr,
                    &PrintHit);
}


void CustomPrintHits(cpix_Hits    * hits,
                     Itk::TestMgr * testMgr,
                     void        (* printHitFunc)(cpix_Document *, Itk::TestMgr *))
{
    using namespace std;

    int32_t
        hitCount = cpix_Hits_length(hits);

    if (cpix_Failed(hits))
        {
            ITK_EXPECT(testMgr,
                       false,
                       "Failed to get number of hits");
            cpix_ClearError(hits);
            return;
        }

    cout << "Number of hits: " << hitCount << endl;

    cpix_Document
        **doc;
    ALLOC_DOC(doc, 1);

    for (int32_t i = 0; i < hitCount; ++i)
        {
            cpix_Hits_doc(hits,
                          i,
                          doc,
                          1);

            if (cpix_Failed(hits) || (doc_Fetch_Failed(doc[0])))
                {
                    ITK_EXPECT(testMgr,
                               false,
                               "Failed to get doc %d",
                               i);
                    cpix_ClearError(hits);
                    break;
                }
            // OBS PrintHit(&doc,
            printHitFunc(doc[0],
                         testMgr);
        }
        
    FREE_DOC(doc, 1);
}




IdxUtil::IdxUtil()
    : idxDb_(NULL),
      schemaId_(0)
{
    ;
}

void IdxUtil::init(bool create) throw (Itk::PanicExc)
{
    cpix_Result
        result;

    if (create)
        {
            cpix_IdxDb_defineVolume(&result,
                                    qualBaseAppClass(),
                                    idxDbPath());

            if (cpix_Succeeded(&result))
                {
                    idxDb_ = cpix_IdxDb_openDb(&result,
                                               qualBaseAppClass(),
                                               cpix_IDX_CREATE);
                }
        }
    else
        {
            idxDb_ = cpix_IdxDb_openDb(&result,
                                       qualBaseAppClass(),
                                       cpix_IDX_OPEN);
        }
    
    if (idxDb_ == NULL)
        {
            wchar_t
                report[256];

            cpix_Error_report(result.err_,
                              report,
                              sizeof(report) / sizeof(wchar_t));

            ITK_PANIC("Could not create/open idx db '%s' for '%s - %S'",
                      idxDbPath(),
                      qualBaseAppClass(),
                      report);
}
             schemaId_ = addSchema();
}

void IdxUtil::reload() throw (Itk::PanicExc)
{
    cpix_Result
        result;

    cpix_IdxDb_releaseDb(idxDb_);
    idxDb_ = NULL; 
    schemaId_ = NULL; 
    
    idxDb_ = cpix_IdxDb_openDb(&result,
                               qualBaseAppClass(),
                               cpix_IDX_OPEN);
	
    if (idxDb_ == NULL)
        {
            ITK_PANIC("Could not reopen idx db '%s' for '%s'",
                      idxDbPath(),
                      qualBaseAppClass());
        }
	
    schemaId_ = addSchema(); 
}

void IdxUtil::recreate() throw (Itk::PanicExc)
{
    cpix_Result
        result;

    cpix_IdxDb_releaseDb(idxDb_);
    idxDb_ = NULL; 
    schemaId_ = NULL; 
	
    cpix_IdxDb_defineVolume(&result,
                            qualBaseAppClass(),
                            idxDbPath());
    if (cpix_Succeeded(&result))
        {
            idxDb_ = cpix_IdxDb_openDb(&result,
                                       qualBaseAppClass(),
                                       cpix_IDX_CREATE);
        }
	
    if (idxDb_ == NULL)
        {
            ITK_PANIC("Could not reopen idx db '%s' for '%s'",
                      idxDbPath(),
                      qualBaseAppClass());
        }

    schemaId_ = addSchema(); 	
}

void IdxUtil::flush() throw (Itk::PanicExc) 
{
	cpix_IdxDb_flush( idxDb_ );
	
    if ( cpix_Failed( idxDb_ ) )
    {

		ITK_PANIC("Could not flush idx db '%s' for '%s'",
				  idxDbPath(),
				  qualBaseAppClass());
    }
}

void IdxUtil::close() throw() 
{
	if ( idxDb_ ) 
	{
		cpix_IdxDb_releaseDb(idxDb_);
		idxDb_ = 0; 
	}
}

IdxUtil::~IdxUtil() throw()
{
	close(); 
}


void IdxUtil::printHits(cpix_Hits    * hits,
                        Itk::TestMgr * testMgr,
                        bool           allowFailure)
{
    using namespace std;

    int32_t
        hitCount = cpix_Hits_length(hits);

    if (cpix_Failed(hits))
        {
            ITK_EXPECT(testMgr,
                       false,
                       "Failed to get number of hits");
            cpix_ClearError(hits);
            return;
        }

    cout << "Number of hits: " << hitCount << endl;

    cpix_Document
        **doc;
    ALLOC_DOC(doc, 1);

    for (int32_t i = 0; i < hitCount; ++i)
        {
            cpix_Hits_doc(hits,
                          i,
                          doc,
                          1);

            if (cpix_Failed(hits) || (doc_Fetch_Failed(doc[0])))
                {
                    if (allowFailure)
                        {
                            printf("Failed to get doc %d - updated index?\n",
                                   i);
                        }
                    else
                        {
                            ITK_EXPECT(testMgr,
                                       false,
                                       "Failed to get doc %d",
                                       i);
                        }

                    cpix_ClearError(hits);
                    break;
                }
            printHit(doc[0],
                        testMgr);
        }
    FREE_DOC(doc, 1);
}


cpix_IdxDb * IdxUtil::idxDb()
{
    return idxDb_;
}


SchemaId IdxUtil::schemaId()
{
    return schemaId_;
}
    

void IdxUtil::printHit(cpix_Document * doc,
                       Itk::TestMgr   * testMgr)
{
    using namespace std;

    std::wstring
        idStr(getIdStr(doc,
                       testMgr));

    fprintf(stdout, 
    		"DOC (%S): ",
    		idStr.c_str());
       
    const wchar_t 
        * value = cpix_Document_getFieldValue(doc,
												LCPIX_EXCERPT_FIELD);
    
    ITK_EXPECT(testMgr, value != NULL, "EXCERPT field not found");
    
    fprintf(stdout, 
    		"%S\n",
    		value);
}



/****
 * SmsIdxUtil
 */
SmsIdxUtil::SmsIdxUtil(const char * qbac)
    : qbac_(qbac)
{
    ;
}


SmsIdxUtil::~SmsIdxUtil() throw ()
{
    ;
}


const char * SmsIdxUtil::qualBaseAppClass() const
{
    return qbac_.c_str();
}


const char * SmsIdxUtil::idxDbPath() const
{
    return NULL;
}


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



SchemaId SmsIdxUtil::addSchema() throw (Itk::PanicExc)
{
    SchemaId
        rv = cpix_IdxDb_addSchema(idxDb(),
                                  SmsSchema,
                                  sizeof(SmsSchema)/sizeof(cpix_FieldDesc));
    if (cpix_Failed(idxDb()))
        {
            cpix_ClearError(idxDb());
            ITK_PANIC("Failed to add SMS schema");
        }

    return rv;
}


void SmsIdxUtil::deleteSms(size_t         id,
                           Itk::TestMgr * testMgr)
{
    std::wstring
        docUid = GetItemId(id);

    int32_t
        result = cpix_IdxDb_deleteDocuments(idxDb(),
                                            docUid.c_str());


    ITK_EXPECT(testMgr,
               cpix_Succeeded(idxDb()),
               "Failed to delete by %S",
               docUid.c_str());

    if (cpix_Succeeded(idxDb()))
        {
            ITK_MSG(testMgr,
                    "Deleted %d items by %S",
                    result,
                    docUid.c_str());
        }

}


void SmsIdxUtil::indexSms(size_t          id,
                          const wchar_t * body,
                          cpix_Analyzer * analyzer,
                          Itk::TestMgr   * testMgr,
                          bool            update)
{
    using namespace std;

    wstring 
        docUid = GetItemId(id); 
    
    /* TEMP
       currently we are going to use the full SMS body as excerpt
       for testing purposes, but otherwise this piece of code could
       do excerpt generation.
    wchar_t
        excerpt[128];
    cpix_EPIState
        epiState;
    size_t
        maxWords = 6,
        bufSize = sizeof(excerpt) / sizeof(wchar_t);
    cpix_init_EPIState(&epiState);
    cpix_getExcerptOfWText(excerpt,
                           body,
                           &maxWords,
                           &bufSize,
                           &epiState);
    */

    const wchar_t
        * fields[4];
    fields[0] = L"+3585553412"; // to
    fields[1] = L"+3585559078"; // from
    fields[2] = L"inbox";       // folder
    fields[3] = body;           // body
    
    void (*op)(cpix_IdxDb*,
               SchemaId,
               const wchar_t*,
               const char*,
               const wchar_t*,
               const wchar_t*,
               const wchar_t**,
               cpix_Analyzer*) = &cpix_IdxDb_add2;
    const char
        * okFormatStr = "Indexed SMS %d",
        * failureStr = "Failed to index SMS";

    if (update)
        {
            op = &cpix_IdxDb_update2;
            okFormatStr = "Updated SMS %d";
            failureStr = "Failed to update SMS";
        }

    op(idxDb(),
       schemaId(),
       docUid.c_str(),    // doc uid
       SMSAPPCLASS,       // app class
       // TEMP excerpt,           // excerpt
       body,              // (as) excerpt
       NULL,              // app id
       fields,            // fields
       analyzer);
    
    if (cpix_Succeeded(idxDb()))
        {
            ITK_MSG(testMgr,
                    okFormatStr,
                    id);
        }
    else
        {
            wchar_t
                report[256];
            cpix_Error_report(idxDb()->err_,
                              report,
                              sizeof(report) / sizeof(wchar_t)); 
           
            ITK_EXPECT(testMgr,
                       false,
                       "%s %d: %S",
                       failureStr,
                       id,
                       report);
            
            cpix_ClearError(idxDb());
        }
}

std::wstring SmsIdxUtil::getIdStr(cpix_Document * doc,
                                  Itk::TestMgr  * testMgr)
{
    const wchar_t
        * value = cpix_Document_getFieldValue(doc,
                                              LCPIX_DOCUID_FIELD);

    ITK_EXPECT(testMgr, value != NULL, "DOCUID field not found");

    std::wstring
        rv(value);

    rv += L", line ";

    wchar_t
        dummy[32];
    snwprintf(dummy,
              sizeof(dummy) / sizeof(wchar_t),
              L"%d",
              GetItemIndex(value) + 1);

    rv += dummy;

    return rv;
}


/****
 * FileIdxUtil
 */
FileIdxUtil::~FileIdxUtil() throw ()
{
    ;
}


void FileIdxUtil::indexFile(const char    * path,
                            cpix_Analyzer * analyzer,
                            Itk::TestMgr  * testMgr)
{
    wchar_t
        wpath[256];

    size_t
        res = mbstowcs(wpath,
                       path,
                       sizeof(wpath) / sizeof(wchar_t) - 1);
    ITK_ASSERT(testMgr,
               //res >= 0,
			   1,
               "mbstowcs failed, errno: %d",
               errno);
    wpath[sizeof(wpath) / sizeof(wchar_t) - 1] = wchar_t(0);

    /* THIS IS ALSO POSSIBLE
    cpix_IdxDb_add2(idxDb(),
                    schemaId(),
                    wpath,                   // docUid,
                    TEXTAPPCLASS,            // app class
                    NULL,                    // excerpt file parser defined
                    NULL,                    // app id
                    (const wchar_t**)&field, // fields: { cpix_FILTERID_FIELD }
                    analyzer);
    */


    // Using here the non-schema based addition method
    // o creating document
    // o populating it
    // o adding it
    // NOTE: app class, excerpt and mime type have to be defined
    // here, but this is using the file parser functionality
    // inside Cpix, which re-defines these anyway
    cpix_Result
        result;
    cpix_Document
        * doc = cpix_Document_create(&result,
                                     wpath,          // docUid,
                                     NULL,           // app class
                                     NULL,           // excerpt (file parser d)
                                     NULL);          // mime type

    wchar_t
        report[512];
    
    if (cpix_Failed(&result))
        {
            cpix_Error_report(result.err_,
                              report,
                              sizeof(report)/sizeof(wchar_t));
            ITK_EXPECT(testMgr,
                       false,
                       "Failed to create document %S: %S",
                       wpath,
                       report);
            cpix_ClearError(&result);
            return;
        }

    cpix_Field
        filterIdField;

    cpix_Field_initialize(&filterIdField,
                          LCPIX_FILTERID_FIELD,
                          LCPIX_FILEPARSER_FID,
                          cpix_STORE_YES | cpix_INDEX_NO);
                   
    if (cpix_Failed(&filterIdField))
        {
            cpix_Error_report(filterIdField.err_,
                              report,
                              sizeof(report)/sizeof(wchar_t));
            ITK_EXPECT(testMgr,
                       false,
                       "Failed to create field");
            cpix_ClearError(&filterIdField);
            cpix_Document_destroy(doc);
            return;
        }

    cpix_Document_add(doc,
                      &filterIdField);

    if (cpix_Failed(doc))
        {
            cpix_Error_report(doc->err_,
                              report,
                              sizeof(report)/sizeof(wchar_t));
            ITK_EXPECT(testMgr,
                       false,
                       "Failed to add field to doc");
            cpix_ClearError(doc);
            cpix_Document_destroy(doc);
            cpix_Field_release(&filterIdField);
            // at this point reader is owned by field already
            return;
        }

    cpix_IdxDb_add(idxDb(),
                   doc,
                   analyzer);

    if (cpix_Failed(idxDb()))
        {
            cpix_Error_report(idxDb()->err_,
                              report,
                              sizeof(report)/sizeof(wchar_t));
            ITK_EXPECT(testMgr,
                       false,
                       "Failed to index document %S: %S\n",
                       wpath,
                       report);
            cpix_ClearError(idxDb());
        }
    else
        {
            wpath[0] = '!';
            ITK_MSG(testMgr,
                    "Indexed file: %S",
                    wpath);
        }

    cpix_Document_destroy(doc);
}


cpix_FieldDesc FileSchema[] = {
    {
    		LCPIX_FILTERID_FIELD,                  // name_
        cpix_STORE_YES | cpix_INDEX_NO,       // cfg_
    }
};


const char * FileIdxUtil::qualBaseAppClass() const
{
    return FILE_QBASEAPPCLASS;
}


const char * FileIdxUtil::idxDbPath() const
{
    return NULL;
}


SchemaId FileIdxUtil::addSchema() throw (Itk::PanicExc)
{
    return cpix_IdxDb_addSchema(idxDb(),
                                FileSchema,
                                sizeof(FileSchema)/sizeof(cpix_FieldDesc));
}


std::wstring FileIdxUtil::getIdStr(cpix_Document * doc,
                                   Itk::TestMgr  * testMgr)
{
    const wchar_t
        * value = cpix_Document_getFieldValue(doc,
                                              LCPIX_DOCUID_FIELD);
    ITK_EXPECT(testMgr, value != NULL, "DOCUID field not found");

    std::wstring
        rv(value);

    rv[0] = '!';

    return rv;
}



VolumeFileIdxUtil::VolumeFileIdxUtil(const MVFTest * mvfTest)
        : mvfTest_(mvfTest)
{
    ;
}


VolumeFileIdxUtil::~VolumeFileIdxUtil() throw ()
{
    ;
}


const char * VolumeFileIdxUtil::qualBaseAppClass() const
{
    return mvfTest_->qualifiedBaseAppClass_;
}


const char * VolumeFileIdxUtil::idxDbPath() const
{
    return mvfTest_->idxDbPath_;
}

