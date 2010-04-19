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

#ifndef CPIX_LOG_H
#define CPIX_LOG_H

#include "cpixinit.h"

extern enum cpix_LogLevel GlobalLogLevel;


/**
 * Emits a log message on the std output. Time information as the
 * prefix, then the message defined by format and its printf like
 * parameters. No need to terminate the message string with newline
 * character, the newline character is added automatically anyway.
 */
void logMsg(cpix_LogLevel  logLevel,
            const char   * format,
            ...);


/**
 * Emits a debug log message on std output (using logMsg).
 */
void logDbgMsg(const char * format,
               ...);


/**
 * Very similary to logMsg, except this one does NOT produce a
 * date/time information prefix for each message. Useful for
 * generating test output for I/O captured unit tests.
 */
void logTestMsg(cpix_LogLevel  logLevel,
                const char   * format,
                ...);


#endif

