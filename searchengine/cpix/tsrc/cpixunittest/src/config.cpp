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

#include "config.h"

#ifdef __WINS__ 
const char* Mp3TestCorpus[] = {
	MEDIA_TEST_CORPUS_PATH "\\Albert Einstein, E mc 2.mp3",
	MEDIA_TEST_CORPUS_PATH "\\One Step For Man.mp3",
	MEDIA_TEST_CORPUS_PATH "\\The Eagle Has Landed.mp3",
	0
};
#else
const char* Mp3TestCorpus[] = {
	MEDIA_TEST_CORPUS_PATH "\\Albert Einstein, E mc 2.mp3",
	MEDIA_TEST_CORPUS_PATH "\\One Step For Man.mp3",
	MEDIA_TEST_CORPUS_PATH "\\The Eagle Has Landed.mp3",
	0
};
#endif 
