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

#include "idxutil.h"

#include "cpixtools.h"


std::wstring id2Wstr(int id)
{
    wchar_t
        buffer[32];

    snwprintf(buffer,
              sizeof(buffer)/sizeof(wchar_t),
              L"%d",
              id);
    return std::wstring(buffer);
}


int wstr2Id(const wchar_t * str)
{
    wchar_t
        * end = const_cast<wchar_t*>(str) + wcslen(str);

    return wcstol(str,
                  &end,
                  10);
}



IdxUtil::IdxUtil()
    : idxDb_(NULL),
      idxSearcher_(NULL),
      queryParser_(NULL),
      analyzer_(NULL),
      schemaId_(0),
      idx_(0)
{
    ;
}


void IdxUtil::init(bool create) throw (Itk::PanicExc)
{
    idx_ = 0;

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

    idxSearcher_ = cpix_IdxSearcher_openDb(&result,
                                           qualBaseAppClass());
    
    if (idxDb_ == NULL || idxSearcher_ == NULL)
        {
            wchar_t
                report[256];
            cpix_Error_report(result.err_,
                              report,
                              sizeof(report) / sizeof(wchar_t));
            
            ITK_PANIC("Could not create/open idx db/searcher '%s' for '%s': %S",
                      idxDbPath(),
                      qualBaseAppClass(),
                      report);
        }

    schemaId_ = addSchema();

    if (cpix_Failed(idxDb()))
        {
            cpix_IdxDb_releaseDb(idxDb_);
            ITK_PANIC("Could not add schema");
        }

    analyzer_ = cpix_CreateSimpleAnalyzer(&result);

    if (cpix_Failed(&result))
        {
            cpix_IdxDb_releaseDb(idxDb_);
            ITK_PANIC("Could not create simple analyzer");
        }

    queryParser_ = cpix_QueryParser_create(&result,
                                           LCPIX_DEFAULT_FIELD,
                                           analyzer_);

    if (cpix_Failed(&result))
        {
            cpix_IdxDb_releaseDb(idxDb_);
            cpix_Analyzer_destroy(analyzer_);
            ITK_PANIC("Could not create query parser");
        }
}


IdxUtil::~IdxUtil() throw ()
{
    cpix_IdxDb_releaseDb(idxDb_);
    idxDb_ = NULL;

    cpix_IdxSearcher_releaseDb(idxSearcher_);
    idxSearcher_ = NULL;

    cpix_QueryParser_destroy(queryParser_);
    queryParser_ = NULL;

    cpix_Analyzer_destroy(analyzer_);
    analyzer_ = NULL;
}


cpix_IdxDb * IdxUtil::idxDb()
{
    return idxDb_;
}


cpix_IdxSearcher * IdxUtil::idxSearcher()
{
    return idxSearcher_;
}


cpix_QueryParser * IdxUtil::queryParser()
{
    return queryParser_;
}


cpix_Analyzer * IdxUtil::analyzer()
{
    return analyzer_;
}


SchemaId IdxUtil::schemaId()
{
    return schemaId_;
}


void IdxUtil::indexItem(const char   * line,
                        Itk::TestMgr * testMgr)
{
    Cpt::auto_array<wchar_t>
        wline(new wchar_t[strlen(line) + 1]);
    mbstowcs(wline.get(),
             line,
             strlen(line) + 1);
    
    const wchar_t * fields[] =  { 
        wline.get(),
    };

    cpix_IdxDb_add2(idxDb(),
                    schemaId(),
                    id2Wstr(idx_).c_str(),   // doc uid
                    TEXTAPPCLASS,            // appclass
                    wline.get(),             // excerpt
                    NULL,                    // app id
                    fields,                  // fields
                    analyzer_);

    if (cpix_Failed(idxDb()))
        {
            ITK_ASSERT(testMgr,
                       false,
                       "Failed to index item %d",
                       idx_);
        }

    ++idx_;
}


void IdxUtil::updateItem(const char   * line,
                         Itk::TestMgr * testMgr)
{
    Cpt::auto_array<wchar_t>
        wline(new wchar_t[strlen(line) + 1]);
    mbstowcs(wline.get(),
             line,
             strlen(line) + 1);
    
    const wchar_t * fields[] =  { 
        wline.get(),
    };

    cpix_IdxDb_update2(idxDb(),
                       schemaId(),
                       id2Wstr(idx_).c_str(),   // doc uid
                       TEXTAPPCLASS,            // appclass
                       wline.get(),             // excerpt
                       NULL,                    // app id
                       fields,                  // fields
                       analyzer_);
    
    if (cpix_Failed(idxDb()))
        {
            ITK_ASSERT(testMgr,
                       false,
                       "Failed to update item %d",
                       idx_);
        }

    ++idx_;
}

void IdxUtil::flush() 
{
	cpix_IdxDb_flush( idxDb_ );
	
    if ( cpix_Failed( idxDb_ ) )
    {
		ITK_PANIC("Could not flush idx db '%s' for '%s'",
				  idxDbPath(),
				  qualBaseAppClass());
    }
}

