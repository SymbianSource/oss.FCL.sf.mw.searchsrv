/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
 * Description:  Tests for SearchStateProvider class.
 *
 */

#include <QtTest/QtTest>
#include "contentinfodbparser.h"
#include "contentinfodbupdate.h"
#include "contentinfodbreadprivate.h"
class ContentinfodbTest : public QObject
    {
Q_OBJECT

public:
    ContentinfodbTest();
    ~ContentinfodbTest();

private slots:
    void init();
    void initTestCase();
    void cleanupTestCase();
    void Testupdate();
    void TestRead();
    void TestDelete();
    void TestCount();
    };

