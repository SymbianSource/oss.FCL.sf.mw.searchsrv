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

#ifndef CPIXANALYZERTESTER_H_
#define CPIXANALYZERTESTER_H_

#include "RSearchServerSession.h"

class CCPixIndexer;
class CCPixSearcher;

/* 

  Encapsulates all the tests. 
  
    Any member function starting 'test' is interpreted as a test to be executed.
    
      The tests will be executed in the order they are in this file.
      
        Note that the code for the individual tests may be in different CPP files.
        
*/
class CTestAnalyzer : public CBase
{   
public:
    CTestAnalyzer(){}

public:
    void setUp();
    void tearDown();
public:
    void testStandardAnalyzer();
    //Remove from test Suite
    void RemovetestSnowballAnalyzer();
    void testWhitespaceTokenizer();
    
private: // supporting functionality
    void PrepareIndexL(); 
    void SetAnalyzerL( const TDesC& aAnalyzer );
    
    void AssertHitCountL( const TDesC& aWord, TInt aCount ); 
    TBool HitL( const TDesC& aWord, const TDesC& aFile ); 
    void AssertHitL( const TDesC& aWord, const TDesC& aFile ); 
    void AssertNoHitL( const TDesC& aWord, const TDesC& aFile ); 

private:
    // CPix database 
    CCPixIndexer* iIndexer;
    CCPixSearcher* iSearcher;
    RSearchServerSession iSession;  
};

#endif /* CPIXANALYZERTESTER_H_ */
