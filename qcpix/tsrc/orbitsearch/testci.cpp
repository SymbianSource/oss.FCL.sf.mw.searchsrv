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
#include <hbtextedit.h>
#include <qsql.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include "testci.h"

TestCI::TestCI( )
{
}

TestCI::~TestCI()
{

}

void TestCI::updatedbcontent( HbTextEdit* aHbTextEdit )
    {
    QString mConnectionName("cpixcontentinfo.sq");
    QString mDatabaseName("c:\\Private\\2001f6fb\\cpixcontentinfo.sq");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", mConnectionName);
    db.setDatabaseName(mDatabaseName);

    QSqlQuery query(QSqlDatabase::database(mConnectionName));
    QString statement = "SELECT * FROM table1";

    query.prepare(statement);
    query.exec();
    QString resultinstring("Database View : \n");
    resultinstring.append( "ContentName   INS   BLS  \n" );
    resultinstring.append( "--------------------------------\n" );
    
    while (query.next())
       {
       QString category_name = query.value(0).toString();
       int ins = query.value(1).toInt ();
       int bls = query.value(2).toInt ( );
       resultinstring  += category_name + "   " +  QString().setNum( ins ) + "   " +QString().setNum( bls ) + "\n";
       aHbTextEdit->setPlainText ( resultinstring );  
       }
    
    db.close();
    return;   
    }

void TestCI::doCITest()
{
    //Declare controls
    HbTextEdit* resultsBox;
    
    HbMainWindow *BigScreen = new HbMainWindow();
    HbWidget *w = new HbWidget;
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout;
    layout->setOrientation(Qt::Vertical);
        
    //Initialize controls    
    resultsBox = new HbTextEdit;
    layout->addItem( resultsBox );
    
    resultsBox->setReadOnly( true );    
    
    updatedbcontent ( resultsBox );
    
    w->setLayout( layout );
    BigScreen->addView ( w );
    BigScreen->show();    
}
