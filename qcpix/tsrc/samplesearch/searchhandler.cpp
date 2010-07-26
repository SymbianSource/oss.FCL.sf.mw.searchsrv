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
#include <cpixsearcher.h>
#include <cpixdocument.h>

CSearchHandler::CSearchHandler()
    {  
    iSearchInterface = CpixSearcher::newInstance("root","_aggregate");
    iSearchInterface->connect(iSearchInterface, SIGNAL(handleSearchResults(int,int)), this, SLOT(__handleSearchResult(int,int)));
    iSearchInterface->connect(iSearchInterface, SIGNAL(handleDocument(int,CpixDocument*)), this, SLOT(__getDocumentAsync(int,CpixDocument*)));
    }

void CSearchHandler::__handleSearchResult(int aError, int estimatedResultCount)
    {
    qDebug() << aError << estimatedResultCount;
    iSearchresultCount= estimatedResultCount;
    emit handleAsyncSearchResult(aError, estimatedResultCount);
    }

void CSearchHandler::__getDocumentAsync(int aError, CpixDocument* aDocument )
    {
    emit handleDocument( aError, aDocument );
    }

CpixDocument* CSearchHandler::getDocumentAtIndex(int aIndex)
    {
    return iSearchInterface->document( aIndex );
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
    iSearchInterface->documentAsync( aIndex );
    }

void CSearchHandler::searchAsync(QString aSearchString)
    {
    iSearchInterface->searchAsync( aSearchString );
    }

void CSearchHandler::cancelLastSearch()
    {
    iSearchInterface->cancelSearch();
    }
