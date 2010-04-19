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

// Own header
#include "cpixidxdb.h"

// cLucene API
#include "CLucene.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"


// CPix internal
#include "idxdb.h"
#include "cpixsearch.h"
#include "cpixhits.h"
#include "initparams.h"
#include "cpixutil.h"
#include "iqrytype.h"

#include "document.h"
#include "analyzer.h"

namespace
{

    template<typename SRC_CHAR>
    wchar_t * getExcerptOf(wchar_t        * dst,
                           const SRC_CHAR * src,
                           size_t         * maxWords,
                           size_t         * bufSize,
                           int            * firstInvocation)
    {
        size_t
            words = 0;
        
        wchar_t
            * p = dst;

        wctype_t
            space = wctype("space");
        
        --(*bufSize);

        bool
            goOn = true;

        while (*src != 0 && 0 < *bufSize && words <= *maxWords && goOn)
            {
                while (*src != 0 && 0 < *bufSize && iswctype(*src, space))
                    {
                        ++src;
                    }
                
                bool
                    firstChar = true;
                
                while (*src != 0 && 0 < *bufSize && !iswctype(*src, space))
                    {
                        // on the first char of a coming word, we need
                        // to do some administration
                        if (firstChar)
                            {
                                // first we check if we can read more words,
                                // and if so, we increase the number of words
                                // read in this go
                                firstChar = false;
                                if (words >= *maxWords)
                                    {
                                        goOn = false;
                                        break;
                                    }
                                ++words;

                                // second, we put a delimiting space between
                                // words emitted on the dst buffer - if
                                // necessary
                                if (!*firstInvocation)
                                    {
                                        if (1 < *bufSize)
                                            {
                                                *p++ = ' ';
                                                --(*bufSize);
                                            }
                                        else
                                            {
                                                goOn = false;
                                                break;
                                            }
                                    }
                                else
                                    *firstInvocation = false;
                                
                            }
                        
                        *p++ = static_cast<wchar_t>(*src);
                        --(*bufSize);
                        ++src;
                    }
            }

        *p = 0;
        *maxWords -= words;
        ++(*bufSize);
        
        return p;
    }

}


void cpix_init_EPIState(cpix_EPIState * state)
{
    // the internal state currently stores only if an invocation is
    // the first one for a specific text stream, or subsequent one
    // (see argument 'firstInvocation' for template function
    // getExcerptOf()).
    *state = 1;
}



wchar_t * cpix_getExcerptOfWText(wchar_t       * dst,
                                 const wchar_t * src,
                                 size_t        * maxWords,
                                 size_t        * bufSize,
                                 cpix_EPIState * state)
{
    return getExcerptOf(dst,
                        src,
                        maxWords,
                        bufSize,
                        state);
}



wchar_t * cpix_getExcerptOfText(wchar_t       * dst,
                                const char    * src,
                                size_t        * maxWords,
                                size_t        * bufSize,
                                cpix_EPIState * state)
{
    return getExcerptOf(dst,
                        src,
                        maxWords,
                        bufSize,
                        state);
}



cpix_Term * cpix_Term_create(cpix_Result     * result,
                             const wchar_t   * fieldName,
                             const wchar_t   * fieldValue)
{
    cpix_Term
        * rv = NULL;

    rv = Create(result,
                       CallCtor(rv,
                                fieldName,
                                fieldValue));
    return rv;
}



void cpix_Term_destroy(cpix_Term * thisTerm)
{
    DestroyWrapper(thisTerm);
}




//
// cpix_IdxDb is unfortunately a special case. It is not special
// because its native counterpart, IdxDb is not lucene-implemented,
// but CPix internal (Cpix::IdxDb). It is special because this wrapper
// does not hold actual pointers, but a handle only.
//


