/*
 * config.h
 *
 *  Created on: Jun 3, 2009
 *      Author: admin
 */
 
 /*------------------------------------------------------------------------------
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#ifndef CLEUT_CONFIG_H_
#define CLEUT_CONFIG_H_

#define INDEX_DIRECTORY "c:\\data\\clextraunittest\\_work\\"

#ifdef __WINS__ 
const char CORPUS_FILE[] = "c:\\data\\indexing\\searched\\perfmetrics\\hugetestcorpus.txt";

const char INPUT_INDEX_DIRECTORY[] = "c:\\data\\clextraunittest\\_input\\index";
#else
const char CORPUS_FILE[] = "e:\\data\\indexing\\searched\\perfmetrics\\hugetestcorpus.txt";
const char INPUT_INDEX_DIRECTORY[] = "e:\\data\\clextraunittest\\_input\\index";
#endif

#endif /* CLEUT_CONFIG_H__ */
