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

#ifndef CPIXUNITTEST_TESTUTILS_H_
#define CPIXUNITTEST_TESTUTILS_H_

#include <wchar.h>

#include <string>

#include "config.h"

#include "cpixidxdb.h"
#include "itk.h"

/**

 * To be able to search for docuids, we need to process them (from
 * number to ascii) so that they are not removed by the current
 * analyzers.
 */
std::wstring GetItemId(int itemIndex);
int GetItemIndex(const wchar_t * itemId);


void PrintHit(cpix_Document * doc,
              Itk::TestMgr * testMgr);
void PrintHits(cpix_Hits    * hits,
               Itk::TestMgr * testMgr);
void CustomPrintHits(cpix_Hits    * hits,
                     Itk::TestMgr * testMgr,
                     void        (* printHitFunc)(cpix_Document *, Itk::TestMgr *) = &PrintHit);


/**
 * Can help in indexing and printing SMS stuff.
 */
class IdxUtil
{
private:
    cpix_IdxDb     * idxDb_;
    SchemaId         schemaId_;

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
    virtual void reload() throw (Itk::PanicExc);
    virtual void recreate() throw (Itk::PanicExc);
    virtual void flush() throw (Itk::PanicExc);
    virtual void close() throw ();

    virtual ~IdxUtil() throw ();

    void printHits(cpix_Hits    * hits,
                   Itk::TestMgr * testMgr,
                   bool           allowFailure = false);

    cpix_IdxDb * idxDb();
    SchemaId schemaId();

    virtual void printHit(cpix_Document * doc,
                          Itk::TestMgr   * testMgr);

    
protected:
    virtual const char * qualBaseAppClass() const = 0;
    virtual const char * idxDbPath() const = 0;
    virtual SchemaId addSchema() throw (Itk::PanicExc) = 0;

    virtual std::wstring getIdStr(cpix_Document * doc,
                                  Itk::TestMgr  * testMgr) = 0;

private:
    //
    // private methods
    //
               
};



class SmsIdxUtil : public IdxUtil
{
    std::string    qbac_;

public:

    SmsIdxUtil(const char * qbac = SMS_QBASEAPPCLASS);

    virtual ~SmsIdxUtil() throw ();

    void deleteSms(size_t         id,
                   Itk::TestMgr * testMgr);
    void indexSms(size_t          id,
                  const wchar_t * body,
                  cpix_Analyzer * analyzer,
                  Itk::TestMgr   * testMgr,
                  bool            update = false);
    
protected:
    virtual const char * qualBaseAppClass() const;
    virtual const char * idxDbPath() const;
    virtual SchemaId addSchema() throw (Itk::PanicExc);

    virtual std::wstring getIdStr(cpix_Document * doc,
                                  Itk::TestMgr  * testMgr);
};


class FileIdxUtil : public IdxUtil
{
public:
    virtual ~FileIdxUtil() throw ();

    void indexFile(const char    * path,
                   cpix_Analyzer * analyzer,
                   Itk::TestMgr   * testMgr);

protected:
    virtual const char * qualBaseAppClass() const;
    virtual const char * idxDbPath() const;
    virtual SchemaId addSchema() throw (Itk::PanicExc);
    
    virtual std::wstring getIdStr(cpix_Document * doc,
                                  Itk::TestMgr  * testMgr);
};




struct MVFTest
{
    // test file to index
    const char * textFilePath_;

    const char * qualifiedBaseAppClass_;
    const char * idxDbPath_;
};


class VolumeFileIdxUtil : public FileIdxUtil
{
private:
    const MVFTest         * mvfTest_;


public:
    VolumeFileIdxUtil(const MVFTest * mvfTest);

    virtual ~VolumeFileIdxUtil() throw ();
    virtual const char * qualBaseAppClass() const;
    virtual const char * idxDbPath() const;
};


struct Volume
{
    const char * qbac_;
    const char * path_;
};


#ifdef __WINS__ 
#define DRIVE "c:"
#else
#define DRIVE "e:"
#endif

extern const Volume Volumes[];


#endif // CPIXUNITTEST_TESTUTILS_H_
