/**
 * Copyright 2002-2004 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _lucene_search_highlighter_queryscorer_
#define _lucene_search_highlighter_queryscorer_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include <math.h> // float_t
#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/search/SearchHeader.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/highlighter/WeightedTerm.h"
#include "CLucene/highlighter/HighlightScorer.h"
#include "CLucene/highlighter/QueryTermExtractor.h"
#include "CLucene/highlighter/TextFragment.h"

CL_NS_DEF2(search,highlight)

/**
 * {@link Scorer} implementation which scores text fragments by the number of unique query terms found.
 * This class uses the {@link QueryTermExtractor} class to process determine the query terms and 
 * their boosts to be used. 
 */
//TODO: provide option to boost score of fragments near beginning of document 
// based on fragment.getFragNum()
class QueryScorer : public HighlightScorer
{
private:
	TextFragment * _currentTextFragment;
	CL_NS(util)::CLHashSet<const TCHAR*,
		CL_NS(util)::Compare::TChar,
		CL_NS(util)::Deletor::tcArray> _uniqueTermsInFragment;
	double _totalScore;
	double _maxTermWeight;
	CL_NS(util)::LHashMap<const TCHAR*, const WeightedTerm *,
		CL_NS(util)::Compare::TChar,
		CL_NS(util)::Equals::TChar,
		CL_NS(util)::Deletor::tcArray,
		CL_NS(util)::Deletor::Object<const WeightedTerm> > _termsToFind;

public:
	/**
	* 
	* @param query a Lucene query (ideally rewritten using query.rewrite 
	* before being passed to this class and the searcher)
	*/
	QueryScorer(const Query * query);

	/**
	* 
	* @param query a Lucene query (ideally rewritten using query.rewrite 
	* before being passed to this class and the searcher)
	* @param reader used to compute IDF which can be used to a) score selected fragments better 
	* b) use graded highlights eg set font color intensity
	* @param fieldName the field on which Inverse Document Frequency (IDF) calculations are based
	*/
	QueryScorer(const Query* query, CL_NS(index)::IndexReader* reader, const TCHAR* fieldName);

	QueryScorer(WeightedTerm** weightedTerms);

	~QueryScorer();

	/* (non-Javadoc)
	 * @see org.apache.lucene.search.highlight.FragmentScorer#startFragment(org.apache.lucene.search.highlight.TextFragment)
	 */
	void startFragment(TextFragment* newFragment);
	
	/* (non-Javadoc)
	 * @see org.apache.lucene.search.highlight.FragmentScorer#scoreToken(org.apache.lucene.analysis.Token)
	 */
	float_t getTokenScore(CL_NS(analysis)::Token * token);
	
	/* (non-Javadoc)
	 * @see org.apache.lucene.search.highlight.FragmentScorer#endFragment(org.apache.lucene.search.highlight.TextFragment)
	 */
	float_t getFragmentScore();

	/* (non-Javadoc)
	 * @see org.apache.lucene.search.highlight.FragmentScorer#allFragmentsProcessed()
	 */
	void allFragmentsProcessed();

	/**
	 * 
	 * @return The highest weighted term (useful for passing to GradientFormatter to set
	 * top end of coloring scale.  
		*/
	float_t getMaxTermWeight();

private:
	void initialize(WeightedTerm** weightedTerms);

};

CL_NS_END2

#endif

