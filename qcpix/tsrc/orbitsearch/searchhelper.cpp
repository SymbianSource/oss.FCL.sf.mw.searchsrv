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
#include <qcpixdocument.h>

SearchHelper::SearchHelper(HbLineEdit* searchBx, HbPushButton* searchBtn, HbTextEdit* searchRslt)
    :searchBox( searchBx ), searchButton( searchBtn ), resultsBox( searchRslt )
    {
    searcher = QCPixSearcher::newInstance("root","_aggregate");
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
    QString searchString = searchBox->text();

#if STAR_SEARCH
    searchString += "*";
#elif NO_STAR_SEARCH
        ;//do nothing
#elif ESCAPE_SPECIAL_CHARS
    //C-style array query - so that we dont have to hard code the length.
    //Escape '\' first so that it does not re-escape all the other escapes.
    QString escapeTheseCharacters [] = {"\\", "+", "-", "&&", "||", "!", 
                                        "(", ")", "{", "}", "[", "]", "^", 
                                        "~", "*", "?", ":", "\0"};
    for( int i=0; escapeTheseCharacters[i] != "\0"; i++ ){
        QString escapedCharacter = "\\" + escapeTheseCharacters[i];
        searchString.replace( escapeTheseCharacters[i], escapedCharacter );
    }
#endif
    
    hits = searcher->search( searchString );
    
    resultString = "SearchTime: " + QString().setNum( searchTime.elapsed() ) + " ms \r\n";
    resultString += "Hits: " + QString().setNum( hits ) + "\r\n";
    resultsBox->setPlainText( resultString );

#if !DONT_SHOW_RESULTS
    if( hits > 0 )
        {
        QCPixDocument* temp = NULL;
        int docCount = 0;
        do{
          temp = searcher->getDocument( docCount++ );
          resultString += temp->baseAppClass() + " " + temp->docId() + " " + temp->excerpt() + "\r\n\r\n";
          delete temp;
          }while( hits > docCount );
        }
    resultsBox->setPlainText( resultString );
#endif //DONT_SHOW_RESULTS
    }
