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

#ifndef COMMON_H_
#define COMMON_H_

// Enable performance logging
// #define ENABLE_PERFORMANCE_LOGGING

/** @addtogroup Common */
/*\@{*/

#include "indevicecfg.h"
#include <flogger.h>

/**
 * @file
 * @ingroup Common
 * @brief Provides exceptions and field name definitions
 */

/**
 * @brief Harvested item action type
 */ 
enum TCPixActionType
    {
    ECPixAddAction = 0,
    ECPixUpdateAction,
    ECPixRemoveAction,
//  ECPixMmcMount, // Not Used. 
//  ECPixMmcDismount, // Not Used. 
//  ECPixMmcReset // Not Used.
    };

/**
 * @anchor exceptions
 * Server leaves with with this error code, if initialization of a OpenC CPix library fails.
 */
const TInt KErrCPixInitializationFailed = -68000;

/**
 * Client request is completed with this leave code, if opening of a specified database fails.
 */
const TInt KErrCannotOpenDatabase = -68001;

/**
 * Client request is completed with this leave code, if Server cannot create analyzer. This is internal Server error, which may occur while opening of a specified database.
 */
const TInt KErrCannotCreateAnalyzer = -68002;

/**
 * Client request is completed with this leave code, if Server cannot create query parser. This is internal Server error, which may occur while opening of a specified database.
 */
const TInt KErrCannotCreateQueryParser = -68003;

/**
 * Client request is completed with this leave code, if parsing of a query sent by Client fails.
 */
const TInt KErrQueryParsingFailed = -68004;

/**
 * Client request is completed with this leave code, if execution of a database fails.
 */
const TInt KErrDatabaseQueryFailed = -68005;

/**
 * Client request is completed with this leave code, if accessing to the specific result document fails.
 */
const TInt KErrDocumentAccessFailed = -68006;

/**
 * Client or Harvester request is completed with this leave code, if addition of a document fails.
 */
const TInt KErrCannotAddDocument = -68007;

/**
 * Client or Harvester request is completed with this leave code, if update of a document fails.
 */
const TInt KErrCannotUpdateDocument = -68008;

/**
 * Request is completed with this leave code, if calling flush operation fails.
 */
const TInt KErrDatabaseFlushFailed = -68009;

/**
 * Request is completed with this leave code, if creation of a new document fails.
 */
const TInt KErrCannotCreateDocument = -68010;

/**
 * Request is completed with this leave code, if creation of a new field fails while adding a new document.
 */
const TInt KErrCannotCreateDocumentField = -68011;

/**
 * Request is completed with this leave code, if creation of a new term fails, when creating term list
 */
// const TInt KErrCannotCreateTerm = -68012; // TODO Not used - consider removing it!

/**
 * Volume cannot be defined.
 */
const TInt KErrCannotDefineVolume = -68013;

/**
 * Volume cannot be undefined.
 */
// const TInt KErrCannotUndefineVolume = -68014; // TODO Not used - consider removing it!

/**
 * Client or Harvester request is completed with this leave code, if addition of a document fails.
 */
const TInt KErrCannotDeleteDocument = -68015;

_LIT(KPerformanceLogFolder, "CPix\\Performance");
_LIT(KPerformanceLogFile, "Performance.txt");

/**
 * Helper class used by performance logging macros
 */
class TPerformanceLogger
    {
public: // Constructors and destructors
    
    /**
     * Constructor
     * @param aMethodName Method name used in logging messages
     */
    TPerformanceLogger(const char* aMethodName)
        {
        iMethodName.Set((const TUint8*)aMethodName);
        iStartTime.HomeTime();
        
        TRAP_IGNORE( CreatePerformanceMessageL(ETrue) );
        }

    /**
     * Destructor
     */
    ~TPerformanceLogger()
        {
        TRAP_IGNORE( CreatePerformanceMessageL(EFalse) );
        }
    
public: // New functions
    
    /**
     * Log message
     * @param aMessage Message to be logged
     */
    void LogMessage(const TDesC& aMessage)
        {
        RFileLogger::Write( KPerformanceLogFolder(),
                            KPerformanceLogFile(),
                            EFileLoggingModeAppend,
                            aMessage );
        }
    
private:
    
    /**
     * Log performance message
     * @param aMethodEntry ETrue if this is method entry, EFalse is method exit
     */
    void CreatePerformanceMessageL(TBool aMethodEntry)
        {
        TTime currentTime;
        currentTime.HomeTime();
        TBuf<255> msgBuf;
        //currentTime.FormatL( msgBuf, _L("%H:%T:%S:%C ") );
        currentTime.FormatL( msgBuf, _L("%C ") );
        
        HBufC* methodName = HBufC::NewLC( iMethodName.Length() );
        methodName->Des().Copy( iMethodName );
        
        if ( aMethodEntry )
            {
            msgBuf.Append(_L("Enter: "));
            }
        else
            {
            msgBuf.Append(_L("Exit: "));
            }
        
        msgBuf.Append( *methodName );
        CleanupStack::PopAndDestroy(methodName);

        if ( !aMethodEntry )
            {
            msgBuf.Append(_L(" Difference " ));
            msgBuf.AppendNum( currentTime.Int64() - iStartTime.Int64() );
            msgBuf.Append(_L(" microseconds"));
            }
        
        RFileLogger::Write( KPerformanceLogFolder(),
                            KPerformanceLogFile(),
                            EFileLoggingModeAppend,
                            msgBuf );
        }
    
private:
    
    TPtrC8 iMethodName;
    TTime iStartTime;
    };

#if defined(_DEBUG) && defined(ENABLE_PERFORMANCE_LOGGING)
#define PERFORMANCE_LOG_START(x)   TPerformanceLogger cpixPerformanceLogger(x)
#define PERFORMANCE_LOG_MESSAGE(x) cpixPerformanceLogger.LogMessage(x)
#else
#define PERFORMANCE_LOG_START(x)
#define PERFORMANCE_LOG_MESSAGE(x)
#endif

/*\@}*/ // group Common

#endif /*COMMON_H_*/

