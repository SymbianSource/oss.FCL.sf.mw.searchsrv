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
#ifndef ITK_ITK_H_
#define ITK_ITK_H_


#include <glib.h>

#include "itktesters.h"
#include "itktestmgr.h"
#include "itkobservers.h"


/**
 * Integration Test Kit
 */
namespace Itk
{
    
    class TestMgr;

    void itk_expect(TestMgr     * testMgr,
                    bool          succeeded,
                    const char  * expr,
                    const char  * file,
                    size_t        line,
                    const char  * format,
                    ...);


    void itk_assert(TestMgr     * testMgr,
                    bool          succeeded,
                    const char  * expr,
                    const char  * file,
                    size_t        line,
                    const char  * format,
                    ...);

    void itk_msg(TestMgr     * testMgr,
                 const char  * file,
                 size_t        line,
                 const char  * format,
                 ...);


    void itk_dbgMsg(TestMgr     * testMgr,
                    const char  * format,
                    ...);


    void itk_report(TestMgr     * testMgr,
                    const char  * name,
                    const char  * format,
                    ...);


    /**
     * Timestamp related utility functions
     */
    typedef GTimeVal Timestamp;

    void getTimestamp(Timestamp * timestamp);
    long getElapsedMs(Timestamp * later,
                      Timestamp * earlier);
    
}


#include "itk.hpp"


/**
 * Makes an expectation, and in case of failure, it is logged, but the
 * same test sequence is still continued. Message can be a printf-like
 * format with arguments following it.
 */
#define ITK_EXPECT(t,x,f,args...) Itk::itk_expect(t, x, #x, __FILE__, __LINE__, f, ##args)

/**
 * Makes an assertion, and in case of failure, it is logged and all of
 * the rest of the subsequent testcases in the innermost inclosing
 * test context are dropped. Message can be a printf-like format with
 * arguments following it.
 */
#define ITK_ASSERT(t,x,f,args...) Itk::itk_assert(t, x, #x, __FILE__, __LINE__, f, ##args)

/**
 * Dumps a message, printf-like formatting facility.
 */
#define ITK_MSG(t,f,args...) Itk::itk_msg(t, __FILE__, __LINE__, f, ##args)


/**
 * Dumps a message to the "debug" console (NEVER gets redirected!), in
 * some cases a developer may want to be able to always see a message
 * during running, like dots ('.') showing the progress of lengthy
 * test process.
 */
#define ITK_DBGMSG(t,f,args...) Itk::itk_dbgMsg(t, f, ##args)

/**
 * Panics the current setup() of a context or test case. The exact way
 * it behaves is most similar to ITK_ASSERT (it falls through the rest
 * of the test cases of the current test context), but this has the
 * semantics that something within the test logic (and not the tested
 * logic) went wrong. Also, the only way to signal error in a setup()
 * of an ITestContext. Printf-like formatting facility.
 */
#define ITK_PANIC(f, args...) throw Itk::PanicExc(__FILE__, __LINE__, f, ##args)


/**
 * To define any name/value pair (or short title / brief explanation)
 * report items that should be part of the generated summary. For
 * instance, performance measurements (like elapsed time) are best
 * reported through this macro. Printf-like formatting facility.
 */
#define ITK_REPORT(t,n,f,args...) Itk::itk_report(t, n, f, ##args)

#endif // ITK_ITK_H_
