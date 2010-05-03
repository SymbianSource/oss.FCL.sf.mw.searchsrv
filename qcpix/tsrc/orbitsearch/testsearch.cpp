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

#include <QtGui>
#include <QGraphicsLayout>
#include <hbmainwindow.h>
#include <hbwidget.h>
#include "uicontrols.h"
#include "testsearch.h"
#include "searchhelper.h"

TestSearch::TestSearch( )
{
}

TestSearch::~TestSearch()
{

}
	
void TestSearch::doSearchTest()
{
    //Declare controls
    ORBIT_SEARCH_CONTROLS
    HbMainWindow *BigScreen = new HbMainWindow();
    HbWidget *w = new HbWidget;
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout;
    layout->setOrientation(Qt::Vertical);
        
    //Initialize controls
    searchBox = new HbLineEdit;
    layout->addItem( searchBox );
    
    searchButton = new HbPushButton( "Search" );
    layout->addItem( searchButton );
    
    resultsBox = new HbTextEdit;
    layout->addItem( resultsBox );
    
    SearchHelper* sh = new SearchHelper(searchBox, searchButton, resultsBox);  
    QObject::connect( searchButton, SIGNAL( clicked() ), sh, SLOT(doSearch()) );
    
    w->setLayout( layout );
    BigScreen->addView ( w );
    BigScreen->show();    
}
