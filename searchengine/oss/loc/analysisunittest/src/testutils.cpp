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

#include "testutils.h"

#include <iostream>

using namespace lucene::util;

bool readLine(Reader& reader, wchar_t* buf, int buflen) 
{
	int i = 0; 
	while (i < buflen-1)
	{
		int c = reader.read();
		if (c == -1 && i == 0) return false; // EOF
		if (c == '\r' || c == '\n' || c == -1) break;
		buf[i++] = c; 
	}
	buf[i] = '\0';
	return true; 
}

void printTokens(lucene::analysis::Analyzer& analyzer, const wchar_t* text) 
{
	std::auto_ptr<lucene::analysis::TokenStream> stream( 
		analyzer.tokenStream( L"", new StringReader( text ) ) ); 
	
	lucene::analysis::Token token; 
	while (stream->next(&token)) {
		int pos = token.getPositionIncrement(); 
		if (pos == 0) {
			printf("|"); 
		} else {
			for (int i = 0; i < pos; i++) printf(" "); 
		}
		printf("'%S'", token.termText());
		fflush(stdout); 
	}
	printf("\n");
	
}
