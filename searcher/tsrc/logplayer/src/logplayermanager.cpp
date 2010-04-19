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


//  Include Files
#include <e32cons.h>
#include <S32MEM.H>
#include <BACLINE.H>
#include "LogPlayerManager.h"
#include "CCPixIndexer.h"
#include "CCPixSearcher.h"
#include "CSearchDocument.h"

// Contants
const TUint KLogPlayerMaxIdLength = 20;
const TUint KLogPlayerMaxTimeStampLength = 20;
const TChar KDefineVolumeSeparator = '/';

_LIT8( KIndexer, "INDEXER" );
_LIT8( KSearcher, "SEARCHER" );

_LIT(KLogPlayerStart, "Starting Log Player\n");
_LIT(KLogPlayerStop, "Stopping Log Player\n");

void ExecuteLogL( const THandleInformation& aInformation )
    {
    CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
    CleanupStack::PushL(scheduler);
    CActiveScheduler::Install(scheduler);

    CLogPlayerManager* manager = CLogPlayerManager::NewLC(aInformation.iLogFileName);

    manager->ExecuteLogL( 0,
            aInformation.iFilePosition,
            aInformation.iHandleId,
            aInformation.iStartTime );

    CleanupStack::PopAndDestroy( manager );

    CleanupStack::PopAndDestroy(scheduler);
    }

TInt ThreadFunction(TAny* aParam)
    {
    CTrapCleanup* cleanup = CTrapCleanup::New();

    THandleInformation handleInformation = *(reinterpret_cast<THandleInformation*>( aParam ) );

    TRAPD( err, ExecuteLogL( handleInformation ) );

    delete cleanup;
    return err;
}

// ========================= MEMBER FUNCTIONS ==================================

// -----------------------------------------------------------------------------
// CLogPlayerManager::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CLogPlayerManager* CLogPlayerManager::NewL( const TDesC& aLogFileName )
    {
    CLogPlayerManager* logPlayerManager = CLogPlayerManager::NewLC( aLogFileName );
    CleanupStack::Pop(logPlayerManager);
    return logPlayerManager;
    }

// -----------------------------------------------------------------------------
// CLogPlayerManager::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CLogPlayerManager* CLogPlayerManager::NewLC( const TDesC& aLogFileName )
    {
    CLogPlayerManager* logPlayerManager = new (ELeave) CLogPlayerManager( aLogFileName );
    CleanupStack::PushL(logPlayerManager);
    logPlayerManager->ConstructL();
    return logPlayerManager;
    }

// -----------------------------------------------------------------------------
// CLogPlayerManager::~CLogPlayerManager()
// Destructor.
// -----------------------------------------------------------------------------
//
CLogPlayerManager::~CLogPlayerManager()
    {
	// TODO: Clear iHandleInformationArray
	
    delete iLogPlayerTimer;
    
	delete iIndexer;
	delete iSearcher;
	iSession.Close();
	
	iReadStream.Close();
	iFile.Close();
	iFs.Close();
	
	delete iWait;
    }

void CLogPlayerManager::TimerCallback()
    {
	TRAP_IGNORE( ContinueExecutionL() );
    }

// -----------------------------------------------------------------------------
// CLogPlayerManager::ExecuteLog()
// -----------------------------------------------------------------------------
//
void CLogPlayerManager::ExecuteLogL(
		CConsoleBase* aConsole,
		TInt aPosition,
		TInt aHandle,
		const TTime& aStartTime )
{
	iHandle = aHandle;
	iStartTime = aStartTime;
	
	if ( aConsole )
	{
        iConsole = aConsole;
		aConsole->Printf( KLogPlayerStart );
	}

	// Open read stream for file
    iReadStream.Attach( iFile, aPosition );
    
    if ( iHandle == -1 )
    {
    	iSession.StopHouseKeeping();
    }

    TRAP_IGNORE( ReadTimeAndWait() );

    iWait->Start();

    if ( iHandle == -1 )
    {
    	iSession.ContinueHouseKeeping();
    }

    if ( aConsole )
		{
    	aConsole->Printf( KLogPlayerStop );
		}
}