int  IdxUtil::curIdx() const
{
    return idx_;
}


void IdxUtil::setCurIdx(int idx)
{
    idx_ = idx;
}


const char * IdxUtil::qualBaseAppClass() const
{
    return FILE_QBASEAPPCLASS;
}

const char * IdxUtil::idxDbPath() const
{
    // OBS return FILE_CLUCENE_DB;
    return NULL;
}


cpix_FieldDesc FileSchema[] = {
    {
        CONTENTS_FIELD,                             // name_
        cpix_STORE_NO | cpix_INDEX_TOKENIZED,       // cfg_
    }
};


SchemaId IdxUtil::addSchema() throw (Itk::PanicExc)
{
    return cpix_IdxDb_addSchema(idxDb(),
                                FileSchema,
                                sizeof(FileSchema)/sizeof(cpix_FieldDesc));
}




/***************************************************
 * PoiIdxUtil
 */

cpix_FieldDesc PoiSchema[] = {

    {
        L"gpslong",
        cpix_STORE_NO | cpix_INDEX_UNTOKENIZED,
    },

    {
        L"gpslat",
        cpix_STORE_NO | cpix_INDEX_UNTOKENIZED,
    },
    
    {
        L"whatnot",
        cpix_STORE_NO | cpix_INDEX_UNTOKENIZED,
    },

    {
        L"id-und",
        cpix_STORE_NO | cpix_INDEX_UNTOKENIZED,
    },

    {
        L"field01",
        cpix_STORE_NO | cpix_INDEX_TOKENIZED,
    },

    {
        L"field02",
        cpix_STORE_NO | cpix_INDEX_TOKENIZED,
    },

    {
        L"field03",
        cpix_STORE_NO | cpix_INDEX_TOKENIZED,
    },

    {
        L"field04",
        cpix_STORE_NO | cpix_INDEX_TOKENIZED,
    },

    {
        L"field05",
        cpix_STORE_NO | cpix_INDEX_TOKENIZED,
    },

    {
        L"field06",
        cpix_STORE_NO | cpix_INDEX_TOKENIZED,
    },

    {
        L"field07",
        cpix_STORE_NO | cpix_INDEX_TOKENIZED,
    },

    {
        L"field08",
        cpix_STORE_NO | cpix_INDEX_TOKENIZED,
    },

    {
        L"field09",
        cpix_STORE_NO | cpix_INDEX_TOKENIZED,
    },

    {
        L"field10",
        cpix_STORE_NO | cpix_INDEX_TOKENIZED,
    },

    {
        L"field11",
        cpix_STORE_NO | cpix_INDEX_TOKENIZED,
    },

    {
        L"field12",
        cpix_STORE_NO | cpix_INDEX_TOKENIZED,
    },

};


#define POI_FIELD_NUM sizeof(PoiSchema) / sizeof(cpix_FieldDesc)
#define POI_APPCLASS "root maps poi"
#define POI_QBASEAPPCLASS "@0:root maps poi"

void PoiIdxUtil::indexItem(const char   * poiLine,
                           Itk::TestMgr * testMgr)
{
    const wchar_t * fields[POI_FIELD_NUM];
    
    for (const wchar_t ** p = fields;
         p - fields < POI_FIELD_NUM;
         ++p)
        {
            *p = NULL;
        }

    std::list<std::wstring>
        wFields;

    getFields(poiLine,
              wFields);

    std::list<std::wstring>::const_iterator
        i = wFields.begin(),
        end = wFields.end();

    const wchar_t
        ** p = fields;

    std::wstring
        excerpt;
    
    for (; i != end && p - fields < POI_FIELD_NUM; ++i, ++p)
        {
            *p = i->c_str();
            if (p - fields >= 4 && i->length() > 0)
                {
                    excerpt += i->c_str();
                    excerpt += L' ';
                }
        }
    
    cpix_IdxDb_add2(idxDb(),
                    schemaId(),
                    id2Wstr(idx_).c_str(),
                    POI_APPCLASS,
                    excerpt.c_str(),
                    NULL,
                    fields,
                    analyzer_);
                    
    if (cpix_Failed(idxDb()))
        {
            ITK_ASSERT(testMgr,
                       false,
                       "Failed to index item %d",
                       idx_);
        }

    ++idx_;
}


