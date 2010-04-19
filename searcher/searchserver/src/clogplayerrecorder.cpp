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


#include <S32MEM.h>

#include "CLogPlayerRecorder.h"

TBool CLogPlayerRecorder::iLogOpen = EFalse;
RFs CLogPlayerRecorder::iRfs;
RFile CLogPlayerRecorder::iFile;
RArray<TUint> CLogPlayerRecorder::iHandleArray;
TInt64 CLogPlayerRecorder::iStartTimeStamp = KErrNotFound;

const TUint digitsIn64BitNumber = 20;

//_LIT(KCPixLogPlayerLogFolder, "CPix\\LogPlayer");
//_LIT(KCPixLogPlayerLogFile, "LogPlayer.txt");
_LIT8(KFlush, "FLUSH");
_LIT8(KSpace, " ");
_LIT8(KNewLine, "\n");
_LIT8( KDefineVolume, "DEFINEVOLUME" );
_LIT8( KHouseKeep, "HOUSEKEEP" );
_LIT8( KOpenDb, "OPENDB" );
_LIT8( KAdd, "ADD" );
_LIT8( KUpdate, "UPDATE" );
_LIT8( KDelete, "DELETE" );
_LIT8( KSearch, "SEARCH" );
_LIT8( KGetDocument, "GET" );
_LIT8( KReset, "RESET" );
_LIT8( KCancel, "CANCEL" );
_LIT8( KDefineVolumeSeparator, "/" );

void CLogPlayerRecorder::LogHouseKeepL()
    {
    OpenL();
    
    RBuf8 buffer;
    buffer.CleanupClosePushL();
    
    AppendTimeStampToMsgBuffer( buffer );
    AppendToMsgBuffer( buffer, KHouseKeep );
    AppendToMsgBuffer( buffer, KSpace ); // TODO: Fix. Ugly hack to make log execution work.
    AppendToMsgBuffer( buffer, KNewLine );
    iFile.Write( buffer );
    iFile.Flush();
    
    CleanupStack::PopAndDestroy( &buffer );
    }

void CLogPlayerRecorder::LogDefineVolumeL(const TDesC& aQualifiedBaseAppClass, 
        const TDesC& aIndexDbPath)
    {
    OpenL();
    
    RBuf8 buffer;
    buffer.CleanupClosePushL();
    AppendTimeStampToMsgBuffer( buffer );
    AppendToMsgBuffer( buffer, KDefineVolume );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, aQualifiedBaseAppClass );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, KDefineVolumeSeparator );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, aIndexDbPath );
    AppendToMsgBuffer( buffer, KNewLine );
    iFile.Write( buffer );
    iFile.Flush();
    
    CleanupStack::PopAndDestroy( &buffer );
    }

void CLogPlayerRecorder::LogFlushL(TUint aId)
    {
    OpenL();
    
    RBuf8 buffer;
    buffer.CleanupClosePushL();
    AppendTimeStampToMsgBuffer( buffer );
    AppendToMsgBuffer( buffer, KFlush );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, GetHandleId(aId) );
    AppendToMsgBuffer( buffer, KNewLine );
    iFile.Write( buffer );
    iFile.Flush();
    
    CleanupStack::PopAndDestroy(&buffer);
    }

void CLogPlayerRecorder::LogDatabaseOpenL(TUint aId, TBool aSearcher, const TDesC& aBaseAppClass)
    {
    OpenL();
 
    TPtrC typePtr = KNullDesC();
    if ( aSearcher )
        {
        typePtr.Set( _L( "SEARCHER" ) );
        }
    else
        {
        typePtr.Set( _L( "INDEXER" ) );
        }
    
    RBuf8 buffer;
    buffer.CleanupClosePushL();
    AppendTimeStampToMsgBuffer( buffer );
    AppendToMsgBuffer( buffer, KOpenDb );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, GetHandleId(aId) );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, typePtr );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, aBaseAppClass );
    AppendToMsgBuffer( buffer, KNewLine );
    iFile.Write( buffer );
    iFile.Flush();
    
    CleanupStack::PopAndDestroy( &buffer );
    }

