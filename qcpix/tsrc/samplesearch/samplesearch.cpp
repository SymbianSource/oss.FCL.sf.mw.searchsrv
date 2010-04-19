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

#include "SampleSearch.h"
#include "searchhandler.h"
#include <QCPixDocument.h>
#include <xqservicerequest.h>

SampleSearch::SampleSearch(QWidget */*parent*/)
{
    iSearchHandler = new CSearchHandler();
    noOfItemsPerPage = 15;
    iCurrentPageNum =0;
    noOfPages =0;
    asyncSearch = false;
    //QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect clientRect(0,0,360,640);// = desktopWidget->availableGeometry();
    setGeometry(clientRect);
    rootLayout = new QVBoxLayout();
    
    rowLayout = new QHBoxLayout();
    lbCaption = new QLabel("Search");
    //lbCaption->setText(tr("Search"));
    rowLayout->addWidget(lbCaption);

    txtSearch = new QLineEdit();
    rowLayout->addWidget(txtSearch);
    rootLayout->addLayout(rowLayout);

    rowButtonLayout = new QHBoxLayout();
    
    btnSearch = new QPushButton();
    btnSearch->setText(tr("Search"));
    rowButtonLayout->addWidget(btnSearch);
    
    btnSearchAsync = new  QPushButton();
    btnSearchAsync->setText(tr("Search ASync"));
    rowButtonLayout->addWidget(btnSearchAsync);

    rootLayout->addLayout(rowButtonLayout);
    
    searchResultLayout = new QHBoxLayout();
    iSearchResultLabel = new QLabel("Search Result Count:");
    searchResultLayout->addWidget(iSearchResultLabel);
    
    txtSearchResultCount = new QLineEdit();
    txtSearchResultCount->setText("0");
    searchResultLayout->addWidget(txtSearchResultCount);
    
    rootLayout->addLayout(searchResultLayout);
    
    iPageInfoLabel = new QLabel();
    iPageInfoLabel->setText("No results to display");
    rootLayout->addWidget(iPageInfoLabel);
 
    
    layoutForNavButton = new QHBoxLayout();
     
    btnPrev = new QPushButton();
    btnPrev->setText(tr("Prev"));
    layoutForNavButton->addWidget(btnPrev);
    connect(btnPrev, SIGNAL(clicked()),this, SLOT(displayPrevPage()));
    rootLayout->addLayout(layoutForNavButton);
    btnPrev->setEnabled(true);
    
    btnNext = new QPushButton();
    btnNext->setText(tr("Next"));
    layoutForNavButton->addWidget(btnNext);
    connect(btnNext, SIGNAL(clicked()),this, SLOT(displayNextPage()));
    btnNext->setEnabled(true);

    dataTable = new QTableWidget();
    dataTable->setColumnCount(3);
    QStringList headings;
    headings << tr("Class")
             << tr("Doc Id")
             << tr("Excerpt");
    dataTable->setHorizontalHeaderLabels(headings);
    rootLayout->addWidget(dataTable);    
    
    setLayout(rootLayout);
    setWindowTitle(tr("Search"));
    connect(btnSearch, SIGNAL(clicked()),this, SLOT(doSearch()));
    connect(btnSearchAsync, SIGNAL(clicked()),this, SLOT(doSearchAsync()));
    connect(iSearchHandler, SIGNAL(handleSearchResult(int,int)),this, SLOT(onSearchComplete(int,int)));
    connect(iSearchHandler, SIGNAL(handleAsyncSearchResult(int,int)),this, SLOT(onAsyncSearchComplete(int,int)));
    connect(iSearchHandler, SIGNAL(handleDocument(int,QCPixDocument*)),this, SLOT(onGetDocumentComplete(int,QCPixDocument*)));
}

