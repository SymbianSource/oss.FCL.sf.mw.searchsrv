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

#include "cpixsearcherprivate.h"
#include <cpixsearcher.h>
#include <CCPixSearcher.h>
#include <cpixcommon.h>

#include "cpixutils.h"

/**
 * Note: Code in this file should never throw OR leak symbian exceptions.
 * Convert all leaves to C++ exceptions.
 */

CpixSearcherPrivate::CpixSearcherPrivate( QObject* aParent )
    {
    iSearchParent = reinterpret_cast<CpixSearcher*>( aParent );
    }

void CpixSearcherPrivate::Construct( QString aDefaultSearchField )
    {
    qt_symbian_throwIfError( iSearchSession.Connect() );//throw exception on error.
    QT_TRAP_THROWING(
    TBuf<KMaxStringLength> defaultSearchField( aDefaultSearchField.utf16() );
    iSearcher = CCPixSearcher::NewL( iSearchSession, defaultSearchField  ) 
    ); //end of QT_TRAP_THROWING
    }

CpixSearcherPrivate::~CpixSearcherPrivate()
    {
    delete iSearcher;
    //iSearchSession.Close();
    }

void CpixSearcherPrivate::HandleSearchResultsL(TInt aError, TInt aEstimatedResultCount)
    {
    PERF_TIME_NOW("Async search complete");
    emit iSearchParent->handleSearchResults( aError, aEstimatedResultCount );
    }

void CpixSearcherPrivate::HandleDocumentL(TInt aError, CSearchDocument* aDocument)
    {
    PERF_TIME_NOW("Async get document complete")
    emit iSearchParent->handleDocument( aError, CpixDocFromCSearchDocument( aDocument ) );
    }
	
void CpixSearcherPrivate::HandleBatchDocumentL(TInt aError,TInt aRetCount, CSearchDocument** aDocument)
    {
    PERF_TIME_NOW("Async get batch document complete")
    emit iSearchParent->handleBatchDocuments( aError, aRetCount,CpixBatchDocFromCSearchDocument( aRetCount, aDocument) );
    }

void CpixSearcherPrivate::HandleOpenDatabaseResultL( TInt aError )
    {
    emit iSearchParent->handleDatabaseSet( aError );
    }

void CpixSearcherPrivate::HandleSetAnalyzerResultL( TInt /*aError*/ )
    {
    //what is to be done here?
    }