TInt CLogPlayerManager::ReadNumberL(RReadStream& aReadStream,
        TChar aSeparator)
    {
    TBuf8<KLogPlayerMaxIdLength> id;
    aReadStream.ReadL(id, aSeparator);
    TLex8 lexer(id);
    TInt idVal;
    lexer.Val(idVal);
    return idVal;
    }

void CLogPlayerManager::ReadTextL(RReadStream& aReadStream, TDes8& aText,
        TChar aSeparator)
    {
    aReadStream.ReadL(aText, aSeparator);
    aText.Trim();
    }

void CLogPlayerManager::ReadTextL(RReadStream& aReadStream, TDes8& aText,
        TInt aLength)
    {
    aReadStream.ReadL(aText, aLength);
    }

TInt64 CLogPlayerManager::ReadTimeStampL(RReadStream& aReadStream)
    {
    TBuf8<KLogPlayerMaxTimeStampLength> timeStamp;
    aReadStream.ReadL(timeStamp, KLogPlayerFieldSeparator);
    TLex8 lexer(timeStamp);
    TInt64 timeVal;
    TInt err = lexer.Val(timeVal);
    if (err != KErrNone)
        {
        User::Leave(KErrNotSupported);
        }
    return timeVal;
    }

// -----------------------------------------------------------------------------
// CLogPlayerManager::ContinueExecutionL()
// -----------------------------------------------------------------------------
//
void CLogPlayerManager::ContinueExecutionL()
    {
    _LIT8( KOpenDb, "OPENDB" );
    _LIT8( KDefineVolume, "DEFINEVOLUME" );
    _LIT8( KReset, "RESET" );
    _LIT8( KFlush, "FLUSH" );
    _LIT8( KAdd, "ADD" );
    _LIT8( KUpdate, "UPDATE" );
    _LIT8( KDelete, "DELETE" );
    _LIT8( KSearch, "SEARCH" );
    _LIT8( KGet, "GET" );
    _LIT8( KCancel, "CANCEL" );
    _LIT8( KHouseKeep, "HOUSEKEEP" );


    // Note: Huge descriptor from stack. Re-use this if possible.
	// TODO: Remove and use member variable iTextBuffer
	TBuf8<KLogPlayerMaxTextLength> textBuffer;

	// Read command
	ReadTextL(iReadStream, textBuffer);

	// Command: OPENDB
	if (textBuffer.Compare(KOpenDb) == 0)
	{
        if ( iConsole )
            {
            iConsole->Printf(_L("OPENDB\n"));
            }
		TInt index = 0;
		TBool isSearcher = EFalse;
		HBufC* baseAppClass = ReadOpenDbL(iReadStream, index, isSearcher);
		CleanupStack::PushL( baseAppClass );

		if ( index == iHandle )
		{
			if ( isSearcher )
			{
				TRAP_IGNORE( iSearcher->OpenDatabaseL(*baseAppClass) );
			}
			else
			{
				TRAP_IGNORE( iIndexer->OpenDatabaseL(*baseAppClass) );
			}
		}
		CleanupStack::PopAndDestroy( baseAppClass );

		// Check if handler for this index is created
		if ( iHandle == -1 && !IsHandlerCreated( index ) )
		{
			THandleInformation handleInformation;
			handleInformation.iLogFileName = iLogFileName;
			handleInformation.iFilePosition = iStartPosition.Offset();
			handleInformation.iHandleId = index;
			handleInformation.iStartTime = iStartTime;

			iHandleInformationArray.Append( handleInformation );

			// Create new thread for this new handler
			// Thread name: "LogManager_" + handle id
			TBuf<KMaxFileName> threadName;
			_LIT( KThreadNameBase, "LogManager_" );
			threadName.Append( KThreadNameBase );
			threadName.AppendNum( index );

			// Create thread
			RThread thread;
			thread.Create( threadName,
					ThreadFunction,
					1024*10,
					0,
					&iHandleInformationArray[iHandleInformationArray.Count()-1] );
			thread.Resume();
		}
	}
	// Command: DEFINEVOLUME
	else if (textBuffer.Compare(KDefineVolume) == 0)
	{
        if ( iConsole )
            {
            iConsole->Printf(_L("DEFINEVOLUME\n"));
            }

		// Get QualifiedBaseAppClass
		ReadTextL(iReadStream, textBuffer, KDefineVolumeSeparator);
		HBufC* qualifiedBaseAppClass = HBufC::NewLC(textBuffer.Length());
		TUint textBufferLen = textBuffer.Length() - 1; // Remove separator at end
		qualifiedBaseAppClass->Des().Copy( textBuffer.Left( textBufferLen ) );
		qualifiedBaseAppClass->Des().Trim();

		// Get Index path
		ReadTextL(iReadStream, textBuffer, KLogPlayerFieldSeparatorEndLine);
		HBufC* indexDbPath = HBufC::NewLC(textBuffer.Length());
		indexDbPath->Des().Copy(textBuffer);

		if ( iHandle == -1 )
		{
			iSession.DefineVolume(*qualifiedBaseAppClass, *indexDbPath);
		}

		CleanupStack::PopAndDestroy(indexDbPath);
		CleanupStack::PopAndDestroy(qualifiedBaseAppClass);
	}
	// Command: RESET
	else if (textBuffer.Compare(KReset) == 0)
	{
        if ( iConsole )
            {
            iConsole->Printf(_L("RESET\n"));
            }

		TInt index = 0;
		ReadResetL(iReadStream, index);
		if ( index == iHandle )
		{
			TRAP_IGNORE( iIndexer->ResetL() );
		}
	}
	// Command: FLUSH
	else if (textBuffer.Compare(KFlush) == 0)
	{
        if ( iConsole )
            {
            iConsole->Printf(_L("FLUSH\n"));
            }

		TInt index = 0;
		ReadFlushL(iReadStream, index);
		if ( index == iHandle )
		{
			TRAP_IGNORE( iIndexer->FlushL() );
		}
	}
	// Command: ADD
	else if (textBuffer.Compare(KAdd) == 0)
	{
        if ( iConsole )
            {
            iConsole->Printf(_L("ADD\n"));
            }

		TInt index = 0;
		CSearchDocument* doc = ReadAddL(iReadStream, index);
		CleanupStack::PushL(doc);

		if ( index == iHandle )
		{
			TRAP_IGNORE( iIndexer->AddL(*doc) );
		}

		CleanupStack::PopAndDestroy(doc);
	}
	// Command: UPDATE
	else if (textBuffer.Compare(KUpdate) == 0)
	{
        if ( iConsole )
            {
            iConsole->Printf(_L("UPDATE\n"));
            }

		TInt index = 0;
		CSearchDocument* doc = ReadAddL(iReadStream, index);
		CleanupStack::PushL(doc);

		if ( index == iHandle )
		{
			TRAP_IGNORE( iIndexer->UpdateL(*doc) );
		}

		CleanupStack::PopAndDestroy(doc);
	}
	// Command: DELETE
	else if (textBuffer.Compare(KDelete) == 0)
	{
        if ( iConsole )
            {
            iConsole->Printf(_L("DELETE\n"));
            }

		TInt index = 0;
		HBufC* docUid = ReadDeleteL(iReadStream, index);
		CleanupStack::PushL(docUid);

		if ( index == iHandle )
		{
			TRAP_IGNORE( iIndexer->DeleteL(*docUid) );
		}

		CleanupStack::PopAndDestroy(docUid);
	}
	// Command: SEARCH
	else if (textBuffer.Compare(KSearch) == 0)
	{
        if ( iConsole )
            {
            iConsole->Printf(_L("SEARCH\n"));
            }

		TInt index = 0;
		HBufC* searchQuery = ReadSearchL(iReadStream, index);
		CleanupStack::PushL(searchQuery);

		if ( index == iHandle )
		{
			TRAP_IGNORE( iSearcher->SearchL(*this, *searchQuery) );
		}

		CleanupStack::PopAndDestroy(searchQuery);
	}
	// Command: GET
	else if (textBuffer.Compare(KGet) == 0)
	{
        if ( iConsole )
            {
            iConsole->Printf(_L("GET\n"));
            }

		TInt index = 0;
		TInt docIndex = ReadGetL(iReadStream, index);

		if ( index == iHandle )
		{
			TRAP_IGNORE( iSearcher->GetDocumentL(docIndex, *this) );
		}
	}
	// Command: CANCEL
	else if (textBuffer.Compare(KCancel) == 0)
	{
        if ( iConsole )
            {
            iConsole->Printf(_L("CANCEL\n"));
            }

		TInt index = 0;
		ReadCancelL(iReadStream, index);

		if ( index == iHandle )
		{
			iSearcher->Cancel();
		}
	}
	// Command: HOUSEKEEP
	else if (textBuffer.Compare(KHouseKeep) == 0)
	{
        if ( iConsole )
            {
            iConsole->Printf(_L("HOUSEKEEP\n"));
            }

		ReadHouseKeepingL(iReadStream);
		if ( iHandle == -1 )
		{
			iSession.ForceHouseKeeping();
		}
	}
	else
	{
		iWait->AsyncStop();
		return;
	}
	
	ReadTimeAndWait();
}