SampleSearch::~SampleSearch()
{
    delete dataTable;
    delete btnSearch;
    delete txtSearch;  
    delete btnPrev;
    delete btnNext;
    delete rowLayout;
    delete lbCaption;
    delete iPageInfoLabel;
    delete iSearchResultLabel;
    delete txtSearchResultCount;    
    delete layoutForNavButton;
    delete searchResultLayout;
    delete rowButtonLayout;
    delete rootLayout;

    delete iSearchHandler;
}

void SampleSearch::searchCompleteHelper(int aError, int aResultCount)
    {
    txtSearchResultCount->setText(QString::number(aResultCount));
    iSearchResultsCount = aResultCount;
    noOfPages = (int)iSearchResultsCount/noOfItemsPerPage;
    if((iSearchResultsCount%noOfItemsPerPage) >0)
        {
        noOfPages++;
        }
    }

void SampleSearch::onAsyncSearchComplete(int aError, int aResultCount)
    {
    clearSearchResults();
    searchCompleteHelper( aError, aResultCount );
    setSearchResultsHeader();
    currentRow = 0;
    nextDocumentIndex = 0;
    iSearchHandler->getDocumentAsyncAtIndex( nextDocumentIndex++ );
    }

void SampleSearch::onGetDocumentComplete(int aError, QCPixDocument* aDoc)
    {
    if( aDoc == NULL ) return;
    addDocumentToResultsPage( aDoc, currentRow++ );
    if( currentRow < noOfItemsPerPage ) 
        iSearchHandler->getDocumentAsyncAtIndex( nextDocumentIndex++ );
    }

void SampleSearch::onSearchComplete(int aError, int aResultCount)
    {
    searchCompleteHelper( aError, aResultCount );
    createSearchResultTableAndAddContents();
    }

void SampleSearch::doSearch()
    {   
    clearSearchResults();
    asyncSearch = false;
    iSearchResultsCount=0;
    iCurrentPageNum =0;
    btnPrev->setEnabled(false);        
    if(iSearchHandler)
        {
        iSearchHandler->cancelLastSearch();
        iSearchHandler->search(txtSearch->text());
        }
    }

void SampleSearch::doSearchAsync()
    {
    clearSearchResults();
    asyncSearch = true;
    iSearchResultsCount=0;
    iCurrentPageNum =0;
    btnPrev->setEnabled(false);        
    if(iSearchHandler)
        {
        iSearchHandler->cancelLastSearch();
        iSearchHandler->searchAsync( txtSearch->text() );
        }
    }

void SampleSearch::clearSearchResults()
    {    
    //Very Very Dirty hack!!!
    rootLayout->removeWidget(dataTable);
    int noOfRows = dataTable->rowCount();
    for(int i=0; i<noOfRows;i++)
        {
        dataTable->removeRow(i);
        }
    
    delete dataTable;
    
    dataTable = new QTableWidget();
    dataTable->setColumnCount(3);
    QStringList headings;
    headings << tr("Class")
             << tr("Doc Id")
             << tr("Excerpt");
    dataTable->setHorizontalHeaderLabels(headings);
    rootLayout->addWidget(dataTable);
    docInfo.clear(); //does this leak objects?
    }

void SampleSearch::displayNextPage()
    {    
    if(iCurrentPageNum <noOfPages)
        {
        iCurrentPageNum++;
        }
    if(iCurrentPageNum==noOfPages-1)
        {
        btnNext->setEnabled(false);
        btnPrev->setEnabled(true);
        }
    else
        {
        btnNext->setEnabled(true);
        btnPrev->setEnabled(true);
        }
    if( !asyncSearch )
        createSearchResultTableAndAddContents();
    else
        {
        currentRow = 0;
        clearSearchResults();
        setSearchResultsHeader();
        iSearchHandler->getDocumentAsyncAtIndex( nextDocumentIndex++ );
        }
    }

