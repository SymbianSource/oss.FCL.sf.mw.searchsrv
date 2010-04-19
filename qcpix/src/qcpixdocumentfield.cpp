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

#include <qcpixdocumentfield.h>
#include "qcpixdocumentfieldprivate.h"

/**
 * Note: Code in this file should never throw OR leak symbian exceptions.
 * Convert all leaves to C++ exceptions.
 */

QCPixDocumentField::QCPixDocumentField( const QString aName, const QString aValue, const int aConfig )
    :iPvtImpl( new QCPixDocumentFieldPrivate(aName,aValue,aConfig) )
    {
    }

QCPixDocumentField* QCPixDocumentField::newInstance( const QString aName, const QString aValue, const int aConfig )
    {
    QCPixDocumentField* field = NULL;
    try{
        field = new QCPixDocumentField( aName, aValue, aConfig );
    }
    catch(...){
        delete field;
        return NULL;
    }
    return field;
    }

QCPixDocumentField::~QCPixDocumentField()
    {
    delete iPvtImpl;
    }

QString QCPixDocumentField::name() const
    {
    return iPvtImpl->iName;
    }

QString QCPixDocumentField::value() const
    {
    return iPvtImpl->iValue;
    }

int QCPixDocumentField::config() const
    {
    return iPvtImpl->iConfig;
    }
    
void QCPixDocumentField::setName(const QString aName)
    {
    iPvtImpl->iName = aName;
    }

void QCPixDocumentField::setValue(const QString aValue)
    {
    iPvtImpl->iValue = aValue;
    }

void QCPixDocumentField::setConfig(const int aConfig)
    {
    iPvtImpl->iConfig = aConfig;
    }

//End of File
