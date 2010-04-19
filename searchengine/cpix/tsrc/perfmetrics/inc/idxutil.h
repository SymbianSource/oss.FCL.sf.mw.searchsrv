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
#ifndef CPIX_PERFMETRICS_IDXUTIL_H_
#define CPIX_PERFMETRICS_IDXUTIL_H_

#include "itk.h"

#include "cpixidxdb.h"


std::wstring id2Wstr(int id);
int wstr2Id(const wchar_t * str);



/**
 * Can help in indexing and printing FILE stuff.
 */
class IdxUtil
{
protected:
    cpix_IdxDb       * idxDb_;
    cpix_IdxSearcher * idxSearcher_;
    cpix_QueryParser * queryParser_;
    cpix_Analyzer    * analyzer_;
    SchemaId           schemaId_;
    int                idx_;


public:
    /**
     * Constructor.
     */
    IdxUtil();
    /**
     * Constructor proper. On failure, uses ITK_PANIC, i.e. it throws
     * Itk::PanicExc, so it can be called from an IContext::setup()
     * implementation as well as from a test function.
     */
    virtual void init(bool create = true) throw (Itk::PanicExc);

    virtual ~IdxUtil() throw ();

    cpix_IdxDb * idxDb();
    cpix_IdxSearcher * idxSearcher();
    cpix_QueryParser * queryParser();
    cpix_Analyzer * analyzer();
    SchemaId schemaId();

    virtual void indexItem(const char   * line,
                           Itk::TestMgr * testMgr);
    virtual void updateItem(const char   * line,
                            Itk::TestMgr * testMgr);
    
    virtual void flush(); 

    int  curIdx() const;
    void setCurIdx(int idx);
    
    virtual const char * qualBaseAppClass() const;
    virtual const char * idxDbPath() const;
    virtual SchemaId addSchema() throw (Itk::PanicExc);

};


class PoiIdxUtil : public IdxUtil
{
public:

    void indexItem(const char   * poiLine,
                   Itk::TestMgr * testMgr);
    void updateItem(const char   * poiLine,
                    Itk::TestMgr * testMgr);

    virtual const char * qualBaseAppClass() const;
    virtual const char * idxDbPath() const;
    virtual SchemaId addSchema() throw (Itk::PanicExc);


protected:
    /**
     * Splits the line along delimiter character '|', and for each field,
     * discards the prefix (if any) up until the first '=' character.
     */
    void getFields(const char              * fieldsLine,
                   std::list<std::wstring> & wFields);
};


class StreetIdxUtil : public PoiIdxUtil
{
public:

    virtual const char * qualBaseAppClass() const;
    virtual const char * idxDbPath() const;
    virtual SchemaId addSchema() throw (Itk::PanicExc);
};

    
#endif
