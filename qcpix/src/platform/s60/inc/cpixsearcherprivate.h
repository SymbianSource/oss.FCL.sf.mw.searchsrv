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

#ifndef _CPIXSEARCHER_PVTIMPL_H
#define _CPIXSEARCHER_PVTIMPL_H

#include <e32base.h>
#include <QObject>
#include <MCPixDatabaseObserver.h>
#include <MCPixSearcherObserver.h>
#include <RSearchServerSession.h>

//forward Declarations
class CCPixSearcher;
class CDocument;
class RSearchServerSession;
class CpixSearcher;

class CpixSearcherPrivate :  public CBase, //can this be made QObject?
							 public MCPixSearchRequestObserver ,
							 public MCPixNextDocumentRequestObserver,
							 public MCPixOpenDatabaseRequestObserver,
							 public MCPixSetAnalyzerRequestObserver
{
public:
    CpixSearcherPrivate(QObject* aParent);
    void Construct( QString aDefaultSearchField );
    ~CpixSearcherPrivate();

public :
    void HandleSearchResultsL(TInt aError, TInt aEstimatedResultCount); //from MCPixSearchRequestObserver
    void HandleDocumentL(TInt aError, CSearchDocument* aDocument); // from MCPixNextDocumentRequestObserver
	void HandleBatchDocumentL(TInt aError,TInt aRetCount, CSearchDocument** aDocument); // from MCPixNextDocumentRequestObserver
    void HandleOpenDatabaseResultL( TInt aError ); //MCPixOpenDatabaseRequestObserver
    void HandleSetAnalyzerResultL( TInt aError ); //MCPixSetAnalyzerRequestObserver

public: //Keep these to avoid wrapper functions.
    RSearchServerSession iSearchSession; //owned
    CCPixSearcher* iSearcher; //owned
    CpixSearcher* iSearchParent; //not owned.
};

#endif //_CPIXSEARCHER_PVTIMPL_H