// specialization of the generic CreateWrapper for the special
// case of creating wrapper for a handler
template<>
void
CreateWrapper<IdxDbHndl,cpix_Result,cpix_IdxDb>(IdxDbHndl      hndl,
                                                cpix_Result  * result,
                                                cpix_IdxDb  *& wrapper)
{
    wrapper = NULL;
    wrapper = CreateWrapper_Pure<cpix_IdxDb,cpix_Result>(result);
    if (wrapper != NULL)
        {
            wrapper->handle_ = hndl;
            wrapper->err_ = NULL;
        }
    else
        {
            Cpix::IdxDb::release(hndl);
        }
}


// special constructor functor that does not return with a pointer to
// an instance but with a handle
class IdxDbCtorFunctor
{
    
    const char *   qualBaseAppClass_;
    bool           creating_;
public:
    typedef Cpix::IdxDb NativeClass;
    typedef cpix_IdxDb WrpClass;

    IdxDbCtorFunctor(const char * qualBaseAppClass,
                     bool         creating)
        : qualBaseAppClass_(qualBaseAppClass),
          creating_(creating)
    {
        ;
    }


    IdxDbHndl operator()()
    {
        using namespace Cpix;

        if (creating_)
            {
                // (re-)creating
                return IIdxDb::getIdxDbHndlCreating(qualBaseAppClass_);
            }
        else
            {
                // opening
                return IIdxDb::getIdxDbHndl(qualBaseAppClass_,
                                            false); // no multi search
            }
    }
};


cpix_IdxDb * cpix_IdxDb_openDb(cpix_Result       * result,
                               const char        * qualBaseAppClass,
                               cpix_IDX_OpenMode   openMode)
{
    cpix_IdxDb
        * rv = NULL;
    rv = Create(result,
                IdxDbCtorFunctor(qualBaseAppClass,
                                 static_cast<bool>(openMode)));
    return rv;
}



void cpix_IdxDb_releaseDb(cpix_IdxDb * thisIdxDb)
{
    if (thisIdxDb != NULL)
        {
            cpix_Result
                result;
            XlateExc(&result,
                     CallFreeFunc(&Cpix::IIdxDb::release,
                                  thisIdxDb->handle_));
            free(thisIdxDb);
        }
}


cpix_Hits * 
cpix_IdxDb_search(cpix_IdxDb * thisIdxDb,
                  cpix_Query * query)
{
    cpix_Hits
        * rv = NULL;

    using namespace Cpix;

    cpix_Hits * (Cpix::IQryType:: * func)(cpix_IdxDb *) 
        = &Cpix::IQryType::search;

    // This is curious and unusual setup: the error reporting goes on
    // thisIdxDb instance, but the instance used as "this" instance is
    // the query instance. This is due to the extra level of
    // indirection added by unified search interface and hiding
    // various types of queries behind the query instance
    rv = XlateExc(thisIdxDb,                              // report
                  Caller(query,                       // "this"
                             func,
                             thisIdxDb));                 // argument

    return rv;
}


SchemaId cpix_IdxDb_addSchema(cpix_IdxDb           * thisIdxDb,
                              const cpix_FieldDesc * fieldDescs,
                              size_t                 count)
{
    using namespace Cpix;

    return XlateExc(thisIdxDb,
                    Caller(thisIdxDb,
                                  &IIdxDb::addSchema,
                                  fieldDescs,
                                  count));
}


void cpix_IdxDb_add(cpix_IdxDb      * thisIdxDb,
                    cpix_Document   * document,
                    cpix_Analyzer   * analyzer)
{
    using namespace Cpix;
    using namespace lucene::analysis;

    Cpix::SystemAnalyzer
        * a = Cast2Native(analyzer);

    XlateExc(thisIdxDb,
             Caller(thisIdxDb,
                           &IIdxDb::add,
                           Cast2Native(document),
                           static_cast<Analyzer*>(a)));
}


