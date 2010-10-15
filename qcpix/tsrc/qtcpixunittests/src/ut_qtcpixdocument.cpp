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

#include "ut_qtcpixdocument.h"

#include <QtCore>
#include <QCoreApplication>
#include <cpixdocument.h>
#include <cpixdocumentfield.h>
#include <QtTest/QtTest>
#include "../../QtTestUtil/QtTestUtil.h"

void CpixDocumentTester::testNewInstance()
    {
    CpixDocument* doc = CpixDocument::newInstance();
    QVERIFY( doc != NULL );
    delete doc;
    }

void CpixDocumentTester::testGetSetName_data()
    {
    QTest::addColumn<QString>( "baseAppClass" );
    QTest::newRow("root") << "root";
    QTest::newRow("root contact") << "root contact";
    }

void CpixDocumentTester::testGetSetName()
    {
    QFETCH( QString, baseAppClass );
    iDoc->setBaseAppClass( baseAppClass );
    QVERIFY( iDoc->baseAppClass() == baseAppClass );
    }

void CpixDocumentTester::testGetSetDocId_data()
    {
    QTest::addColumn<QString>( "docId" );
    QTest::newRow("C:\\some\\doc\\id") << "C:\\some\\doc\\id";
    }

void CpixDocumentTester::testGetSetDocId()
    {
    QFETCH( QString, docId );
    iDoc->setDocId( docId );
    QVERIFY( iDoc->docId() == docId );
    }

void CpixDocumentTester::testGetSetExcerpt_data()
    {
    QTest::addColumn< QString >( "excerpt" );
    QTest::newRow("this is a sample excerpt") << "this is a sample excerpt";
    }

void CpixDocumentTester::testGetSetExcerpt()
    {
    QFETCH( QString, excerpt );
    iDoc->setExcerpt( excerpt );
    QVERIFY( iDoc->excerpt() == excerpt );
    }

void CpixDocumentTester::testGetFieldCount()
    {
    iDoc->addField("testName", "testValue", 10);
    iDoc->addField("testName2", "testValue2", 12);
    QVERIFY( iDoc->fieldCount() == 2 );
    }

void CpixDocumentTester::testAddGetField()
    {
    iDoc->addField("testName", "testValue", 10);
    iDoc->addField("testName2", "testValue2", 12);
    const CpixDocumentField& field1 = iDoc->field( 0 );
    QVERIFY( field1.name() == "testName" && field1.value() == "testValue" && field1.config() == 10 );
    const CpixDocumentField& field2 = iDoc->field( 1 );
    QVERIFY( field2.name() == "testName2" && field2.value() == "testValue2" && field2.config() == 12 );
    //below two for retriving the field by its name.
    const CpixDocumentField& field3 = iDoc->field("testName");
    QVERIFY( field3.name() == "testName" && field3.value() == "testValue" && field3.config() == 10 );
    const CpixDocumentField& field4 = iDoc->field("testName2");
    QVERIFY( field4.name() == "testName2" && field4.value() == "testValue2" && field4.config() == 12 );
    }

void CpixDocumentTester::init()
    {
    iDoc = CpixDocument::newInstance();
    }

void CpixDocumentTester::cleanup()
    {
    delete iDoc;
    }

QTTESTUTIL_REGISTER_TEST( CpixDocumentTester );
