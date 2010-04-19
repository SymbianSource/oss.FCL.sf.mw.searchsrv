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
#include <hbapplication.h>
#include <hbmainwindow.h>
#include <hbwidget.h>

#include "searchhelper.h"

int main(int argc, char *argv[])
{
    HbApplication a(argc, argv);
    
    //Declare controls
    ORBIT_SEARCH_CONTROLS
    
    HbMainWindow mainWindow;
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
    mainWindow.addView( w );
  
    mainWindow.show();
    
    return a.exec();
}
