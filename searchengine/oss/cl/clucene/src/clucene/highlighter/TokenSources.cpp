#include "CLucene/StdHeader.h"
#include "TokenSources.h"

#include "CLucene/util/VoidList.h"

CL_NS_DEF2(search,highlight)
CL_NS_USE(analysis)
CL_NS_USE(index)
CL_NS_USE(util)

TokenSources::TokenSources(void)
{
}

TokenSources::~TokenSources(void)
{
}

TokenStream* TokenSources::getAnyTokenStream(IndexReader* reader,int32_t docId, TCHAR* field, Analyzer* analyzer)
{
	TokenStream* ts=NULL;

	TermFreqVector* tfv=reader->getTermFreqVector(docId,field);
	if(tfv!=NULL)
	{
//		todo: this is actually very dodgy... we try casting
//		to TermPositionVector, we take the token stream
//		only if the cast works... should have a way of
//		knowing what type this is
		TermPositionVector* tmp = NULL;
		try{
             tmp = dynamic_cast<TermPositionVector *> (tfv);
		}catch(...){
			//ignore
		}
		if ( tmp != NULL )
		    ts=getTokenStream(tmp);
	}
	//No token info stored so fall back to analyzing raw content
	if(ts==NULL)
	{
		ts=getTokenStream(reader,docId,field,analyzer);
	}
	return ts;
}


TokenStream* TokenSources::getTokenStream(TermPositionVector* tpv)
{
    //assumes the worst and makes no assumptions about token position sequences.
    return getTokenStream(tpv,false);   
}

TokenStream* TokenSources::getTokenStream(TermPositionVector* tpv, bool tokenPositionsGuaranteedContiguous)
{
    //an object used to iterate across an array of tokens
    /*class StoredTokenStream extends TokenStream
    {
        Token tokens[];
        int32_t currentToken=0;
        StoredTokenStream(Token tokens[])
        {
            this.tokens=tokens;
        }
        public Token next()
        {
            if(currentToken>=tokens.length)
            {
                return NULL;
            }
            return tokens[currentToken++];
        }            
    }     */   
    //code to reconstruct the original sequence of Tokens
    const TCHAR** terms=tpv->getTerms();          
    const int32_t* freq= (int32_t *)tpv->getTermFrequencies();
	int32_t freqLen = tpv->size();

    size_t totalTokens=0;
	{
		for (int32_t t = 0; t < freqLen; t++)
			totalTokens+=freq[t];
	}

    Token** tokensInOriginalOrder=NULL;
	CLSetList<Token*,Token::OrderCompare>* unsortedTokens = NULL;
    for (int32_t t = 0; t < freqLen; t++)
    {
        TermVectorOffsetInfo** offsets=(TermVectorOffsetInfo**)tpv->getOffsets(t);
        if(offsets==NULL)
            return NULL;
        
        int32_t* pos=NULL;
		int32_t posLen=0;
        if(tokenPositionsGuaranteedContiguous)
        {
            //try get the token position info to speed up assembly of tokens into sorted sequence
            pos=(int32_t *)tpv->getTermPositions(t);
			posLen=1;//todo
        }

		if ( tokensInOriginalOrder != NULL )
			tokensInOriginalOrder = _CL_NEWARRAY(Token*, totalTokens+1);

        if(pos==NULL)
        {	
            //tokens NOT stored with positions or not guaranteed contiguous - must add to list and sort later
            if(unsortedTokens==NULL)
                unsortedTokens=_CLNEW CLSetList<Token*,Token::OrderCompare>(false);
            for (int32_t tp=0; offsets[tp]!=NULL; tp++)
            {
                unsortedTokens->insert(_CLNEW Token(terms[t],
                    offsets[tp]->getStartOffset(),
                    offsets[tp]->getEndOffset()));
            }
        }
        else
        {
            //We have positions stored and a guarantee that the token position information is contiguous
            
            // This may be fast BUT wont work if Tokenizers used which create >1 token in same position or
            // creates jumps in position numbers - this code would fail under those circumstances
            
            //tokens stored with positions - can use this to index straight into sorted array
            for (int32_t tp = 0; tp < posLen; tp++)
            {
                if ( tokensInOriginalOrder )
                    {
                    tokensInOriginalOrder[pos[tp]]=_CLNEW Token(terms[t],
                        offsets[tp]->getStartOffset(),
                        offsets[tp]->getEndOffset());
                    }
            }                
        }
    }
    //If the field has been stored without position data we must perform a sort        
    if(unsortedTokens!=NULL)
    {
		if ( totalTokens<unsortedTokens->size() ){
			_CLDELETE_ARRAY(tokensInOriginalOrder);
			tokensInOriginalOrder = _CL_NEWARRAY(Token*,unsortedTokens->size()+1);
		}
		//the list has already sorted our items //todo:check that this is true...
		if ( tokensInOriginalOrder )
		   unsortedTokens->toArray(tokensInOriginalOrder);
		
		return _CLNEW StoredTokenStream(tokensInOriginalOrder,unsortedTokens->size());
    }else
		return _CLNEW StoredTokenStream(tokensInOriginalOrder,totalTokens);
}

TokenStream* TokenSources::getTokenStream(IndexReader* reader,int32_t docId, TCHAR* field)
{
	TermFreqVector* tfv=reader->getTermFreqVector(docId,field);
	if(tfv==NULL)
	{
		TCHAR buf[250];
		_sntprintf(buf,250,_T("%s in doc #%d does not have any term position data stored"),field,docId);
		_CLTHROWT(CL_ERR_IllegalArgument,buf);
		return NULL;
	}

	//todo:bad way of doing this...
	TermPositionVector* tmp = NULL;
	try{
		tmp = dynamic_cast<TermPositionVector *> (tfv); //check to see if tfv is a Tpv
	}catch(...){}
	TokenStream* stream = NULL;
	if ( tmp != NULL ){
		TermPositionVector* tpv = dynamic_cast<TermPositionVector *> (reader->getTermFreqVector(docId,field));
		if ( tpv )
		    stream = getTokenStream(tpv);  
	    //return getTokenStream(tpv);	        
	}else{
		TCHAR buf[250];
		_sntprintf(buf,250,_T("%s in doc #%d does not have any term position data stored"),field,docId);
		_CLTHROWT(CL_ERR_IllegalArgument,buf);
		//return NULL;
	}
	return stream;
}

//convenience method
TokenStream* TokenSources::getTokenStream(IndexReader* reader,int32_t docId, TCHAR* field,Analyzer* analyzer)
{
	CL_NS(document)::Document* doc=reader->document(docId);
	const TCHAR* contents=doc->get(field);
	if(contents==NULL)
	{
		TCHAR buf[250];
		_sntprintf(buf,250,_T("Field %s in document #%d is not stored and cannot be analyzed"),field,docId);
		_CLTHROWT(CL_ERR_IllegalArgument,buf);
		return NULL;
	}
    return analyzer->tokenStream(field,_CLNEW StringReader(contents));
}

TokenSources::StoredTokenStream::StoredTokenStream(CL_NS(analysis)::Token** tokens, size_t len)
{
	currentToken = 0;
    this->tokens=tokens;
	this->length = len;
}
bool TokenSources::StoredTokenStream::next(CL_NS(analysis)::Token* token)
{
    if(currentToken>=length)
    {
        return false;
    }
	Token* t = tokens[currentToken++];

	token->set(t->termText(),t->startOffset(),t->endOffset(),t->type());;
    return true;
}
void TokenSources::StoredTokenStream::close(){
	
}

CL_NS_END2
