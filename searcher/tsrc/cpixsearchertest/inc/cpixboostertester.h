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
#ifndef CPIXBOOSTTESTER_H_
#define CPIXBOOSTTESTER_H_

#include "RSearchServerSession.h"

class CCPixIndexer;
class CCPixSearcher;

/* 

  Encapsulates all the tests. 
  
    Any member function starting 'test' is interpreted as a test to be executed.
    
      The tests will be executed in the order they are in this file.
      
        Note that the code for the individual tests may be in different CPP files.
        
*/
class CTestBoost : public CBase
{   
public:
    CTestBoost(){}

public:
    
    void setUp();
    void tearDown();
    
public:

    void testNoBoost();

    void testBoost();

    void testFieldBoost();

private: // supporting functionality
    
    void AddDocumentL( TInt aId, const TDesC& aContent, TReal aBoost ); 
    void AddDocumentWithFieldBoostL( TInt aId, const TDesC& aContent, TReal aBoost ); 
    void AssertHitDocumentL( TInt aIndex, TInt aId ); 
    
private:
    // CPix database 
    CCPixIndexer* iIndexer;
    CCPixSearcher* iSearcher;
    RSearchServerSession iSession;  
};
#endif /* CPIXBOOSTTESTER_H_ */
