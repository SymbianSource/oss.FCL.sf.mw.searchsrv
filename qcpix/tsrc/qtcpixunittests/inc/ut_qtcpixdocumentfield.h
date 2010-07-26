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

#ifndef UT_QTCPIXDOCUMENTFIELD_H_
#define UT_QTCPIXDOCUMENTFIELD_H_

#include <QtTest/QtTest>

class CpixDocumentField;
class CpixDocumentFieldTester: public QObject
    {
    Q_OBJECT
private slots:
    void testNewInstance();
    void testGetSetName();
    void testGetSetName_data();
    void testGetSetValue();
    void testGetSetValue_data();
    void testGetSetConfig();
    void testGetSetConfig_data();
    
    //private slots that are run before and after each testcase.
    void init();
    void cleanup();
    
private:
    CpixDocumentField* iField;
    };


#endif /* UT_QTCPIXDOCUMENTFIELD_H_ */
