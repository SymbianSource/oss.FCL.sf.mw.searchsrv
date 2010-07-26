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

#include "ut_qtcpixsearcher.h"

#include <QtCore>
#include <QCoreApplication>
#include <cpixsearcher.h>
#include <cpixdocument.h>
#include <QtTest/QtTest>
#include "../../QtTestUtil/QtTestUtil.h"

//The following data is common to all the functions. So avoid duplication by using the following macros.
#define TEST_SEARCH_DATA \
    QTest::addColumn<QString>("searchString"); \
    QTest::addColumn<int>("estimatedResultCount"); \
    QTest::newRow("Robert") << "Robert" << 53; \
    QTest::newRow("Leonar") << "Leonar" << 74; \
    QTest::newRow("Leas") << "Leas" << 8; \
    QTest::newRow("Not Found") << "Not Found" << 0; \
    QTest::newRow("This is a very long string. Trying to make the search string more than 255 characters. Is it more than 255 yet? Not sure. Dont think so. Guess this should do. No It does not. How much more do I have to type to make the string 255 characters long...bored of typing...hope this would do...Yes, now about 300 chars!") << "Long String" << 0;
    
#define TEST_SEARCH_DATA_FETCH \
    QFETCH( QString, searchString ); \
    QFETCH( int, estimatedResultCount );

#define TEST_GET_DATA \
    QTest::addColumn<QString>("baseAppClass"); \
    QTest::newRow("root") << "root"; \
    QTest::newRow("root contact") << "root contact";

#define TEST_GET_DATA_FETCH \
    QFETCH(QString, baseAppClass);

void TestCpixSearcher::testNewInstance()
    {
    CpixSearcher* searcher = CpixSearcher::newInstance();
    QVERIFY( searcher != NULL );
    delete searcher;
    }

void TestCpixSearcher::init()
    {
    iSearcher = CpixSearcher::newInstance("root", "_aggregate");//default to root searcher.
    QVERIFY( iSearcher!=NULL );
    }

void TestCpixSearcher::cleanup()
    {
    delete iSearcher;
    }

void TestCpixSearcher::testSetDatabase_data()
    {
    TEST_GET_DATA;
    }

void TestCpixSearcher::testSetDatabase()
    {
    TEST_GET_DATA_FETCH;
    Q_ASSERT( iSearcher );
    iSearcher->setDatabase( baseAppClass );//no way to verify this :(
    }

void TestCpixSearcher::HandleDatabaseSet(int aError)
    {
    iHandleSetDatabaseComplete = true;
    iError = aError;
    }

void TestCpixSearcher::testSetDatabaseAsync_data()
    {
    TEST_GET_DATA;
    }

void TestCpixSearcher::testSetDatabaseAsync()
    {
    TEST_GET_DATA_FETCH;
    Q_ASSERT( iSearcher );
    iHandleSetDatabaseComplete = false;
    iError = -2 /*General: Something non-specific*/;
    connect( iSearcher, SIGNAL(handleDatabaseSet(int)), this, SLOT(HandleDatabaseSet(int)) );
    iSearcher->setDatabaseAsync( baseAppClass );
    while( !iHandleSetDatabaseComplete ) QCoreApplication::processEvents( QEventLoop::WaitForMoreEvents );
    QVERIFY( iError == 0/*No error*/ );
    }

void TestCpixSearcher::testSearch_data()
    {
    TEST_SEARCH_DATA;
    }

void TestCpixSearcher::testSearch()
    {
    TEST_SEARCH_DATA_FETCH;
    Q_ASSERT( iSearcher );
    QVERIFY(  iSearcher->search( searchString ) == estimatedResultCount );
    }

void TestCpixSearcher::HandleSearchResults(int aError, int aEstimatedCount)
    {
    if( aError == 0/*None*/ ) iHandleSearchResultsComplete = aEstimatedCount;
    else iHandleSearchResultsComplete = 0;
    }

void TestCpixSearcher::testSearchAsync_data()
    {
    TEST_SEARCH_DATA;
    }

void TestCpixSearcher::testSearchAsync()
    {
    TEST_SEARCH_DATA_FETCH;
    Q_ASSERT( iSearcher );
    iHandleSearchResultsComplete = -1;
    
    //Do Search
    connect( iSearcher, SIGNAL(handleSearchResults(int,int)), this, SLOT(HandleSearchResults(int,int)) );
    iSearcher->cancelSearch();
    iSearcher->searchAsync( searchString );
    
    //Wait for SearchAsync to complete.
    while( iHandleSearchResultsComplete == -1 ) QCoreApplication::processEvents( QEventLoop::WaitForMoreEvents );
    QVERIFY( iHandleSearchResultsComplete == estimatedResultCount );
    }

void TestCpixSearcher::testGetDocument_data()
    {
    TEST_SEARCH_DATA;
    }

void TestCpixSearcher::testGetDocument()
    {
    TEST_SEARCH_DATA_FETCH;
    Q_ASSERT( iSearcher );
    QVERIFY( iSearcher->search( searchString ) == estimatedResultCount );

    //Check only baseAppClass for now. Not sure about the order of docs returned.
    //Hence check that we are getting "Robert" from contacts.
    if( estimatedResultCount>0 ){
        CpixDocument* doc = iSearcher->document( 0 );
        QVERIFY( doc->baseAppClass() == "root contact" );
    }
    else return;
    }

void TestCpixSearcher::HandleDocument(int /*aError*/, CpixDocument* aDoc)
    {
    iHandleGetDocumentComplete = true;
    iDoc = aDoc;
    }

void TestCpixSearcher::testGetDocumentAsync_data()
    {
    TEST_SEARCH_DATA;
    }

void TestCpixSearcher::testGetDocumentAsync()
    {
    TEST_SEARCH_DATA_FETCH;
    Q_ASSERT( iSearcher );
    iHandleGetDocumentComplete = false;
    iHandleSearchResultsComplete = -1;
    
    //Do Search
    iSearcher->connect( iSearcher, SIGNAL(handleSearchResults(int,int)), this, SLOT(HandleSearchResults(int,int)) );
    iSearcher->searchAsync( searchString );
    while( iHandleSearchResultsComplete == -1 ) QCoreApplication::processEvents( QEventLoop::WaitForMoreEvents );
    QVERIFY( iHandleSearchResultsComplete == estimatedResultCount );
    
    //Get Document
    connect( iSearcher, SIGNAL(handleDocument(int,CpixDocument*)), this, SLOT(HandleDocument(int,CpixDocument*)) );
    if( iHandleSearchResultsComplete>0 ) 
        iSearcher->documentAsync( 0 );
    else 
        return;
    
    //Wait for getDocumentAsync to complete.
    while( !iHandleGetDocumentComplete ) QCoreApplication::processEvents( QEventLoop::WaitForMoreEvents );
    QVERIFY( iDoc->baseAppClass() == "root contact" );
    }

QTTESTUTIL_REGISTER_TEST( TestCpixSearcher );
