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

#ifndef SEARCHSERVERCONFIGURATION_H_
#define SEARCHSERVERCONFIGURATION_H_

/**
 * NOTE: This file MUST contain preprocessor commands only 
 * as it is included from the MMP 
 */

/**
 * Not just gcce or armcc release build, but actual release version. 
 * Release builds are used also to gather performance information, 
 * which should be disabled in actual releases.
 */
//#define RELEASE

#if defined(RELEASE) && defined(_DEBUG)
#warning "Release version is built with debug flag on!"
#endif


/**
 * Heartbeat time definitions.
 *
 * Maximum idle time: index database handles being idle longer than
 * this should be flushed / closed transparently - next time their
 * clients use them they are brought back.
 *
 * Heartbeat period time: this frequently kick the CPix/OpenC layer to
 * check on idle index database handles and/or perform other
 * houskeeping.
 *
 * NOTE: these time definitions are different (more) frequent for the
 * debug builds, for testability.
 */
#if defined(_DEBUG)
#define IDXDB_MAXIDLE_SEC  60           // 1 minute
#define HEARTBEAT_PERIOD_USEC  30000000 // 0.5 minute
#else
#define IDXDB_MAXIDLE_SEC  60           // 1 minute
#define HEARTBEAT_PERIOD_USEC  30000000 // 0.5 minute
#endif


/**
 * Thread priority delta for qry slave thread within CPix engine. The
 * slave thread performing the actual queries will have its priority
 * boosted with this amount. Can be negative too, but you want a
 * positive number here.
 */
#define QRY_THREAD_PRIORITY_DELTA 10


#if defined(_DEBUG)
// In debug build, we can control the OpenC layer CPix debug logs
// generation - comment out if not needed
#define CPIX_OPENC_LOGDBG
#endif

/**
 * Currently additional testing utility can be compiled and attached
 * into the server. This testing utility provides mechanisms for...
 * 
 * 1. Starting performance recording
 * 2. Stopping performance recording
 * 3. Dumping performance records into CSV files. 
 *    * Currently dumps only heap peak and average usage
 * 4. Shutting down the server
 * 
 * At this moment, recording RAM and execution times of individual 
 * executions is not used for reports and therefore not supported. 
 * Overall, the executions times should be reported from client side, 
 * as it is place where the times really matter. 
 */

#if defined(_DEBUG)
#define PROVIDE_TESTING_UTILITY
#endif

#ifdef RECORD_PERFORMANCE
#define RECORD_MEMORY_USAGE
#define RECORD_EXECUTION_TIMES
#endif

#ifdef RECORD_MEMORY_USAGE
//#define LOG_PERIODIC_MEMORY_USAGE	
//#define LOG_EXECUTION_MEMORY_USAGE
#endif

#ifdef RECORD_EXECUTION_TIMES 
//#define LOG_EXECUTION_TIMES 
#endif	

#if defined(RECORD_HEAP_USAGE) && defined(RELEASE) 
#warning "Memory consumption should not be recorder in actual release!"
#endif

#if defined(RECORD_HEAP_USAGE) && defined(_DEBUG) 
#warning "Memory records are unreliable in debug builds"
#endif

#endif /*SEARCHSERVERCONFIGURATION_H_*/