// -----------------------------------------------------------------------------
// CLogPlayerManager::ReadFlushL
// -----------------------------------------------------------------------------
//
void CLogPlayerManager::ReadFlushL(RFileReadStream& aReadStream, TInt& aIndex)
    {
    aIndex = ReadNumberL(aReadStream, KLogPlayerFieldSeparatorEndLine);
    }

// -----------------------------------------------------------------------------
// CLogPlayerManager::ReadAddL
// -----------------------------------------------------------------------------
//
CSearchDocument* CLogPlayerManager::ReadAddL(RFileReadStream& aReadStream, TInt& aIndex)
    {
    aIndex = ReadNumberL(aReadStream);

    // Data is in serialized binary format preceded by size in bytes
    TInt serializedSize = ReadNumberL(aReadStream);
    HBufC8* data = HBufC8::NewLC(serializedSize);
    TPtr8 dataPtr = data->Des();
    ReadTextL(aReadStream, dataPtr, serializedSize);

    RDesReadStream desReadStream(*data);
    CleanupClosePushL(desReadStream);
    CSearchDocument* searchDocument = CSearchDocument::NewLC(
            desReadStream);

    CleanupStack::Pop(searchDocument);
    CleanupStack::PopAndDestroy(&desReadStream);
    CleanupStack::PopAndDestroy(data);

    // Remove new line
    ReadTextL(aReadStream, iTextBuffer, KLogPlayerFieldSeparatorEndLine);
    
    return searchDocument;
    }

