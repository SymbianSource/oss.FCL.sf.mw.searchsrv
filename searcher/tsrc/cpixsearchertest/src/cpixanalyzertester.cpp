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

#include "cpixanalyzertester.h"
#include "ccpixsearcher.h"
#include "CSearchDocument.h"
#include "CCPixIndexer.h"
#include "indevicecfg.h"
#include <f32file.h>

_LIT( KStemCorpusBaseAppClass, "test stem" );
_LIT( KStemCorpusQualifiedBaseAppClass, "@c:test stem" );

_LIT( KStemEnCorpusDirectory, "c:\\data\\cpixS60unittest\\stem\\en" ); 
_LIT( KStemCorpusDatabase, "c:\\data\\indexing\\test\\stem\\" );

void IndexFileL( CCPixIndexer& aIndexer, const TDesC& aFile ) 
    {
    aIndexer.AddL(
        *CSearchDocument::NewLC(aFile, KNullDesC, KNullDesC, CSearchDocument::EFileParser));
    CleanupStack::PopAndDestroy();
    aIndexer.FlushL();
    }

void IndexDirectoryL( CCPixIndexer& aIndexer, const TDesC& aDirectory ) 
    {
    TBuf<256> buf; 
    buf.Append( aDirectory ); 
    buf.Append( _L( "\\*" ) ); 
    
    RFs fs;
    fs.Connect();
    CleanupClosePushL(fs);  
    
    CDir* dir;
    User::LeaveIfError(fs.GetDir(buf,
                                 KEntryAttDir|KEntryAttNormal,
                                 ESortByName,
                                 dir));
    CleanupStack::PushL( dir ); 

    for (TInt i=0; i < dir->Count(); i++) 
        {
        const TEntry& aEntry = (*dir)[i];
        // Check if entry is a hidden or system file
        // if true -> do nothing
        if( !aEntry.IsHidden() && !aEntry.IsSystem() && !aEntry.IsDir() )
            {
            TBuf<256> fullName; 
            fullName.Append( aDirectory ); 
            fullName.Append( _L( "\\") ); 
            fullName.Append( aEntry.iName ); 
            IndexFileL( aIndexer, fullName ); 
            }
        }
    CleanupStack::PopAndDestroy( dir ); 
    CleanupStack::PopAndDestroy(); // fs 
    }

void CTestAnalyzer::setUp()
    {
    User::LeaveIfError(iSession.Connect());
    iSession.DefineVolume(KStemCorpusQualifiedBaseAppClass, KStemCorpusDatabase );

    iIndexer = CCPixIndexer::NewL(iSession);
    iIndexer->OpenDatabaseL(KStemCorpusQualifiedBaseAppClass);

    iSearcher = CCPixSearcher::NewL(iSession, _L( "" CONTENTS_FIELD ) );
    iSearcher->OpenDatabaseL(KStemCorpusBaseAppClass);
    }

void CTestAnalyzer::tearDown()
    {
    delete iIndexer;
    delete iSearcher;
    iSession.Close();
    }

void CTestAnalyzer::PrepareIndexL()
    {
    iIndexer->ResetL(); // destroy all
    IndexDirectoryL( *iIndexer, KStemEnCorpusDirectory );
    }

void CTestAnalyzer::SetAnalyzerL( const TDesC& aAnalyzer )
    {
    iIndexer->SetAnalyzerL( aAnalyzer ); 
    iSearcher->SetAnalyzerL( aAnalyzer ); 
    }

void CTestAnalyzer::AssertHitCountL( const TDesC& aWord, TInt aCount )
    {
    TInt err = KErrNone;
    if( iSearcher->SearchL( aWord )!= aCount )
        err = KErrGeneral;
    User::LeaveIfError( err );
    }

TBool CTestAnalyzer::HitL( const TDesC& aWord, const TDesC& aFileId ) 
    {
    TBuf<256> file; 
    file.Append( KStemEnCorpusDirectory ); 
    file.Append( _L( "\\") ); 
    file.Append( aFileId ); 
    file.Append( _L( ".txt") ); 
    
    TInt n = iSearcher->SearchL( aWord );
    
    for (int i = 0; i < n; i++) 
        {
        CSearchDocument* doc = iSearcher->GetDocumentL( i ); 
        CleanupStack::PushL( doc ); 
        if ( doc->Id() == file ) 
            {
            CleanupStack::PopAndDestroy( doc ); 
            return ETrue; 
            }
        CleanupStack::PopAndDestroy( doc ); 
        }
    return EFalse; 
    }

void CTestAnalyzer::AssertHitL( const TDesC& aWord, const TDesC& aFileId )
    {
    if ( !HitL( aWord, aFileId ) ) {
        User::Leave(KErrNotFound);
        }
    }

void CTestAnalyzer::AssertNoHitL( const TDesC& aWord, const TDesC& aFileId )
    {
    if ( HitL( aWord, aFileId ) ) 
        {
        User::Leave(KErrGeneral);
        }
    }

void CTestAnalyzer::testStandardAnalyzer()
    {
    SetAnalyzerL( _L( "" CPIX_ANALYZER_STANDARD ) ); 
    
    PrepareIndexL(); 

    // Test lack of stemming
    AssertHitCountL( _L( "happy" ), 1 );
    AssertHitL  ( _L( "happy" ), _L( "1") );
    AssertNoHitL( _L( "happy" ), _L( "2" ) );
    AssertNoHitL( _L( "happy" ), _L( "3") );
    AssertNoHitL( _L( "happy" ), _L( "4") );

    // More lack of stemming related
    AssertNoHitL( _L( "happiness" ), _L( "1") );
    AssertHitL  ( _L( "happiness" ), _L( "2") );

    // Test case insensitiveness
    AssertHitL( _L( "Happy" ),    _L( "1") );
    AssertHitL( _L( "what" ),     _L( "4") );
    }

void CTestAnalyzer::RemovetestSnowballAnalyzer()
    {
    SetAnalyzerL( _L( "" 
                      CPIX_TOKENIZER_STANDARD
                      CPIX_PIPE
                      CPIX_FILTER_LOWERCASE
                      CPIX_PIPE
                      CPIX_FILTER_STOP L"(en)"
                      CPIX_PIPE
                      CPIX_FILTER_STEM L"(en)" )
                 ); 

    PrepareIndexL(); 
    
    // Test stemming
    AssertHitCountL( _L( "happy" ), 2 );
    AssertHitL(   _L( "happy" ), _L( "1" ) );
    AssertHitL(   _L( "happy" ), _L( "2" ) );
    AssertNoHitL( _L( "happy" ), _L( "3" ) );
    AssertNoHitL( _L( "happy" ), _L( "4" ) );
    }

void CTestAnalyzer::testWhitespaceTokenizer()
    {
    SetAnalyzerL( _L( "" CPIX_TOKENIZER_WHITESPACE ) ); 

    PrepareIndexL(); 
    
    // Assert case sensitivess and tokenization
    AssertHitL(   _L( "happy." ),       _L("1") ); // correct
    AssertNoHitL( _L( "Happy." ),       _L("1") ); // wrong case
    AssertNoHitL( _L( "happy" ),        _L("1") ); // wrong tokenization
    AssertNoHitL( _L( "Happy" ),        _L("1") ); // wrong case and tokenization
    AssertHitL(   _L( "What" ),         _L("4") ); // correct
    AssertNoHitL( _L( "what" ),         _L("4") ); // wrong case 
    AssertHitL(   _L( "\"happiness!\"" ),_L("2") ); // correct
    AssertNoHitL( _L( "happiness" ),    _L("2") ); // wrong tokenization
    }
