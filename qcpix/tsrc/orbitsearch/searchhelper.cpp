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

SearchHelper::SearchHelper(HbLineEdit* searchBx, HbPushButton* searchBtn, HbTextEdit* searchRslt, HbLineEdit* indexBox, HbLineEdit* countBox, HbPushButton* getdocbutton)
    :searchBox( searchBx ), searchButton( searchBtn ), resultsBox( searchRslt ), indexBox(indexBox), countBox(countBox), getdocbutton(getdocbutton)
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
    resultString = "";
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
    searchString = searchBox->text();
        ;//do nothing
#endif
    iHits = 0;
    iHits = searcher->search( searchString );

    if (searchTime.elapsed() >= 0)
        resultString = "SearchTime: " + QString().setNum( searchTime.elapsed() ) + " ms \r\n";
    resultString += "Hits: " + QString().setNum( iHits ) + "\r\n";
    resultsBox->setPlainText( resultString );

#if !DONT_SHOW_RESULTS
    if( iHits > 0 )
        {
        resultString += "Enter Index,count values and press GetDocs button to get the results";
        resultString +="\r\n";
        resultsBox->setPlainText( resultString );
        }
#endif //DONT_SHOW_RESULTS
    }

void SearchHelper::showdocs()
    {
    QString indexstring = indexBox->text();
    bool ok = false;
    int index = indexstring.toInt(&ok);
    if (ok)
        {
        QString countstring = countBox->text();
        int count = countstring.toInt(&ok);
        if (ok)
            {
            if ( index <= iHits )
                {
                if ( count == 1)
                    {
                    //call the normal get doc API
                    CpixDocument* temp = NULL;
                    do{
                      temp = searcher->document( index++ );
                      resultString += temp->baseAppClass() + " " + temp->docId() + " " + temp->excerpt() + "\r\n\r\n";
                      delete temp;
                      }while( iHits > index );
                    }
                else
                    {
                    //call batch doc API
                    CpixDocument** temp = NULL;
                    do{
                       int retdoccount = 0;
                       temp = searcher->batchdocument(index,retdoccount,count);
                       for (int i=0; i< retdoccount; i++)
                           {
                           resultString += temp[i]->baseAppClass() + " " + temp[i]->docId() + " " + temp[i]->excerpt() + "\r\n\r\n";
                           delete temp[i];
                           }
                        delete temp;
                        temp = NULL;
                        index += retdoccount;
                    }while (iHits > index);
                    }
                }
            else resultString += " Requested document is out of range";
            resultsBox->setPlainText( resultString );
            }
        }
    }
