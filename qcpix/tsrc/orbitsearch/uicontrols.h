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

#ifndef UICONTROLS_H_
#define UICONTROLS_H_

#include <hblineedit.h>
#include <hbpushbutton.h>
#include <hbtextedit.h>

//***********************************************************
//Fine controls for custom builds.

//Default search mode. Append "*" to the end of search string
//NOTE: These macros are mutually exclusive and precedence
//in the order below.
//i.e., STAR_SEARCH over-rides NO_STAR_SEARCH which overrides 
//ESCAPE_SPECIAL_CHARS.
#define STAR_SEARCH 0

//Vanilla search. Send search string to engine "as-is". 
#define NO_STAR_SEARCH 0

//Enable the prefix_search macro to enable Prefix query feature. This will add $prefix to the query 
//string.

#define PREFIX_SEARCH 1

//If you dont want to see results, define this flag.
#define DONT_SHOW_RESULTS 0

//************************************************************

#define ORBIT_SEARCH_CONTROLS   \
    HbLineEdit* searchBox;      \
    HbPushButton* searchButton; \
    HbTextEdit* resultsBox;

#endif /* UICONTROLS_H_ */
