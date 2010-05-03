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
#ifndef TESTCI_H_
#define TESTCI_H_

#include <QObject>
#include <hbtextedit.h>
class TestCI: public QObject
    {
Q_OBJECT
public:
    TestCI( );
    ~TestCI();

public slots:
    void doCITest();
    
private:
    void updatedbcontent( HbTextEdit* aHbTextEdit );
    
	};

#endif //TESTCI_H_
