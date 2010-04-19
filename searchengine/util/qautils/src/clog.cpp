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
#include "CLog.h"

#include <s32file.h>
#include <e32cons.h>
#include <utf.h>

void MLog::LogL( TRefByValue<const TDesC> aFormat, ... ) 
	{
	VA_LIST args;
	VA_START( args, aFormat );  
	VLogL( aFormat, args ); 
	VA_END( args );  
	}

CFileConsoleLog::CFileConsoleLog( CConsoleBase* aConsole )
: 	iConsole( aConsole ), 
	iFile( NULL ),
	iFs( 0 )
	{
	}

void CFileConsoleLog::ConstructL( RFs& aFs, const TDesC& aFile ) 
	{
	aFs.MkDirAll( aFile );  // Prepare necessary paths
	aFs.Delete( aFile );  // Kill the old file
	iFile = new RFile(); 
	User::LeaveIfError( iFile->Create( aFs, aFile, EFileWrite|EFileStreamText ) != KErrNone );
	}

void CFileConsoleLog::ConstructL( const TDesC& aFile ) 
	{
	iFs = new RFs(); 
	User::LeaveIfError( iFs->Connect() ); 
	ConstructL( *iFs, aFile ); 
	}

CFileConsoleLog::~CFileConsoleLog()
	{
	if ( iFile ) 
		{
		iFile->Close(); 
		delete iFile; 
		}
	if ( iFs ) 
		{
		iFs->Close(); 
		delete iFs; 
		}
	}

CFileConsoleLog* CFileConsoleLog::NewL( CConsoleBase* aConsole,
										const TDesC& aFile )
	{
	CFileConsoleLog* self = new ( ELeave ) CFileConsoleLog( aConsole ); 
	CleanupStack::PushL( self ); 
	self->ConstructL( aFile ); 
	CleanupStack::Pop( self ); 
	return self;
	}

		
void CFileConsoleLog::VLogL( const TDesC& aFormat, VA_LIST aArgs )
	{
	HBufC* buf = HBufC::NewLC( 2048 ); 
	buf->Des().FormatList( aFormat, aArgs ); 
	buf->Des().Append( _L( "\n" ) ); 
	
	if ( iConsole ) 
		{
		iConsole->Write( *buf ); 
		}
    HBufC8* buf8 = CnvUtfConverter::ConvertFromUnicodeToUtf8L( *buf );
	iFile->Write( *buf8 );
	delete buf8;  
	CleanupStack::PopAndDestroy( buf ); 
	}

void CFileConsoleLog::FlushL()
	{
	if ( iConsole ) 
		{
		// nothing needs to be done? 
		}
	if ( iFile ) 
		{
		iFile->Flush();
		}
	}

TShortTimeStampLog::TShortTimeStampLog()
:	iLog( 0 ) {}

TShortTimeStampLog::TShortTimeStampLog( MLog& aLog )
:	iLog( &aLog ) 
	{
	}

void TShortTimeStampLog::SetLog( MLog& aLog ) 
	{
	iLog = &aLog;
	}

void TShortTimeStampLog::VLogL( const TDesC& aDesc, VA_LIST aArgs )
	{
	HBufC* buf = HBufC::NewLC( aDesc.Length() + 64 ); 
	TTime homeTime; 
	homeTime.HomeTime();
	TDateTime time = homeTime.DateTime();
	
	buf->Des().AppendNumFixedWidth( (TUint)time.Hour(), EDecimal, 2 );
	buf->Des().Append( _L(":") ); 
	buf->Des().AppendNumFixedWidth( (TUint)time.Minute(), EDecimal, 2 );
	buf->Des().Append( _L(" : ") );
	buf->Des().Append( aDesc ); 
	iLog->VLogL( *buf, aArgs ); 
	CleanupStack::PopAndDestroy( buf ); 
	}

void TShortTimeStampLog::FlushL()
	{
	iLog->FlushL();
	}

