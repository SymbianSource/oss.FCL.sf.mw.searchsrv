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

#include <qcpixsearcher.h>
#include <qcpixcommon.h>
#include <CCPixSearcher.h>
#include <CSearchDocument.h>

#include "qcpixsearcherprivate.h"
#include "qcpixutils.h"

/**
 * Note: Code in this file should never throw OR leak symbian exceptions.
 * Convert all leaves to C++ exceptions.
 */

QCPixSearcher::QCPixSearcher( QString aDefaultSearchField )
    :iPvtImpl( new QCPixSearcherPrivate( this ) )
    {
    }

QCPixSearcher::~QCPixSearcher()
    {
    delete iPvtImpl;
    }

QCPixSearcher* QCPixSearcher::newInstance()
    {
    QCPixSearcher* searcher = NULL;
    try{
        searcher = new QCPixSearcher( QString() );
        searcher->iPvtImpl->Construct( QString() );
    }
    catch(...){
        delete searcher;
        return NULL;
    }
    return searcher;
    }

QCPixSearcher* QCPixSearcher::newInstance( QString aBaseAppClass, QString aDefaultSearchField )
    {
    QCPixSearcher* searcher = NULL;
    try{
        searcher = new QCPixSearcher( aDefaultSearchField );
        searcher->iPvtImpl->Construct( aDefaultSearchField );
        searcher->setDatabase( aBaseAppClass );
    }
    catch(...){
        delete searcher->iPvtImpl;
        return NULL;
    }
    return searcher;
    }

void QCPixSearcher::setDatabase( QString aBaseAppClass )
    {
    TBuf<KMaxStringLength> baseAppClass( aBaseAppClass.utf16() );
    QT_TRAP_THROWING( iPvtImpl->iSearcher->OpenDatabaseL( baseAppClass ) );
    }

void QCPixSearcher::setDatabaseAsync( QString aBaseAppClass )
    {
    TBuf<KMaxStringLength> baseAppClass( aBaseAppClass.utf16() );
    QT_TRAP_THROWING( iPvtImpl->iSearcher->OpenDatabaseL( *iPvtImpl, baseAppClass ) );
    }

int QCPixSearcher::search( QString aSearchString, QString aDefaultSearchField )
    {
    TBuf<KMaxStringLength> searchString( aSearchString.utf16() );
    TBuf<KMaxStringLength> defaultSearchField( aDefaultSearchField.utf16() );
    //ideally would have had just the following single line:
    //QT_TRAP_THROWING( return iPvtImpl->iSearcher->SearchL( searchString, defaultSearchField ) );
    //But the RCVT compiler throws up warnings. The following is just to suppress those warnings.
    int tmp = 0;
    QT_TRAP_THROWING( tmp = iPvtImpl->iSearcher->SearchL( searchString, defaultSearchField ) );
    return tmp;
    }

void QCPixSearcher::searchAsync( QString aSearchString, QString aDefaultSearchField )
    {
    TBuf<KMaxStringLength> searchString( aSearchString.utf16() );
    TBuf<KMaxStringLength> defaultSearchField( aDefaultSearchField.utf16() );
    QT_TRAP_THROWING( iPvtImpl->iSearcher->SearchL( *iPvtImpl, searchString, defaultSearchField ) );
    }

QCPixDocument* QCPixSearcher::getDocument( int aIndex )
    {
    QCPixDocument* tmp = 0;
    QT_TRAP_THROWING( tmp = QCPixDocFromCPixDoc( iPvtImpl->iSearcher->GetDocumentL( aIndex ) ) );
    return tmp;
    }
 
void QCPixSearcher::getDocumentAsync( int aIndex )
    {
    QT_TRAP_THROWING( iPvtImpl->iSearcher->GetDocumentL( aIndex, *iPvtImpl ) );
    }

void QCPixSearcher::cancelSearch()
    {
    iPvtImpl->iSearcher->Cancel();
    }