// -----------------------------------------------------------------------------
// CLogPlayerManager::ReadDeleteL
// -----------------------------------------------------------------------------
//
HBufC* CLogPlayerManager::ReadDeleteL(RFileReadStream& aReadStream, TInt& aIndex)
    {
    aIndex = ReadNumberL(aReadStream);

    ReadTextL(aReadStream, iTextBuffer, KLogPlayerFieldSeparatorEndLine);
    TBuf<KLogPlayerMaxTextLength> docUid16;
    docUid16.Copy(iTextBuffer);
    return docUid16.AllocL();
    }

// -----------------------------------------------------------------------------
// CLogPlayerManager::ReadSearchL
// -----------------------------------------------------------------------------
//
HBufC* CLogPlayerManager::ReadSearchL(RFileReadStream& aReadStream, TInt& aIndex)
    {
    aIndex = ReadNumberL(aReadStream);

    ReadTextL(aReadStream, iTextBuffer, KLogPlayerFieldSeparatorEndLine);
    TBuf<KLogPlayerMaxTextLength> searchQuery;
    searchQuery.Copy(iTextBuffer);
    return searchQuery.AllocL();
    }

// -----------------------------------------------------------------------------
// CLogPlayerManager::ReadGetL
// -----------------------------------------------------------------------------
//
TInt CLogPlayerManager::ReadGetL(RFileReadStream& aReadStream, TInt& aIndex)
    {
	aIndex = ReadNumberL(aReadStream);

	TInt docIndex = ReadNumberL(aReadStream,
            KLogPlayerFieldSeparatorEndLine);

	return docIndex;
    }

// -----------------------------------------------------------------------------
// CLogPlayerManager::ReadCancelL
// -----------------------------------------------------------------------------
//
void CLogPlayerManager::ReadCancelL(RFileReadStream& aReadStream, TInt& aIndex)
    {
	aIndex = ReadNumberL(aReadStream,
            KLogPlayerFieldSeparatorEndLine);
    }