void SampleSearch::displayPrevPage()
    {    
    if(iCurrentPageNum >0)
        {
        iCurrentPageNum--;
        }
    if(iCurrentPageNum==0)
        {
        btnPrev->setEnabled(false);
        btnNext->setEnabled(true);
        }
    else
        {
        btnPrev->setEnabled(true);
        btnNext->setEnabled(true);
        }
    if( !asyncSearch )
        createSearchResultTableAndAddContents();
    else
        {
        currentRow = 0;
        nextDocumentIndex = iCurrentPageNum*noOfItemsPerPage;
        clearSearchResults();
        setSearchResultsHeader();
        iSearchHandler->getDocumentAsyncAtIndex( nextDocumentIndex++ );
        }
    }

int SampleSearch::getStartIndexForPage()
    {
    return iCurrentPageNum*noOfItemsPerPage;
    }

void SampleSearch::setSearchResultsHeader()
    {
    QString strText;
    strText.append("Displaying results for Page (");
    strText.append(QString::number(iCurrentPageNum+1));
    strText.append(" of ");
    strText.append(QString::number(noOfPages));
    strText.append(")");
    iPageInfoLabel->setText(strText);
    }

void SampleSearch::createSearchResultTableAndAddContents()
    {
    clearSearchResults();
    setSearchResultsHeader();
    int startIndex = getStartIndexForPage();
    int endIndex = startIndex + noOfItemsPerPage;
    if(endIndex >iSearchResultsCount )
        {
        endIndex = iSearchResultsCount;
        }
    int rowCount=0;
    for(int i =startIndex;i<endIndex;i++)
        {
        if(rowCount<noOfItemsPerPage)
            {
            QCPixDocument* aSearchDoc = iSearchHandler->getDocumentAtIndex(i);
            addDocumentToResultsPage( aSearchDoc, rowCount );
            rowCount++;
            }
        }
    }

//This class is first needed in the function below
class DocLaunchInfo{
public:
    DocLaunchInfo(QString aType, QString aDocId)
    :type(aType), docId(aDocId)
    {}
    
public:
    QString type;
    QString docId;
};

void SampleSearch::addDocumentToResultsPage( QCPixDocument* aSearchDoc, int rowCount )
    {
    QTableWidgetItem *appClassCol = new QTableWidgetItem("Col1");
    appClassCol->setFlags(Qt::ItemIsEnabled);

    QTableWidgetItem *docuidCol = new QTableWidgetItem("Col2");    
    docuidCol->setFlags(Qt::ItemIsEnabled);

    QTableWidgetItem *excerptCol = new QTableWidgetItem("Col3");
    excerptCol->setFlags(Qt::ItemIsEnabled);

    dataTable->insertRow(rowCount);
    connect( dataTable, SIGNAL( cellClicked(int,int) ), this, SLOT( launchDoc(int,int) ) );
    dataTable->setItem(rowCount,0,appClassCol);
    dataTable->setItem(rowCount,1,docuidCol);
    dataTable->setItem(rowCount,2,excerptCol);
    
    if(appClassCol)
        {
        appClassCol->setText( aSearchDoc->baseAppClass() );
        }

    if(docuidCol)
        {
        docuidCol->setText( aSearchDoc->docId() );
        }

    if(excerptCol)
        {
        excerptCol->setText( aSearchDoc->excerpt() );
        }
    
    docInfo.push_back( new DocLaunchInfo( aSearchDoc->baseAppClass(), aSearchDoc->docId() ) );
    }
    
void SampleSearch::doLaunchDoc(DocLaunchInfo& info)
    {
    //Instead of having these traps all over the place, we'll have it just here.
    QT_TRAP_THROWING(/* Call all the symbian APIs here */);
    if( info.type == "root calendar" ){
        XQServiceRequest *snd = new XQServiceRequest("com.nokia.services.phonebookservices.Fetch", "open(int)",false);
        *snd << info.docId.toInt();
        snd->send();
        }
    else mBox.show();
    }

void SampleSearch::launchDoc(int row,int /*col*/)
    {
    doLaunchDoc( *docInfo[row] );
    }
