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

#include "qcpixsearcherprivate.h"
#include <qcpixsearcher.h>
#include <CCPixSearcher.h>
#include <qcpixcommon.h>

#include "qcpixutils.h"

/**
 * Note: Code in this file should never throw OR leak symbian exceptions.
 * Convert all leaves to C++ exceptions.
 */

QCPixSearcherPrivate::QCPixSearcherPrivate( QObject* aParent )
    {
    iSearchParent = reinterpret_cast<QCPixSearcher*>( aParent );
    }

void QCPixSearcherPrivate::Construct( QString aDefaultSearchField )
    {
    qt_symbian_throwIfError( iSearchSession.Connect() );//throw exception on error.
    TBuf<KMaxStringLength> defaultSearchField( aDefaultSearchField.utf16() );
    QT_TRAP_THROWING( iSearcher = CCPixSearcher::NewL( iSearchSession, defaultSearchField  ) );
    }

QCPixSearcherPrivate::~QCPixSearcherPrivate()
    {
    delete iSearcher;
    //iSearchSession.Close();
    }

void QCPixSearcherPrivate::HandleSearchResultsL(TInt aError, TInt aEstimatedResultCount)
    {
    PERF_TIME_NOW("Async search complete");
    emit iSearchParent->handleSearchResults( aError, aEstimatedResultCount );
    }

void QCPixSearcherPrivate::HandleDocumentL(TInt aError, CSearchDocument* aDocument)
    {
    PERF_TIME_NOW("Async get document complete")
    emit iSearchParent->handleDocument( aError, QCPixDocFromCPixDoc( aDocument ) );
    }

void QCPixSearcherPrivate::HandleOpenDatabaseResultL( TInt aError )
    {
    emit iSearchParent->handleDatabaseSet( aError );
    }

void QCPixSearcherPrivate::HandleSetAnalyzerResultL( TInt /*aError*/ )
    {
    //what is to be done here?
    }