void CLogPlayerRecorder::LogAddL( TUint aId, const CSearchDocument& aSearchDocument )
    {
    OpenL();
    
    HBufC8* data = SerializeDocumentL( aSearchDocument );
    CleanupStack::PushL( data );
    
    RBuf8 buffer;
    buffer.CleanupClosePushL();
    AppendTimeStampToMsgBuffer( buffer );
    AppendToMsgBuffer( buffer, KAdd );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, GetHandleId(aId) );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, data->Length() );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, *data );
    AppendToMsgBuffer( buffer, KNewLine );
    iFile.Write( buffer );
    iFile.Flush();
    
    CleanupStack::PopAndDestroy(&buffer);
    CleanupStack::PopAndDestroy(data);
    }

void CLogPlayerRecorder::LogUpdateL( TUint aId, const CSearchDocument& aSearchDocument )
    {
    OpenL();
    
    HBufC8* data = SerializeDocumentL( aSearchDocument );
    CleanupStack::PushL( data );
    
    RBuf8 buffer;
    buffer.CleanupClosePushL();
    AppendTimeStampToMsgBuffer( buffer );
    AppendToMsgBuffer( buffer, KUpdate );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, GetHandleId(aId) );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, data->Length() );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, *data );
    AppendToMsgBuffer( buffer, KNewLine );
    iFile.Write( buffer );
    iFile.Flush();
    
    CleanupStack::PopAndDestroy(&buffer);
    CleanupStack::PopAndDestroy(data);
    }

void CLogPlayerRecorder::LogDeleteL( TUint aId, const TDesC& aDocUid )
    {
    OpenL();
    
    RBuf8 buffer;
    buffer.CleanupClosePushL();
    AppendTimeStampToMsgBuffer( buffer );
    AppendToMsgBuffer( buffer, KDelete );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, GetHandleId(aId) );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, aDocUid );
    AppendToMsgBuffer( buffer, KNewLine );
    iFile.Write( buffer );
    iFile.Flush();
    
    CleanupStack::PopAndDestroy(&buffer);
    }

void CLogPlayerRecorder::LogSearchL( TUint aId, const TDesC& searchTerms )
    {
    OpenL();
    
    RBuf8 buffer;
    buffer.CleanupClosePushL();
    AppendTimeStampToMsgBuffer( buffer );
    AppendToMsgBuffer( buffer, KSearch );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, GetHandleId(aId) );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, searchTerms );
    AppendToMsgBuffer( buffer, KNewLine );
    iFile.Write( buffer );
    iFile.Flush();
    
    CleanupStack::PopAndDestroy(&buffer);
    }

void CLogPlayerRecorder::LogGetDocumentL( TUint aId, TUint aIndex )
    {
    OpenL();
    
    RBuf8 buffer;
    buffer.CleanupClosePushL();
    AppendTimeStampToMsgBuffer( buffer );
    AppendToMsgBuffer( buffer, KGetDocument );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, GetHandleId(aId) );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, aIndex );
    AppendToMsgBuffer( buffer, KNewLine );
    iFile.Write( buffer );
    iFile.Flush();
    
    CleanupStack::PopAndDestroy(&buffer);
    }

void CLogPlayerRecorder::LogResetL( TUint aId )
    {
    OpenL();
    
    RBuf8 buffer;
    buffer.CleanupClosePushL();
    AppendTimeStampToMsgBuffer( buffer );
    AppendToMsgBuffer( buffer, KReset );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, GetHandleId(aId) );
    AppendToMsgBuffer( buffer, KNewLine );
    iFile.Write( buffer );
    iFile.Flush();
    
    CleanupStack::PopAndDestroy(&buffer);
    }