void PoiIdxUtil::updateItem(const char   * poiLine,
                            Itk::TestMgr * testMgr)
{
    const wchar_t * fields[POI_FIELD_NUM];
    
    for (const wchar_t ** p = fields;
         p - fields < POI_FIELD_NUM;
         ++p)
        {
            *p = NULL;
        }

    std::list<std::wstring>
        wFields;

    getFields(poiLine,
              wFields);

    std::list<std::wstring>::const_iterator
        i = wFields.begin(),
        end = wFields.end();

    const wchar_t
        ** p = fields;
    
    std::wstring
        excerpt;

    for (; i != end && p - fields < POI_FIELD_NUM; ++i, ++p)
        {
            *p = i->c_str();
            if (p - fields >= 4 && i->length() > 0)
                {
                    excerpt += i->c_str();
                    excerpt += L' ';
                }
        }
    
    cpix_IdxDb_update2(idxDb(),
                       schemaId(),
                       id2Wstr(idx_).c_str(),
                       POI_APPCLASS,
                       excerpt.c_str(),
                       NULL,
                       fields,
                       analyzer_);
    
    if (cpix_Failed(idxDb()))
        {
            ITK_ASSERT(testMgr,
                       false,
                       "Failed to update item %d",
                       idx_);
        }

    ++idx_;
}


const char * PoiIdxUtil::qualBaseAppClass() const
{
    return POI_QBASEAPPCLASS;
}


const char * PoiIdxUtil::idxDbPath() const
{
#ifdef __WINS__ 
    return CPIX_TEST_INDEVICE_INDEXDB_PHMEM "\\root\\maps\\poi";
#else
    return CPIX_TEST_INDEVICE_INDEXDB_MMC "\\root\\maps\\poi";
#endif
}


SchemaId PoiIdxUtil::addSchema() throw (Itk::PanicExc)
{
    return cpix_IdxDb_addSchema(idxDb(),
                                PoiSchema,
                                POI_FIELD_NUM);
}


#define BLANKS L" \t\r\n"

void PoiIdxUtil::getFields(const char              * fieldsLine,
                           std::list<std::wstring> & wFields)
{
    wFields.clear();

    // scan the whole fieldsLine
    for (const char * p = fieldsLine;
         *p != 0;
         ++p)
        {
            const char
                * fieldStart = p;
            
            // scan for next field
            while (*p != 0 && *p != '|')
                {
                    if (*p == '=')
                        {
                            fieldStart = p + 1;
                        }

                    ++p;
                }
            
            // if a non-null field is found, widen it and push it to
            // the list
            if (p != fieldStart)
                {
                    size_t
                        size = p - fieldStart + 1;
                    Cpt::auto_array<wchar_t>
                        wTmp(new wchar_t[size]);
                    mbstowcs(wTmp.get(),
                             fieldStart,
                             size);
                    wTmp.get()[size - 1] = 0;

                    std::wstring
                        wstr(wTmp.get());
                    
                    size_t
                        pos = wstr.find_first_not_of(BLANKS);
                    if (pos == std::wstring::npos)
                        {
                            continue;
                        }
                    if (pos != 0)
                        {
                            wstr = wstr.substr(pos);
                        }
                    pos = wstr.find_last_not_of(BLANKS);
                    if (pos != 0)
                        {
                            wstr = wstr.substr(0,
                                               pos + 1);
                        }

                    wFields.push_back(std::wstring(wstr.c_str()));
                }

            if (*p != 0)
                {
                    ++p;
                }
        }
}



/**********************************************************
 * Street
 */


#define STREET_QBASEAPPCLASS "@0:root maps street"


const char * StreetIdxUtil::qualBaseAppClass() const
{
    return STREET_QBASEAPPCLASS;
}


const char * StreetIdxUtil::idxDbPath() const
{
#ifdef __WINS__ 
    return CPIX_TEST_INDEVICE_INDEXDB_PHMEM "\\root\\maps\\street";
#else
    return CPIX_TEST_INDEVICE_INDEXDB_MMC "\\root\\maps\\street";
#endif
}

cpix_FieldDesc StreetSchema[] = {

    { L"street",
      cpix_STORE_NO | cpix_INDEX_TOKENIZED
    },

    { L"zip",
      cpix_STORE_NO | cpix_INDEX_UNTOKENIZED
    },

    { L"whatnot1",
      cpix_STORE_NO | cpix_INDEX_UNTOKENIZED
    },

    { L"city",
      cpix_STORE_NO | cpix_INDEX_TOKENIZED
    },

    { L"whatnot2",
      cpix_STORE_NO | cpix_INDEX_UNTOKENIZED
    },

    { L"countrycode",
      cpix_STORE_NO | cpix_INDEX_TOKENIZED
    },

    { L"citycountry",
      cpix_STORE_NO | cpix_INDEX_TOKENIZED
    },

    { L"whatnot3",
      cpix_STORE_NO | cpix_INDEX_UNTOKENIZED
    },

    { L"gps",
      cpix_STORE_NO | cpix_INDEX_UNTOKENIZED
    },

    { L"whatnot4",
      cpix_STORE_NO | cpix_INDEX_UNTOKENIZED
    },

};


#define STREET_FIELD_NUM sizeof(StreetSchema)/sizeof(cpix_FieldDesc)

SchemaId StreetIdxUtil::addSchema() throw (Itk::PanicExc)
{
    return cpix_IdxDb_addSchema(idxDb(),
                                StreetSchema,
                                STREET_FIELD_NUM);

}
