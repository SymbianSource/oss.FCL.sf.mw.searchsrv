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


#ifndef CLOGPLAYERRECORDER_H
#define CLOGPLAYERRECORDER_H

// INCLUDE FILES
#include <e32base.h>
#include <f32file.h>
#include <flogger.h>

#include "CSearchDocument.h"
#include "indevicecfg.h"

#ifdef RECORD_FOR_LOG_PLAYER
#define LOG_PLAYER_RECORD(x)  x
#else
#define LOG_PLAYER_RECORD(x)
#endif

// CLASS DECLARATION
/**
 *  CSearchServer
 */
class CLogPlayerRecorder : public CBase
    {
public:
    // Constructors and destructors

    // None

public:
    // New functions

    /**
     * Log housekeeping
     */
    static void LogHouseKeepL();

    /**
     * Log define volume
     */
    static void LogDefineVolumeL(const TDesC& aQualifiedBaseAppClass,
            const TDesC& aIndexDbPath);
    
    /**
     * Log flush
     */
    static void LogFlushL(TUint aId);
    
    /**
     * Log database opening 
     */
    static void LogDatabaseOpenL(TUint aId, TBool aSearcher, const TDesC& aBaseAppClass);
    
    /**
     * Log document add
     */
    static void LogAddL( TUint aId, const CSearchDocument& aSearchDocument );

    /**
     * Log document update
     */
    static void LogUpdateL( TUint aId, const CSearchDocument& aSearchDocument );

    /**
     * Log document delete
     */
    static void LogDeleteL( TUint aId, const TDesC& aDocUid );

    /**
     * Log document search
     */
    static void LogSearchL( TUint aId, const TDesC& aDocUid );

    /**
     * Log document getting
     */
    static void LogGetDocumentL( TUint aId, TUint aIndex );
    
    /**
     * Log reset
     */
    static void LogResetL( TUint aId );
        
    /**
     * Log Cancel
     */
    static void LogCancelL( TUint aId );
        

private:
    // New functions

    /**
     * Open RFileLogger connection
     */
    static void OpenL();

    /**
     * Serialize search document
     */
    static HBufC8* SerializeDocumentL( const CSearchDocument& aSearchDocument );
    
    
    /**
     * Get handle id
     * @param unique identifier for subsession i.e. memory-address
     * @return Log player usable id
     */
    static TUint GetHandleId( TUint aId );

    /**
     * Append time stamp to buffer
     * First call creates time stamp with value 0 and following calls 
     * microseconds relative to first call
     * @param aBuffer Buffer where time stamp is appended. 
     */
    static void AppendTimeStampToMsgBuffer( RBuf8& aBuffer );
    
    /**
     * Append data to buffer
     * @param aBuffer Buffer where data is appended
     * @return aData Data to be appended
     */
    static void AppendToMsgBuffer( RBuf8& aBuffer, const TDesC8& aData );

    /**
     * Append data to buffer
     * @param aBuffer Buffer where data is appended
     * @return aData Data to be appended
     */
    static void AppendToMsgBuffer( RBuf8& aBuffer, const TDesC16& aData );

    /**
     * Helper method for AppendToMsgBuffer
     * @param aBuffer Buffer where data is appended
     * @return aData Data to be appended
     */
    static void DoAppendToMsgBuffer( RBuf8& aBuffer, const TDesC16& aData );

    /**
     * Append integer to buffer
     * @param aBuffer Buffer where interger is appended
     * @return aData Integer to be appended
     */
    static void AppendToMsgBuffer( RBuf8& aBuffer, TInt64 aData );

    
private:
    // Data

    // Is log file already opened
    static TBool iLogOpen;
    
    // File server serssion
    static RFs iRfs;
    
    // File handling log writing
    static RFile iFile;
    
    // Subsession array. Used to map log player usable id values
    static RArray<TUint> iHandleArray;
    
    // First call time (in microseconds) of AppendTimeStampToMsgBuffer
    static TInt64 iStartTimeStamp;
    };

#endif // CLOGPLAYERRECORDER_H
// End of File