void CLogPlayerRecorder::LogCancelL( TUint aId )
    {
    OpenL();
    
    RBuf8 buffer;
    buffer.CleanupClosePushL();
    AppendTimeStampToMsgBuffer( buffer );
    AppendToMsgBuffer( buffer, KCancel );
    AppendToMsgBuffer( buffer, KSpace );
    AppendToMsgBuffer( buffer, GetHandleId(aId) );
    AppendToMsgBuffer( buffer, KNewLine );
    iFile.Write( buffer );
    iFile.Flush();
    
    CleanupStack::PopAndDestroy(&buffer);
    }

void CLogPlayerRecorder::OpenL()
    {
    _LIT( KLogName, "C:\\logs\\CPix\\LogPlayer\\LogPlayer.txt");
    
    if ( !iLogOpen )
        {
        User::LeaveIfError( iRfs.Connect() );
        TInt err = iFile.Open( iRfs, KLogName(), EFileWrite | EFileStream );
        if ( err != KErrNone )
            {
            err = iFile.Create( iRfs, KLogName(), EFileWrite );
            User::LeaveIfError(err);
            }
        
        iLogOpen = ETrue;
        }
    }

HBufC8* CLogPlayerRecorder::SerializeDocumentL( const CSearchDocument& aSearchDocument )
    {
    HBufC8* serializedDocument = HBufC8::NewLC( aSearchDocument.Size() );
    TPtr8 serializedDocumentPtr = serializedDocument->Des();
    
    // Initialize a new stream
    RDesWriteStream stream;
    stream.Open(serializedDocumentPtr);
    stream.PushL();

    // Externalize hits to the stream
    aSearchDocument.ExternalizeL(stream);

    // Commit and destroy the stream
    stream.CommitL();
    CleanupStack::PopAndDestroy(&stream);   

    CleanupStack::Pop(serializedDocument);
    return serializedDocument;
    }


TUint CLogPlayerRecorder::GetHandleId( TUint aId )
    {
    TInt index = iHandleArray.Find( aId );
    if ( index == KErrNotFound )
        {
        iHandleArray.Append(aId);
        index = iHandleArray.Count() - 1;
        }
    
    return index;
    }

void CLogPlayerRecorder::AppendTimeStampToMsgBuffer( RBuf8& aBuffer )
    {
    TTime time;
    time.HomeTime();
    
    TInt64 timeDiff = 0;
    
    if  ( iStartTimeStamp == KErrNotFound )
        {
        iStartTimeStamp = time.Int64();
        }
    else
        {
        timeDiff = time.Int64() - iStartTimeStamp;
        }
    
    TBuf8<digitsIn64BitNumber> timeBuffer;
    timeBuffer.AppendNum( timeDiff );
    AppendToMsgBuffer( aBuffer, timeBuffer );
    AppendToMsgBuffer( aBuffer, KSpace );
    }

void CLogPlayerRecorder::AppendToMsgBuffer( RBuf8& aBuffer, const TDesC8& aData )
    {
    TUint requiredLength = aBuffer.Length() + aData.Length();
    if ( aBuffer.MaxLength() < requiredLength )
        {
        aBuffer.ReAlloc(requiredLength);
        }
    
    aBuffer.Append( aData );
    }

void CLogPlayerRecorder::AppendToMsgBuffer( RBuf8& aBuffer, const TDesC16& aData )
    {
    HBufC8* data = HBufC8::New( aData.Length() );
    if ( !data )
        {
        return;
        }
    
    data->Des().Copy( aData );
    
    AppendToMsgBuffer(aBuffer, *data);
    
    delete data;
    }

void CLogPlayerRecorder::AppendToMsgBuffer( RBuf8& aBuffer, TInt64 aData )
    {
    TUint requiredLength = aBuffer.Length() + digitsIn64BitNumber;
    if ( aBuffer.MaxLength() < requiredLength )
        {
        aBuffer.ReAlloc(requiredLength);
        }
    
    aBuffer.AppendNum( aData );
    }
