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
#include <CSearchDocument.h>
#include <CDocumentField.h>
#include "qcpixutils.h"

QString QStringFromDescriptor( const TDesC& aDesc )
    {
    return QString::fromUtf16( aDesc.Ptr(), aDesc.Length() );
    }

QCPixDocument* QCPixDocFromCPixDoc( CSearchDocument* aDoc )
    {
    if( aDoc == NULL ) return NULL;

    QCPixDocument* cpixDoc = QCPixDocument::newInstance();
    cpixDoc->setBaseAppClass( QStringFromDescriptor( aDoc->AppClass() ) );
    cpixDoc->setDocId( QStringFromDescriptor( aDoc->Id() ) );
    cpixDoc->setExcerpt( QStringFromDescriptor( aDoc->Excerpt() ) );
    
    int fieldCount = aDoc->FieldCount();
    for( int i=0; i<fieldCount; i++ ){
        const CDocumentField& field = aDoc->Field( i );
        cpixDoc->addField( QStringFromDescriptor( field.Name() ), QStringFromDescriptor( field.Value() ), field.Config() );
    }

    delete aDoc;
    return cpixDoc;
    }
