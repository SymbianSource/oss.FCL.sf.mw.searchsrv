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

#include <cpixdocumentfield.h>

void CpixDocumentFieldTester::testNewInstance()
    {
    CpixDocumentField* field = CpixDocumentField::newInstance( QString(), QString(), 0 );
    QVERIFY( field != NULL );
    }

void CpixDocumentFieldTester::testGetSetName_data()
    {
    QTest::addColumn< QString >( "name" );
    
    QTest::newRow("testName") << "testName" ;
    QTest::newRow("testName2") << "testName2" ;
    }

void CpixDocumentFieldTester::testGetSetName()
    {
    QFETCH( QString, name );
    iField->setName( name );
    QVERIFY( iField->name() == name );
    }

void CpixDocumentFieldTester::testGetSetValue_data()
    {
    QTest::addColumn< QString >( "value" );
    
    QTest::newRow("testValue") << "testValue" ;
    QTest::newRow("testVlaue2") << "testValue2" ;
    }

void CpixDocumentFieldTester::testGetSetValue()
    {
    QFETCH( QString, value );
    iField->setValue( value );
    QVERIFY( iField->value() == value );
    }

void CpixDocumentFieldTester::testGetSetConfig_data()
    {
    QTest::addColumn< int >( "config" );
    
    QTest::newRow("10") << 10 ;
    QTest::newRow("20") << 20 ;
    }

void CpixDocumentFieldTester::testGetSetConfig()
    {
    QFETCH( int, config );
    iField->setConfig( config );
    QVERIFY( iField->config() == config );
    }

void CpixDocumentFieldTester::init()
    {
    //test for getting value of a field in the form of list of strings.
    iField = CpixDocumentField::newInstance( "testName", "one two three", 100 );
    QStringList list = iField->values();
    QString str[3] = {"one", "two", "three"};
    QStringList::iterator i = list.begin();
     for(int j = 0;i !=list.end(); ++i, j++)
         QVERIFY( iField!=NULL && iField->name()=="testName" && *i==str[j] && iField->config()==100 );
     
     
    iField = CpixDocumentField::newInstance( "testName", "testValue", 100 );
    QVERIFY( iField!=NULL && iField->name()=="testName" && iField->value()=="testValue" && iField->config()==100 );
    }

void CpixDocumentFieldTester::cleanup()
    {
    delete iField;
    }

QTTESTUTIL_REGISTER_TEST( CpixDocumentFieldTester );
