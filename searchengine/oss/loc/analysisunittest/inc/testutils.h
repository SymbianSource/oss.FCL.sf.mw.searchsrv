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

#ifndef TESTUTILS_H_
#define TESTUTILS_H_

#include "CLucene.h"

bool readLine(lucene::util::Reader& reader, wchar_t* buf, int buflen); 

void printTokens(lucene::analysis::Analyzer& analyzer, const wchar_t* text); 

#endif /* TESTUTILS_H_ */
