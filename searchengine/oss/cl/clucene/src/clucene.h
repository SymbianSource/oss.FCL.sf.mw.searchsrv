
/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
//Includes some standard headers for searching and indexing.
#ifndef _lucene_CLucene_
#define _lucene_CLucene_

#include "clucene/stdheader.h"
#include "CLucene/debug/condition.h"
#include "CLucene/debug/mem.h"
#include "clucene/index/indexreader.h"
#include "CLucene/index/IndexWriter.h"
#include "CLucene/index/MultiReader.h"
#include "clucene/index/term.h"
#include "CLucene/search/indexsearcher.h"
#include "CLucene/index/IndexModifier.h"
#include "CLucene/search/multisearcher.h"
#include "CLucene/search/datefilter.h"
#include "CLucene/search/wildcardquery.h"
#include "CLucene/search/fuzzyquery.h"
#include "CLucene/search/PhraseQuery.h"
#include "CLucene/search/prefixquery.h"
#include "CLucene/search/rangequery.h"
#include "clucene/search/booleanquery.h"
#include "clucene/document/document.h"
#include "CLucene/document/Field.h"
#include "clucene/document/datefield.h"
#include "clucene/store/directory.h"
#include "CLucene/store/FSDirectory.h"
#include "CLucene/queryParser/QueryParser.h"
#include "CLucene/analysis/standard/StandardAnalyzer.h"
#include "CLucene/analysis/Analyzers.h"
#include "CLucene/util/Reader.h"
#include "CLucene/highlighter/Highlighter.h"

#endif
