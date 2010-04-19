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
#include <memory>

#include "cpixstrtools.h"

// Own header
#include "cpixdoc.h"

// cLucene API
#include "CLucene.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"

// CPix internal
#include "cpixsearch.h"
#include "cpixidxdb.h"
#include "idxdb.h"
#include "idxdbmgr.h"
#include "initparams.h"
#include "cpixutil.h"
#include "cpixexc.h"

#include "indevicecfg.h"

#include "document.h"

/***********************************************************
 * Actual implementation of CPix doc API functions.
 */

void cpix_Field_initialize(cpix_Field    * field,
                           const wchar_t * name,
                           const wchar_t * value,
                           int             configs)
{
    Initialize(field,
               CallCtor(field,
                        name,
                        value,
                        configs));
}



const wchar_t * cpix_Field_name(cpix_Field * thisField)
{
    using namespace Cpix;

    return XlateExc(thisField,
                    Caller(thisField,
                               &Field::name));
}



int cpix_Field_isBinary(cpix_Field * thisField)
{
	using namespace Cpix;
    
    bool
        rv = XlateExc(thisField,
                      Caller(thisField,
                                 &Field::isBinary));

    return static_cast<int>(rv);
}



float_t cpix_Field_boost(cpix_Field * thisField)
{
	using namespace Cpix;

    float_t
        rv = XlateExc(thisField,
                      Caller(thisField,
                                 &Field::boost));

    return rv;
}

void cpix_Field_setBoost(cpix_Field    * thisField,
                         float_t         boost)
{
	using namespace Cpix;

    XlateExc(thisField,
             Caller(thisField,
                        &Field::setBoost,
                        boost));
}



const wchar_t * cpix_Field_stringValue(cpix_Field * thisField)
{
	using namespace Cpix;

    return XlateExc(thisField,
                    Caller(thisField,
                               &Field::stringValue));
}

void cpix_Field_release(cpix_Field * thisField)
{
    ReleaseNative(thisField);
}

int cpix_DocFieldEnum_hasMore(cpix_DocFieldEnum * thisDocFieldEnum)
{
	using namespace Cpix;

    bool
        rv = XlateExc(thisDocFieldEnum,
                      Caller(thisDocFieldEnum,
                                 &DocumentFieldIterator::operator bool));

    return static_cast<int>(rv);
}



void cpix_DocFieldEnum_next(cpix_DocFieldEnum * thisDocFieldEnum,
                            cpix_Field        * target)
{
	using namespace Cpix;

    Field*
        pField = XlateExc(thisDocFieldEnum,
                          Caller(thisDocFieldEnum,
                            		 &DocumentFieldIterator::operator++,
                            		 0));

    if (cpix_Succeeded(thisDocFieldEnum))
        {
            target->ptr_ = pField;
            target->err_ = NULL;
        }
}



void cpix_DocFieldEnum_destroy(cpix_DocFieldEnum * thisDocFieldEnum)
{
    DestroyWrapper(thisDocFieldEnum);
}


cpix_Document * cpix_Document_create(cpix_Result   * result,
                                     const wchar_t * docUid,
                                     const char    * appClass,
                                     const wchar_t * excerpt,
                                     const wchar_t * mimeType)
{
	cpix_Document
		* rv = NULL;
	
	rv = Create(result,
			    CallCtor(rv,
			    		 docUid,
						 appClass,
						 excerpt,
						 mimeType));
	
	return rv; 
}


void cpix_Document_add(cpix_Document * thisDocument,
                       cpix_Field    * field)
{
    using namespace Cpix;

    XlateExc(thisDocument,
             Caller(thisDocument,
                        &Document::add,
                        Cast2Native(field)));
}


const wchar_t * 
cpix_Document_getFieldValue(cpix_Document * thisDocument,
                            const wchar_t * fieldName)
    
{
	using namespace Cpix;

    return XlateExc(thisDocument,
                    Caller(thisDocument,
                               &Document::get,
                               fieldName));
}


cpix_DocFieldEnum * cpix_Document_fields(cpix_Document * thisDocument)
{
    cpix_DocFieldEnum
        * rv = NULL;

    using namespace Cpix;
    
    DocumentFieldIterator
        * dfe = XlateExc(thisDocument,
                         Caller(thisDocument,
                                    &Document::fields));

    if (cpix_Succeeded(thisDocument))
        {
            CreateWrapper(dfe,
                          thisDocument,
                          rv);
        }

    return rv;
}


float_t cpix_Document_boost(cpix_Document * thisDocument)
{
	using namespace Cpix;

    float_t
        rv = XlateExc(thisDocument,
                      Caller(thisDocument,
                                 &Document::boost));

    return rv;
}


void cpix_Document_setBoost(cpix_Document * thisDocument,
                            float_t         boost)
{
	using namespace Cpix;

    XlateExc(thisDocument,
             Caller(thisDocument,
                        &Document::setBoost,
                        boost));
}


void cpix_Document_clear(cpix_Document * thisDocument)
{
    using namespace Cpix;

    XlateExc(thisDocument,
             Caller(thisDocument,
                        &Document::clear));
}


void cpix_Document_destroy(cpix_Document * thisDocument)
{
    DestroyWrapper(thisDocument);
}

