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

#include "ut_qtcpixdocumentfield.h"
#include "../../QtTestUtil/QtTestUtil.h"

#include <qcpixdocumentfield.h>

void QCPixDocumentFieldTester::testNewInstance()
    {
    QCPixDocumentField* field = QCPixDocumentField::newInstance( QString(), QString(), 0 );
    QVERIFY( field != NULL );
    }

void QCPixDocumentFieldTester::testGetSetName_data()
    {
    QTest::addColumn< QString >( "name" );
    
    QTest::newRow("testName") << "testName" ;
    QTest::newRow("testName2") << "testName2" ;
    }

void QCPixDocumentFieldTester::testGetSetName()
    {
    QFETCH( QString, name );
    iField->setName( name );
    QVERIFY( iField->name() == name );
    }

void QCPixDocumentFieldTester::testGetSetValue_data()
    {
    QTest::addColumn< QString >( "value" );
    
    QTest::newRow("testValue") << "testValue" ;
    QTest::newRow("testVlaue2") << "testValue2" ;
    }

void QCPixDocumentFieldTester::testGetSetValue()
    {
    QFETCH( QString, value );
    iField->setValue( value );
    QVERIFY( iField->value() == value );
    }

void QCPixDocumentFieldTester::testGetSetConfig_data()
    {
    QTest::addColumn< int >( "config" );
    
    QTest::newRow("10") << 10 ;
    QTest::newRow("20") << 20 ;
    }

void QCPixDocumentFieldTester::testGetSetConfig()
    {
    QFETCH( int, config );
    iField->setConfig( config );
    QVERIFY( iField->config() == config );
    }

void QCPixDocumentFieldTester::init()
    {
    iField = QCPixDocumentField::newInstance( "testName", "testValue", 100 );
    QVERIFY( iField!=NULL && iField->name()=="testName" && iField->value()=="testValue" && iField->config()==100 );
    }

void QCPixDocumentFieldTester::cleanup()
    {
    delete iField;
    }

QTTESTUTIL_REGISTER_TEST( QCPixDocumentFieldTester );
