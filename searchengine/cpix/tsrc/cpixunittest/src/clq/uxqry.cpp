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

#include <wchar.h>
#include <stddef.h>

#include <iostream>
#include <algorithm>

#include "cpixfstools.h"

#include "itk.h"

#include "cpixidxdb.h"

#include "config.h"
#include "suggestion.h"
#include "testutils.h"
#include "testcorpus.h"
#include "setupsentry.h"


/*****************************************************************
 *
 * These tests are for the kinds of searches that a client (UX) is
 * likely to submit.
 */


Itk::TesterBase * CreateUxQryTests()
{
    using namespace Itk;

    /* TODO 
    UxQryContext
        * context = new UxQryContext;

    ContextTester
        * contextTester = new ContextTester("ux",
                                            context);

#define TEST "foobar"
    contextTester->add(TEST,
                       context,
                       &UxQryContext::testFooBar,
                       TEST);
#undef TEST

    // TODO ADD MORE TESTS ...

    return context;
    */

    return NULL;
}

