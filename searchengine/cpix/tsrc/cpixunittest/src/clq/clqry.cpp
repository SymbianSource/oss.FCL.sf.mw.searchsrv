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


// from file uxqry.cpp
Itk::TesterBase * CreateUxQryTests();


/*****************************************************************
 *
 * These are the further test suites that test some kind of clucene
 * qry type that needs to be supported.
 *
 */
Itk::TesterBase * CreateClQryHierarchy() 
{
    using namespace Itk; 

    SuiteTester
        * clQry = new SuiteTester("clq");
    
    clQry->add(CreateUxQryTests());

    // TODO ADD MORE

    return clQry;
}
