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

#include <qcpixdocument.h>
#include <qcpixdocumentfield.h>
#include "qcpixdocumentprivate.h"

/**
 * Note: Code in this file should never throw OR leak symbian exceptions.
 * Convert all leaves to C++ exceptions.
 */

QCPixDocument::QCPixDocument()
    :iPvtImpl( new QCPixDocumentPrivate() )
    {
    }

QCPixDocument* QCPixDocument::newInstance()
    {
    QCPixDocument* doc = NULL;
    try{
    doc = new QCPixDocument();
    }
    catch(...){
        delete doc;
        return NULL;
    }
    return doc;
    }

QCPixDocument::~QCPixDocument()
    {
    delete iPvtImpl;
    }

QString QCPixDocument::docId() const
   {
   return iPvtImpl->iDocId;
   }

QString QCPixDocument::excerpt() const
    {
    return iPvtImpl->iExcerpt;
    }

QString QCPixDocument::baseAppClass() const
    {
    return iPvtImpl->iBaseAppClass;
    }

const QCPixDocumentField& QCPixDocument::field( const int aIndex ) const
    {
    return *(iPvtImpl->iFields.at(aIndex));
    }

int QCPixDocument::fieldCount() const
    {
    return  iPvtImpl->iFields.count();
    }

void QCPixDocument::setDocId(const QString aDocId)
    {
    iPvtImpl->iDocId = aDocId;
    }

void QCPixDocument::setExcerpt(const QString aExcerpt)
    {
    iPvtImpl->iExcerpt = aExcerpt;
    }

void QCPixDocument::setBaseAppClass(const QString aBaseAppClass)
    {
    iPvtImpl->iBaseAppClass = aBaseAppClass;
    }

void QCPixDocument::addField(const QString aName, const QString aValue, const int aConfig)
    {
    iPvtImpl->iFields.append( QCPixDocumentField::newInstance( aName, aValue, aConfig ) );
    }

//End of File
