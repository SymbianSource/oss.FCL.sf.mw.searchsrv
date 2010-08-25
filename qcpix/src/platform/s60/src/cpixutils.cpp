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
#include <CSearchDocument.h>
#include <CDocumentField.h>
#include "cpixutils.h"

QString QStringFromDescriptor( const TDesC& aDesc )
    {
    return QString::fromUtf16( aDesc.Ptr(), aDesc.Length() );
    }

CpixDocument* CpixDocFromCSearchDocument( CSearchDocument* aDoc )
    {
    if( aDoc == NULL ) return NULL;

    CpixDocument* cpixDoc = CpixDocument::newInstance();
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
	
CpixDocument** CpixBatchDocFromCSearchDocument( TInt aReturnDoc, CSearchDocument** aDoc )
    {
    if( aDoc == NULL || !aReturnDoc ) return NULL;
    CpixDocument** cpixBatchDocs = NULL;
    cpixBatchDocs = (CpixDocument**)malloc ( sizeof(CpixDocument*) * (aReturnDoc));
    for (int count = 0; count < aReturnDoc; count++)
        {        
        CpixDocument* cpixDoc = CpixDocument::newInstance();
        if ( cpixDoc )
            {
            cpixDoc->setBaseAppClass( QStringFromDescriptor( aDoc[count]->AppClass() ) );
            cpixDoc->setDocId( QStringFromDescriptor( aDoc[count]->Id() ) );
            cpixDoc->setExcerpt( QStringFromDescriptor( aDoc[count]->Excerpt() ) );
        
            int fieldCount = aDoc[count]->FieldCount();
            for( int i=0; i< fieldCount; i++ )
                {
                const CDocumentField& field = aDoc[count]->Field( i );
                cpixDoc->addField( QStringFromDescriptor( field.Name() ), QStringFromDescriptor( field.Value() ), field.Config() );
                }
            }
        cpixBatchDocs[count]= cpixDoc;
        delete aDoc[count];
        aDoc[count] = NULL;
        }
    delete aDoc;
    aDoc = NULL;
    return cpixBatchDocs;
    }
