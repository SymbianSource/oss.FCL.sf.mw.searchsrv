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

#ifndef SAMPLESEARCH_H
#define SAMPLESEARCH_H

#include <QtGui/QMainWindow>
#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDesktopWidget>
#include <QApplication>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QVector>

class CSearchHandler;
class CpixDocument;
class DocLaunchInfo;

class SampleSearch : public QWidget
{
    Q_OBJECT

public:
	SampleSearch(QWidget *parent = 0);
    ~SampleSearch();
private:
    QLineEdit* txtSearch;
    QPushButton* btnSearch;
    QPushButton* btnSearchAsync;
    QTableWidget* dataTable;
    int noOfItemsPerPage;
    int iCurrentPageNum;
    CSearchHandler* iSearchHandler;
    int iSearchResultsCount;
    int noOfPages; 
    QPushButton* btnNext;
    QPushButton* btnPrev;
    QVBoxLayout* rootLayout;
    QHBoxLayout* rowLayout;
    QHBoxLayout* rowButtonLayout;
    QLabel* lbCaption;
    QHBoxLayout* layoutForNavButton;
    QLabel * iSearchResultLabel;
    QLineEdit* txtSearchResultCount;
    QLabel * iPageInfoLabel;
    QHBoxLayout* searchResultLayout;
    int currentRow;
    int nextDocumentIndex;
    bool asyncSearch;
    QVector<DocLaunchInfo*> docInfo;
    QMessageBox mBox;
    
private:
    void clearSearchResults();
    int getStartIndexForPage();
    void createSearchResultTableAndAddContents();
    void searchCompleteHelper(int aError, int aResultCount);
    void setSearchResultsHeader();
    void addDocumentToResultsPage( CpixDocument* aSearchDoc, int rowCount );
    void openMessaging( int aMessagingId );
    void doLaunchDoc(DocLaunchInfo& info);
    
private slots:
    void launchDoc(int,int);
    void doSearch();
    void doSearchAsync();
    void onSearchComplete(int aError, int aResultCount);
    void onAsyncSearchComplete(int aError, int aResultCount);
    void onGetDocumentComplete(int aError, CpixDocument* aDoc);
    void displayNextPage();
    void displayPrevPage();   
};

#endif // SAMPLESEARCH_H
