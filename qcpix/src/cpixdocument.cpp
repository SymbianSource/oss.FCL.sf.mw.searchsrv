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

#include <cpixdocument.h>
#include <cpixdocumentfield.h>
#include "cpixdocumentprivate.h"

/**
 * Note: Code in this file should never throw OR leak symbian exceptions.
 * Convert all leaves to C++ exceptions.
 */

CpixDocument::CpixDocument()
    :iPvtImpl( new CpixDocumentPrivate() )
    {
    }

CpixDocument* CpixDocument::newInstance()
    {
    CpixDocument* doc = NULL;
    try{
    doc = new CpixDocument();
    }
    catch(...){
        delete doc;
        return NULL;
    }
    return doc;
    }

CpixDocument::~CpixDocument()
    {
    delete iPvtImpl;
    }

QString CpixDocument::docId() const
   {
   return iPvtImpl->iDocId;
   }

QString CpixDocument::excerpt() const
    {
    return iPvtImpl->iExcerpt;
    }

QString CpixDocument::baseAppClass() const
    {
    return iPvtImpl->iBaseAppClass;
    }

const CpixDocumentField& CpixDocument::field( const int aIndex ) const
    {
    return *(iPvtImpl->iFields.at(aIndex));
    }

int CpixDocument::fieldCount() const
    {
    return  iPvtImpl->iFields.count();
    }

void CpixDocument::setDocId(const QString aDocId)
    {
    iPvtImpl->iDocId = aDocId;
    }

void CpixDocument::setExcerpt(const QString aExcerpt)
    {
    iPvtImpl->iExcerpt = aExcerpt;
    }

void CpixDocument::setBaseAppClass(const QString aBaseAppClass)
    {
    iPvtImpl->iBaseAppClass = aBaseAppClass;
    }

void CpixDocument::addField(const QString aName, const QString aValue, const int aConfig)
    {
    iPvtImpl->iFields.append( CpixDocumentField::newInstance( aName, aValue, aConfig ) );
    }

const CpixDocumentField& CpixDocument::field( const QString aName ) const
    {
    int i, count = iPvtImpl->iFields.count();
    for (i =0; i< count;i++)
        {
        if(iPvtImpl->iFields.at(i)->name() == aName)
            break;
        }
        return *(iPvtImpl->iFields.at(i));
    }

//End of File
