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

#ifndef HELPER_H_
#define HELPER_H_

#include <QObject>
#include "uicontrols.h"
#include <cpixsearcher.h>
#include <QTime>

class SearchHelper: public QObject
    {
Q_OBJECT
public:
    SearchHelper(HbLineEdit*, HbPushButton*, HbTextEdit*, HbLineEdit*, HbLineEdit*, HbPushButton* );
    ~SearchHelper();

public slots:
    void doSearch();
    void showdocs();
    
private:
    ORBIT_SEARCH_CONTROLS //Not owned.
    CpixSearcher* searcher;//Owned
    QTime searchTime;
    QString resultString;
    int iHits;
    };

#endif /* HELPER_H_ */