// -----------------------------------------------------------------------------
// CLogPlayerManager::ReadHouseKeepingL
// -----------------------------------------------------------------------------
//
void CLogPlayerManager::ReadHouseKeepingL(RFileReadStream& aReadStream)
    {
    // Remove new line
    ReadTextL(aReadStream, iTextBuffer, KLogPlayerFieldSeparatorEndLine);
    }

// -----------------------------------------------------------------------------
// CLogPlayerManager::ReadOpenDbL
// -----------------------------------------------------------------------------
//
HBufC* CLogPlayerManager::ReadOpenDbL(RFileReadStream& aReadStream, TInt& aIndex, TBool& aIsSearcher)
    {
    aIndex = ReadNumberL(aReadStream);

    // Read SEARCHER / INDEXER
    ReadTextL(aReadStream, iTextBuffer);
    if (iTextBuffer.Compare(KIndexer) == 0)
        {
        aIsSearcher = EFalse;
        }
    else if (iTextBuffer.Compare(KSearcher) == 0)
        {
        aIsSearcher = ETrue;
        }
    else // Unknown type
        {
        User::Leave(KErrNotSupported);
        }

    // Read base app class
    ReadTextL(aReadStream, iTextBuffer, KLogPlayerFieldSeparatorEndLine);
    TBuf<KLogPlayerMaxTextLength> baseAppClass16;
    baseAppClass16.Copy(iTextBuffer);
    return baseAppClass16.AllocL();
    }

// -----------------------------------------------------------------------------
// CLogPlayerManager::ReadResetL
// -----------------------------------------------------------------------------
//
void CLogPlayerManager::ReadResetL(RFileReadStream& aReadStream, TInt& aIndex)
    {
    aIndex = ReadNumberL(aReadStream,
               KLogPlayerFieldSeparatorEndLine);
    }

// -----------------------------------------------------------------------------
// CLogPlayerManager::IsHandlerCreated
// -----------------------------------------------------------------------------
//
TBool CLogPlayerManager::IsHandlerCreated(TInt aIndex)
    {
	TBool ret = EFalse;
	
	for ( TInt i=0; i < iHandleInformationArray.Count(); i++ )
	{
		if ( iHandleInformationArray[i].iHandleId == aIndex )
		{
			ret = ETrue;
			break;
		}
	}
    
	return ret;
    }

// -----------------------------------------------------------------------------
// CLogPlayerManager::ReadTimeAndWait
// -----------------------------------------------------------------------------
//
void CLogPlayerManager::ReadTimeAndWait()
    {
    // If command in current line is OPENDB with new handle id then new thread must
    // start execution at the start of current line. This is used in ContinueExecutionL
    TRAP_IGNORE( iStartPosition = iReadStream.Source()->TellL(MStreamBuf::ERead) );
    
    TInt64 timeStamp = 0;

    TRAPD( err, timeStamp = ReadTimeStampL(iReadStream) );
    if (err != KErrNone)
        {
        // EOF
        iWait->AsyncStop();
        return;
        }

    // First run of master thread
    if ( iStartTime.Int64() == 0 )
        {
        iStartTime.HomeTime();
        }

    iLogPlayerTimer->StartTimer( iStartTime + TTimeIntervalMicroSeconds(timeStamp) );
    }

// -----------------------------------------------------------------------------
// CLogPlayerManager::CLogPlayerManager()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CLogPlayerManager::CLogPlayerManager( const TDesC& aLogFileName )
	: iStartTime(0), iLogFileName(aLogFileName)
    {
    }

// -----------------------------------------------------------------------------
// CLogPlayerManager::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CLogPlayerManager::ConstructL()
    {
    iWait = new(ELeave) CActiveSchedulerWait();

	// Open session to Search Server
	User::LeaveIfError(iSession.Connect());
    
	// Open indexer and searcher
    iIndexer = CCPixIndexer::NewL(iSession);
    iSearcher = CCPixSearcher::NewL(iSession);

    // Open file for reading Log Player data
    User::LeaveIfError( iFs.Connect() );

    TInt err = iFile.Open( iFs, iLogFileName, EFileStream | EFileRead | EFileShareReadersOnly );
    User::LeaveIfError( err );
 
	iLogPlayerTimer = CLogPlayerTimer::NewL(this);
    }
