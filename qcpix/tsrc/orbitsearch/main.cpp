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
#include <hbpushbutton.h>
#include <hbtextedit.h>
#include "testsearch.h"
#include "testci.h"

int main(int argc, char *argv[])
{
    HbApplication a(argc, argv);
    
    //Declare controls    
    HbTextEdit* choiceBox;
    HbPushButton* schButton;//button to start search test
    HbPushButton* ciButton; //button to start content info test
    HbMainWindow mainWindow;
    HbWidget *w = new HbWidget;
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout;
    layout->setOrientation(Qt::Vertical);
    
    //Initialize controls
    choiceBox = new HbTextEdit();
    layout->addItem( choiceBox );
    
    schButton = new HbPushButton( "Test Search" );
    layout->addItem( schButton );
    
    ciButton = new HbPushButton( "Test Content Info" );
    layout->addItem( ciButton );    
    choiceBox->setPlainText ("Select the Test : ");
    TestSearch* ts = new TestSearch( );
    QObject::connect( schButton, SIGNAL( clicked() ), ts, SLOT( doSearchTest() ) );
    
    TestCI* tc = new TestCI( );
    QObject::connect( ciButton, SIGNAL( clicked() ), tc, SLOT( doCITest() ) );
    
    w->setLayout( layout );
    mainWindow.addView( w );
  
    mainWindow.show();
    
    return a.exec();
}
