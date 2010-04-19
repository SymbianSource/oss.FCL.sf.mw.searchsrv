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

#include "searchhandler.h"
#include <qcpixsearcher.h>
#include <qcpixdocument.h>

CSearchHandler::CSearchHandler()
    {  
    iSearchInterface = QCPixSearcher::newInstance("root","_aggregate");
    iSearchInterface->connect(iSearchInterface, SIGNAL(handleSearchResults(int,int)), this, SLOT(__handleSearchResult(int,int)));
    iSearchInterface->connect(iSearchInterface, SIGNAL(handleDocument(int,QCPixDocument*)), this, SLOT(__getDocumentAsync(int,QCPixDocument*)));
    }

void CSearchHandler::__handleSearchResult(int aError, int estimatedResultCount)
    {
    qDebug() << aError << estimatedResultCount;
    iSearchresultCount= estimatedResultCount;
    emit handleAsyncSearchResult(aError, estimatedResultCount);
    }

void CSearchHandler::__getDocumentAsync(int aError, QCPixDocument* aDocument )
    {
    emit handleDocument( aError, aDocument );
    }

QCPixDocument* CSearchHandler::getDocumentAtIndex(int aIndex)
    {
    return iSearchInterface->getDocument( aIndex );
    }

void CSearchHandler::search(QString aSearchString)
    {
    qDebug() << "CSearchHandler::search Enter";
    iSearchresultCount = iSearchInterface->search( aSearchString );
    emit handleSearchResult( KErrNone, iSearchresultCount );
    qDebug() << "CSearchHandler::search Exit";
    }

CSearchHandler::~CSearchHandler()
    {
    delete iSearchInterface;
    }

int CSearchHandler::getSearchResultCount()
    {
    return iSearchresultCount;
    }

void CSearchHandler::getDocumentAsyncAtIndex( int aIndex )
    {
    iSearchInterface->getDocumentAsync( aIndex );
    }

void CSearchHandler::searchAsync(QString aSearchString)
    {
    iSearchInterface->searchAsync( aSearchString );
    }

void CSearchHandler::cancelLastSearch()
    {
    iSearchInterface->cancelSearch();
    }
