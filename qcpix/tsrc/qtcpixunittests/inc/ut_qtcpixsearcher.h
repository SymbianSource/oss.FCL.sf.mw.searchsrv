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

#ifndef UT_QTCPIXSEARCHER_H_
#define UT_QTCPIXSEARCHER_H_

#include <QtTest/QtTest>

class QCPixSearcher;
class QCPixDocument;

class TestQCPixSearcher: public QObject
    {
    Q_OBJECT
public slots:
    void HandleDatabaseSet(int aError);
    void HandleSearchResults(int aError, int aEstimatedCount);
    void HandleDocument(int aError, QCPixDocument* aDoc);

private slots:
    void testNewInstance();
    void testSetDatabase();
    void testSetDatabase_data();
    void testSetDatabaseAsync();
    void testSetDatabaseAsync_data();
    void testSearch();
    void testSearch_data();
    void testSearchAsync();
    void testSearchAsync_data();
    void testGetDocument();
    void testGetDocument_data();
    void testGetDocumentAsync();
    void testGetDocumentAsync_data();
    
    //private slots that will be called before and after EACH test case.
    void init();
    void cleanup();
   
private:
    QCPixSearcher* iSearcher;
    QCPixDocument* iDoc;
    bool iHandleSetDatabaseComplete;
    int iHandleSearchResultsComplete;
    bool iHandleGetDocumentComplete;
    int iError;
    };

#endif /* UT_QTCPIXSEARCHER_H_ */
