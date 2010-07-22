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
    PERF_SEARCH_START_TIMER
    PERF_GETDOC_START_TIMER
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
    QT_TRAP_THROWING( 
    TBuf<KMaxStringLength> baseAppClass( aBaseAppClass.utf16() );
    iPvtImpl->iSearcher->OpenDatabaseL( baseAppClass ) 
    ); //end of QT_TRAP_THROWING
    }

void QCPixSearcher::setDatabaseAsync( QString aBaseAppClass )
    {
    QT_TRAP_THROWING( 
    TBuf<KMaxStringLength> baseAppClass( aBaseAppClass.utf16() );
    iPvtImpl->iSearcher->OpenDatabaseL( *iPvtImpl, baseAppClass ) 
    ); //end of QT_TRAP_THROWING
    }

//The following bit of code is common to two functions - this helps to avoid duplication.
//However, macros make debugging difficult - so, if you need to debug, copy the code below
//and replace the macro, fix the code and bring the fix back to the macro.
#define CREATE_SEARCH_VARS \
        HBufC* searchString = HBufC::NewL( aSearchString.length() + 1 );                    \
        TPtrC searchStringPtr( reinterpret_cast<const TUint16*>( aSearchString.utf16() ) ); \
        searchString->Des().Copy( searchStringPtr );                                        \
                                                                                            \
        HBufC* defaultSearchField = HBufC::NewL( aDefaultSearchField.length() + 1 );        \
        TPtrC aDefaultSearchFieldPtr( reinterpret_cast<const TUint16*>( aDefaultSearchField.utf16() ) );\
        defaultSearchField->Des().Copy( aDefaultSearchFieldPtr );                           

#define DELETE_SEARCH_VARS  \
        delete searchString;\
        delete defaultSearchField;

int QCPixSearcher::search( QString aSearchString, QString aDefaultSearchField )
    {
    PERF_SEARCH_RESTART_TIMER
    int tmp = 0;
    QT_TRAP_THROWING(
        CREATE_SEARCH_VARS;
        //ideally would have had just the following single line:
        //QT_TRAP_THROWING( return iPvtImpl->iSearcher->SearchL( searchString, defaultSearchField ) );
        //But the RCVT compiler throws up warnings. The following is just to suppress those warnings.
        tmp = iPvtImpl->iSearcher->SearchL( *searchString, *defaultSearchField );
        DELETE_SEARCH_VARS;
    ); //QT_TRAP_THROWING
    
    PERF_SEARCH_ENDLOG
    return tmp;
    }

void QCPixSearcher::searchAsync( QString aSearchString, QString aDefaultSearchField )
    {
    PERF_TIME_NOW("Async search start")
    QT_TRAP_THROWING(
        CREATE_SEARCH_VARS;
        iPvtImpl->iSearcher->SearchL( *iPvtImpl, *searchString, *defaultSearchField );
        DELETE_SEARCH_VARS;
    ); //QT_TRAP_THROWING
    }

QCPixDocument* QCPixSearcher::getDocument( int aIndex )
    {
    PERF_GETDOC_RESTART_TIMER
    QCPixDocument* tmp = 0;
    QT_TRAP_THROWING( tmp = QCPixDocFromCPixDoc( iPvtImpl->iSearcher->GetDocumentL( aIndex ) ) );
    PERF_GETDOC_ENDLOG
    return tmp;
    }

void QCPixSearcher::getDocumentAsync( int aIndex )
    {
    PERF_TIME_NOW("Async get document start")
    QT_TRAP_THROWING( iPvtImpl->iSearcher->GetDocumentL( aIndex, *iPvtImpl ) );
    }

void QCPixSearcher::cancelSearch()
    {
    iPvtImpl->iSearcher->Cancel();
    }
