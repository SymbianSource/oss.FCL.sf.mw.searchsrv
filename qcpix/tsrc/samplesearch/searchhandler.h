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

#include <QtCore>
#include <QCoreApplication>
#include <QDebug>
#include <QTest>

class QCPixSearcher;
class QCPixDocument;

class CSearchHandler: public QObject
    {
Q_OBJECT
public:
    CSearchHandler();
    
signals:
    void handleSearchResult(int, int);
    void handleAsyncSearchResult(int, int);
    void handleDocument(int aError, QCPixDocument* aDoc);

private slots:
    void __handleSearchResult(int aError, int estimatedResultCount);
    void __getDocumentAsync(int aError, QCPixDocument* aDocument );

public:
    ~CSearchHandler();
    QCPixDocument* getDocumentAtIndex(int aIndex);
    void getDocumentAsyncAtIndex(int aIndex);
    void search(QString aSearchString);
    void searchAsync(QString aSearchString);
    void cancelLastSearch();
    int getSearchResultCount();

private:
    QCPixSearcher *iSearchInterface;
    QList<QVariantMap> searchResults;
    int iSearchresultCount;
    };
