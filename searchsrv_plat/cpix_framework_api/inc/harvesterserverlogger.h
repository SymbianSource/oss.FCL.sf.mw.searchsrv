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
* Description:  Logger headerfile for CPixSearchServer
*
*/

#ifndef HARVESTERSERVERLOGGER_H
#define HARVESTERSERVERLOGGER_H

/**
 * @file
 * @ingroup Common
 * @brief Contains Logging Macros for debug purpose
 */
 
/**
*
* LOG SELECTION
*
*/
#ifndef _DEBUG
    // UREL BUILD:
    // Set logmode only if not yet set in cpp-file.
    #ifndef CPIX_LOGGING_METHOD
    #define CPIX_LOGGING_METHOD  0   // No logging in UREL builds
    #endif
#else
    // UDEB BUILD:
    #ifndef CPIX_LOGGING_METHOD
    #define CPIX_LOGGING_METHOD  1   // 0 = No logging,
    #endif                          // 1 = Flogger,
                                    // 2 = RDebug
                                    // 3 = Same as 1, but tick added
#endif //_DEBUG


#if CPIX_LOGGING_METHOD > 0
    #define CPIX_LOGGING_ENABLED    
#endif //CPIX_LOGGING_METHOD


/**
*
* FLOGGER SETTINGS
*
*/
#if CPIX_LOGGING_METHOD == 1 || CPIX_LOGGING_METHOD == 3     // Flogger

    #include <flogger.h>

    _LIT(KCPixLogFolder, "CPix\\HarvesterServer");
    _LIT(KCPixLogFile, "HarvesterServer.txt");

#elif CPIX_LOGGING_METHOD == 2    // RDebug

    #include <e32svr.h>

#endif //CPIX_LOGGING_METHOD


/**
*
* LOGGING MACROs
*
* USE THESE MACROS IN YOUR CODE !
*
*/

#if CPIX_LOGGING_METHOD == 1      // Flogger
#define CPIXLOGTEXT(AAA) /*lint -save -e960 */              { RFileLogger::Write(KCPixLogFolder(),KCPixLogFile(),EFileLoggingModeAppend,AAA); } /*lint -restore */
#define CPIXLOGSTRING(AAA) /*lint -save -e960 */            { _LIT(tempLogDes,AAA); RFileLogger::Write(KCPixLogFolder(),KCPixLogFile(),EFileLoggingModeAppend,tempLogDes()); } /*lint -restore */
#define CPIXLOGSTRING2(AAA,BBB) /*lint -save -e960 -e437 */ { _LIT(tempLogDes,AAA); RFileLogger::WriteFormat(KCPixLogFolder(),KCPixLogFile(),EFileLoggingModeAppend,TRefByValue<const TDesC>(tempLogDes()),BBB); } /*lint -restore */
#define CPIXLOGSTRING3(AAA,BBB,CCC) /*lint -save -e960 */   { _LIT(tempLogDes,AAA); RFileLogger::WriteFormat(KCPixLogFolder(),KCPixLogFile(),EFileLoggingModeAppend,TRefByValue<const TDesC>(tempLogDes()),BBB,CCC); } /*lint -restore */

#elif CPIX_LOGGING_METHOD == 2    // RDebug
#define CPIXLOGTEXT(AAA)              RDebug::Print(AAA);
#define CPIXLOGSTRING(AAA)            RDebug::Print(_L(AAA));
#define CPIXLOGSTRING2(AAA,BBB)       RDebug::Print(_L(AAA),BBB);
#define CPIXLOGSTRING3(AAA,BBB,CCC)   RDebug::Print(_L(AAA),BBB,CCC);

#elif CPIX_LOGGING_METHOD == 3    // Flogger + tick
#define CPIXLOGTEXT(AAA) /*lint -save -e960 */              { RFileLogger::Write(KCPixLogFolder(),KCPixLogFile(),EFileLoggingModeAppend,AAA); } /*lint -restore */
#define CPIXLOGSTRING(AAA) /*lint -save -e960 */            { _LIT8(tempLogDes, "%d " ##AAA); TUint tick=User::TickCount(); RFileLogger::WriteFormat(KCPixLogFolder(),KCPixLogFile(),EFileLoggingModeAppend,tempLogDes,tick); } /*lint -restore */
#define CPIXLOGSTRING2(AAA,BBB) /*lint -save -e960 -e437 */ { _LIT8(tempLogDes, "%d " ##AAA); TUint tick=User::TickCount(); RFileLogger::WriteFormat(KCPixLogFolder(),KCPixLogFile(),EFileLoggingModeAppend,tempLogDes,tick,BBB); } /*lint -restore */
#define CPIXLOGSTRING3(AAA,BBB,CCC) /*lint -save -e960 */   { _LIT8(tempLogDes, "%d " ##AAA); TUint tick=User::TickCount(); RFileLogger::WriteFormat(KCPixLogFolder(),KCPixLogFile(),EFileLoggingModeAppend,tempLogDes,tick,BBB,CCC); } /*lint -restore */

#else    // CPIX_LOGGING_METHOD == 0 or invalid

// Example: CPIXLOGTEXT(own_desc);
#define CPIXLOGTEXT(AAA)
    
// Example: CPIXLOGSTRING("Test");
#define CPIXLOGSTRING(AAA)
    
// Example: CPIXLOGSTRING2("Test %i", aValue);
#define CPIXLOGSTRING2(AAA,BBB)
    
// Example: CPIXLOGSTRING3("Test %i %i", aValue1, aValue2);
#define CPIXLOGSTRING3(AAA,BBB,CCC)

#endif  // CPIX_LOGGING_METHOD

#endif    // CPIXLOGGER_H

// End of File
