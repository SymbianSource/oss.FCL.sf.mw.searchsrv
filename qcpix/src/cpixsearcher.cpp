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

#include <cpixsearcher.h>
#include <cpixcommon.h>
#include <CCPixSearcher.h>
#include <CSearchDocument.h>

#include "cpixsearcherprivate.h"
#include "cpixutils.h"

/**
 * Note: Code in this file should never throw OR leak symbian exceptions.
 * Convert all leaves to C++ exceptions.
 */

CpixSearcher::CpixSearcher()
    :iPvtImpl( new CpixSearcherPrivate( this ) )
    {
    PERF_SEARCH_START_TIMER
    PERF_GETDOC_START_TIMER
    }

CpixSearcher::~CpixSearcher()
    {
    delete iPvtImpl;
    }

CpixSearcher* CpixSearcher::newInstance()
    {
    CpixSearcher* searcher = NULL;
    try{
        searcher = new CpixSearcher();
        searcher->iPvtImpl->Construct( QString() );
    }
    catch(...){
        delete searcher;
        return NULL;
    }
    return searcher;
    }

CpixSearcher* CpixSearcher::newInstance( QString aBaseAppClass, QString aDefaultSearchField )
    {
    CpixSearcher* searcher = NULL;
    try{
        searcher = new CpixSearcher();
        searcher->iPvtImpl->Construct( aDefaultSearchField );
        searcher->setDatabase( aBaseAppClass );
    }
    catch(...){
        delete searcher->iPvtImpl;
        return NULL;
    }
    return searcher;
    }

void CpixSearcher::setDatabase( QString aBaseAppClass )
    {
    QT_TRAP_THROWING( 
    TBuf<KMaxStringLength> baseAppClass( aBaseAppClass.utf16() );
    iPvtImpl->iSearcher->OpenDatabaseL( baseAppClass ) ;
    ); //end of QT_TRAP_THROWING
    }

void CpixSearcher::setDatabaseAsync( QString aBaseAppClass )
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

int CpixSearcher::search( QString aSearchString, QString aDefaultSearchField )
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

void CpixSearcher::searchAsync( QString aSearchString, QString aDefaultSearchField )
    {
    PERF_TIME_NOW("Async search start")
    QT_TRAP_THROWING(
        CREATE_SEARCH_VARS;
        iPvtImpl->iSearcher->SearchL( *iPvtImpl, *searchString, *defaultSearchField );
        DELETE_SEARCH_VARS;
    ); //QT_TRAP_THROWING
    }

CpixDocument* CpixSearcher::document( int aIndex )
    {
    PERF_GETDOC_RESTART_TIMER
    CpixDocument* tmp = 0;
    QT_TRAP_THROWING( tmp = CpixDocFromCSearchDocument( iPvtImpl->iSearcher->GetDocumentL( aIndex ) ) );
    PERF_GETDOC_ENDLOG
    return tmp;
    }

void CpixSearcher::documentAsync( int aIndex )
    {
    PERF_TIME_NOW("Async get document start")
    QT_TRAP_THROWING( iPvtImpl->iSearcher->GetDocumentL( aIndex, *iPvtImpl ) );
    }

void CpixSearcher::cancelSearch()
    {
    iPvtImpl->iSearcher->Cancel();
    }
