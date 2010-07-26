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

#include <cpixdocumentfield.h>
#include "cpixdocumentfieldprivate.h"

/**
 * Note: Code in this file should never throw OR leak symbian exceptions.
 * Convert all leaves to C++ exceptions.
 */

CpixDocumentField::CpixDocumentField( const QString aName, const QString aValue, const int aConfig )
    :iPvtImpl( new CpixDocumentFieldPrivate(aName,aValue,aConfig) )
    {
    }

CpixDocumentField* CpixDocumentField::newInstance( const QString aName, const QString aValue, const int aConfig )
    {
    CpixDocumentField* field = NULL;
    try{
        field = new CpixDocumentField( aName, aValue, aConfig );
    }
    catch(...){
        delete field;
        return NULL;
    }
    return field;
    }

CpixDocumentField::~CpixDocumentField()
    {
    delete iPvtImpl;
    }

QString CpixDocumentField::name() const
    {
    return iPvtImpl->iName;
    }

QString CpixDocumentField::value() const
    {
    return iPvtImpl->iValue;
    }

int CpixDocumentField::config() const
    {
    return iPvtImpl->iConfig;
    }
    
void CpixDocumentField::setName(const QString aName)
    {
    iPvtImpl->iName = aName;
    }

void CpixDocumentField::setValue(const QString aValue)
    {
    iPvtImpl->iValue = aValue;
    }

void CpixDocumentField::setConfig(const int aConfig)
    {
    iPvtImpl->iConfig = aConfig;
    }

//End of File
