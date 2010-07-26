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

#include "searchhelper.h"
#include <cpixdocument.h>

SearchHelper::SearchHelper(HbLineEdit* searchBx, HbPushButton* searchBtn, HbTextEdit* searchRslt)
    :searchBox( searchBx ), searchButton( searchBtn ), resultsBox( searchRslt )
    {
    searcher = CpixSearcher::newInstance("root","_aggregate");
    resultsBox->setReadOnly( true );
    resultsBox->setPlainText("Initialized");
    searchTime.start();
    }

SearchHelper::~SearchHelper()
    {
    delete searcher;
    }

void SearchHelper::doSearch()
    {
    resultsBox->setPlainText("Search button clicked!");
    
    int hits = 0;
    QString resultString("");
    resultsBox->setPlainText( resultString );
    searchTime.restart();
    QString searchString;
    
#if PREFIX_SEARCH
    searchString = "$prefix(\""; 
    searchString += searchBox->text();
    searchString += "\")";

#elif STAR_SEARCH
    searchString += searchBox->text();
    searchString += "*";
#elif NO_STAR_SEARCH
        ;//do nothing
#endif
    
    hits = searcher->search( searchString );

    if (searchTime.elapsed() >= 0)
        resultString = "SearchTime: " + QString().setNum( searchTime.elapsed() ) + " ms \r\n";
    resultString += "Hits: " + QString().setNum( hits ) + "\r\n";
    resultsBox->setPlainText( resultString );

#if !DONT_SHOW_RESULTS
    if( hits > 0 )
        {
        CpixDocument* temp = NULL;
        int docCount = 0;
        do{
          temp = searcher->document( docCount++ );
          resultString += temp->baseAppClass() + " " + temp->docId() + " " + temp->excerpt() + "\r\n\r\n";
          delete temp;
          }while( hits > docCount );
        }
    resultsBox->setPlainText( resultString );
#endif //DONT_SHOW_RESULTS
    }