void cpix_IdxDb_add2(cpix_IdxDb      * thisIdxDb,
                     SchemaId          schemaId,
                     const wchar_t   * docUid,
                     const char      * appClass,
                     const wchar_t   * excerpt,
                     const wchar_t   * mimeType,
                     const wchar_t  ** fieldValues,
                     cpix_Analyzer   * analyzer)
{
    using namespace Cpix;
    using namespace lucene::analysis;

    Cpix::SystemAnalyzer
        * a = Cast2Native(analyzer);

    XlateExc(thisIdxDb,
             Caller(thisIdxDb,
                           &IIdxDb::add2,
                           schemaId,
                           docUid,
                           appClass,
                           excerpt,
                           mimeType,
                           fieldValues,
                           static_cast<Analyzer*>(a)));
}

    

int32_t cpix_IdxDb_deleteDocuments(cpix_IdxDb    * thisIdxDb,
                                   const wchar_t * docUid)
{
    using namespace Cpix;

    return XlateExc(thisIdxDb,
                    Caller(thisIdxDb,
                                  &IIdxDb::deleteDocuments,
                                  docUid));
}

    

int32_t cpix_IdxDb_deleteDocuments2(cpix_IdxDb    * thisIdxDb,
                                    cpix_Term     * term)
{
   using namespace Cpix;

   return XlateExc(thisIdxDb,
                   Caller(thisIdxDb,
                                 &IIdxDb::deleteDocuments2,
                                 Cast2Native(term)));
}



void cpix_IdxDb_update(cpix_IdxDb      * thisIdxDb,
                       cpix_Document   * document,
                       cpix_Analyzer   * analyzer)
{
    using namespace Cpix;
    using namespace lucene::analysis;

    Cpix::SystemAnalyzer
        * a = Cast2Native<cpix_Analyzer>(analyzer);

    XlateExc(thisIdxDb,
             Caller(thisIdxDb,
                           &IIdxDb::update,
                           Cast2Native(document),
                           static_cast<Analyzer*>(a)));
}


void cpix_IdxDb_update2(cpix_IdxDb      * thisIdxDb,
                        SchemaId          schemaId,
                        const wchar_t   * docUid,
                        const char      * appClass,
                        const wchar_t   * excerpt,
                        const wchar_t   * mimeType,
                        const wchar_t  ** fieldValues,
                        cpix_Analyzer   * analyzer)
{
    using namespace Cpix;
    using namespace lucene::analysis;

    Cpix::SystemAnalyzer
        * a = Cast2Native<cpix_Analyzer>(analyzer);

    XlateExc(thisIdxDb,
             Caller(thisIdxDb,
                           &IIdxDb::update2,
                           schemaId,
                           docUid,
                           appClass,
                           excerpt,
                           mimeType,
                           fieldValues,
                           static_cast<Analyzer*>(a)));
}



void cpix_IdxDb_setMaxInsertBufSize(cpix_IdxDb * thisIdxDb,
                                    size_t       value)
{
    using namespace Cpix;

    XlateExc(thisIdxDb,
             Caller(thisIdxDb,
                           &IIdxDb::setMaxInsertBufSize,
                           value));
}



void cpix_IdxDb_flush(cpix_IdxDb * thisIdxDb)
{
    using namespace Cpix;

    XlateExc(thisIdxDb,
             Caller(thisIdxDb,
                           &IIdxDb::flush));
}


void cpix_IdxDb_dbgScrapAll(cpix_Result * result)
{
    using namespace Cpix;

    XlateExc(result,
             CallFreeFunc(&IdxDb::scrapAll));
}


void cpix_IdxDb_defineVolume(cpix_Result * result,
                             const char  * qualBaseAppClass,
                             const char  * path)
{
    using namespace Cpix;

    XlateExc(result,
             CallFreeFunc(&IIdxDb::defineVolume,
                          qualBaseAppClass,
                          path));
}


void cpix_IdxDb_undefineVolume(const char * qualBaseAppClass)
{
    using namespace Cpix;

    cpix_Result
        result;

    XlateExc(&result,
             CallFreeFunc(&IIdxDb::undefineVolume,
                          qualBaseAppClass));
}

