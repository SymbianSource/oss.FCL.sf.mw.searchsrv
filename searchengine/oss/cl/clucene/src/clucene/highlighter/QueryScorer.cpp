#include "clucene/stdheader.h"
#include "queryscorer.h"
#include <wchar.h>

CL_NS_DEF2(search,highlight)
CL_NS_USE(index)
CL_NS_USE(analysis)

	QueryScorer::QueryScorer(const Query * query):
        _uniqueTermsInFragment(true),
		_termsToFind(false,true)
		
	 {
		 WeightedTerm** _weighted_terms = QueryTermExtractor::getTerms(query);
		 initialize(_weighted_terms);
		 _CLDELETE_ARRAY(_weighted_terms);
	 }
	 QueryScorer::~QueryScorer()
	 {
	 }

/*	 QueryScorer(Query* query, CL_NS(index)::IndexReader* reader, const TCHAR* fieldName)
	 {
		 WeightedTerm** _weighted_terms = QueryTermExtractor.getIdfWeightedTerms(query, reader, fieldName);
		 initialize(_weighted_terms);
	 }*/


	QueryScorer::QueryScorer(WeightedTerm** weightedTerms)
	{
		 initialize(weightedTerms);
	}
	
	void QueryScorer::initialize(WeightedTerm** weightedTerms)
	{
		_currentTextFragment = NULL;
		_totalScore = 0;
		_maxTermWeight = 0;

		// Copy external weighted terms
		 int i=0;
		 while ( weightedTerms[i] != NULL ){
			const WeightedTerm* existingTerm=_termsToFind.get(weightedTerms[i]->getTerm());
			if( (existingTerm==NULL) ||(existingTerm->getWeight()<weightedTerms[i]->getWeight()) )
  	        {
  				//if a term is defined more than once, always use the highest scoring weight
				WeightedTerm* term = weightedTerms[i];
				_termsToFind.put(term->getTerm(), term);

				_maxTermWeight=max(_maxTermWeight,weightedTerms[i]->getWeight());
  	        }else
				_CLDELETE(weightedTerms[i]);

			i++;
		 }
	}

	void QueryScorer::startFragment(TextFragment * newFragment)
	{
		_uniqueTermsInFragment.clear();
		_currentTextFragment=newFragment;
		_totalScore=0;
		
	}
	/*
	 * Compares the Query term to Field text token if match, returns 1
	 *
	 */

	int QueryScorer::matchQuryText(const TCHAR* termText)
	{
			CL_NS(util)::LHashMap<const TCHAR*, const WeightedTerm *,
		CL_NS(util)::Compare::TChar,
		CL_NS(util)::Equals::TChar,
		CL_NS(util)::Deletor::tcArray,
		CL_NS(util)::Deletor::Object<const WeightedTerm> >::iterator i = _termsToFind.begin();
			for(; i != _termsToFind.end() ; i ++)
			{ 
				const TCHAR * temp = i->first;
				int tempLen =  wcslen(temp);
				int matchFlag = 1;
				if(tempLen <= wcslen(termText))
				{
					for(int j = tempLen - 1 ; j >=0 ; j--)
					{
						if(temp[j] != termText[j])
						{
							matchFlag = 0;
							break;
						}
					}
				}
				else
				{
					matchFlag = 0;
				}
				if(matchFlag)
					return 1;
				
     		}
			return 0;
	}

	
	float_t QueryScorer::getTokenScore(Token * token)
	{
		const TCHAR* termText=token->termText();
		
	//	const WeightedTerm* queryTerm = _termsToFind.get(termText);
	// Instead of checking for weighted terms directly match qurey text to field text.
	    int isQueryTerm = matchQuryText(termText);
		if(isQueryTerm==0)
		{
			//not a query term - return
			return 0;
		}
		//found a query term - is it unique in this doc?
		if(_uniqueTermsInFragment.find(termText)==_uniqueTermsInFragment.end())
		{
			//_totalScore+=queryTerm->getWeight();
			/*
			 * Keeping the Score value to 1 
			 *
			 */
				_totalScore+=1;
			TCHAR* owned_term = stringDuplicate(termText);
			_uniqueTermsInFragment.insert(owned_term);
		}
		//return queryTerm->getWeight();
			return 1;
	}
	
	/**
  	*
  	* @return The highest weighted term (useful for passing to GradientFormatter to set
  	* top end of coloring scale.
  	*/
	float_t QueryScorer::getMaxTermWeight()
	{
  		return _maxTermWeight;
	}


	float_t QueryScorer::getFragmentScore(){
		return _totalScore;
	}

CL_NS_END2
