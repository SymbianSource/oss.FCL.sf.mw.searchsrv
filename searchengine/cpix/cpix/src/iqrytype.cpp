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

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include <map>
#include <memory>
#include <set>

#include "cpixsearch.h"
#include "cpixidxdb.h"
#include "iidxdb.h"
#include "iqrytype.h"
#include "cpixhits.h"
#include "cpixexc.h"

#include "cpixparsetools.h"

#include "fwdtypes.h"
#include "common/cpixlog.h"


/**
 * This file contains the main logic for query parsing. Sub query
 * types (implementing interface IQrytype) are enumerated here for
 * use.
 *
 * General syntax for queries
 * ==========================
 *
 * A general description of the syntax is found in iqrytype.h.
 *
 *
 * Query types and their syntax / arguments
 * ========================================
 *
 * 1 CLucene query
 * ---------------
 *
 * A query that does not start with either of the tokens '$', '*' is
 * interpreted as a clucene query.
 *
 * A clucene query searches on the field the query parser was
 * constructed with. The clucene query can also be invoked explicitly,
 * using the generic syntax and the 'plain' query type identifier:
 *
 *   '$plain(QRY)'
 *
 * where QRY is the clucene query itself. Therefore '$plain(QRY)' and
 * 'QRY' are equivalent.
 *
 *
 * 2 Dump query
 * ------------
 *
 * A dump query is identified by a single '*' query string or '$dump'
 * using the generic syntax and the query type identifier 'dump'. It
 * does not need any argument and it does not take into account the
 * field with wich the query parser was constructed with. The
 * resulting hits will contain all the documents - hence its name.
 *
 * The dump query can take two additional forms:
 *
 *   '* AND QRY' or '$dump(QRY)'
 *
 * these two have the same meaning, namely, dump all documents that
 * adhere the actual criteria given by QRY. In fact, these are
 * resolved as plain, clucene queries. In other words, these for query
 * syntaxes are equivalent:
 *
 *   'QRY'
 *   '$plain(QRY)'
 *   '* AND QRY'
 *   '$dump(QRY)'
 *
 * The reason dump query is made special by delegating tasks to plain
 * clucene query is that clients may have easier time to construct
 * query strings. For instance, the first term might be a word the
 * user entered or '*' if she entered nothing, and depending on
 * whether there is some extra search criteria (like '_appclass:'root
 * file media jpg') can be concatenated at will. That is, the two
 * parts of a query string can be independently constructed based on
 * two unrelated variables (1: whether the user has entered anything,
 * 2: what context / extra criteria we have).
 *
 *
 *
 * 3 Terms query
 * -------------
 *
 * Uses the field given to query parser, and enumerates all the
 * existing terms on that field that match a pattern. It accepts
 * integer argument in the query strings limiting the number of hits
 * to return and an optional application class prefix argument. 
 * The application class prefix given as a string literal. For example:
 *
 * '$terms<10>(happ*)'
 *
 * Enumerate at most 10 terms that are found in a specific field that
 * start with 'happ'.
 * 
 * Following example queries search for all terms starting with 'beatl'
 * occurring in documents, which base application class begins with 
 * 'root file media' (e.g. 'root file media mp3'). 
 * 
 * '$terms<10, 'root file media'>(beatl*)'
 *
 * NOTE: Enumerating all the terms with a syntax like '$terms<10>(*)'
 * is not supported.
 *
 * NOTE: Other clucene wildcard query syntaxes may be used, like
 *
 * '$terms<10>(happ?)' or '$terms<10>(ha*n)'.
 *
 *
 * 4 Address query
 * ---------------
 *
 * Ignores the field given to query parser, and requires two real
 * (floating point numbers) arguments representing GPS reference point
 * (latitude and longitude) coordinates given in decimal form. The
 * inner query itself is a list of words, processed by the address
 * search logic. Example:
 *
 * '$address<60.1234,23.456>(Mannerheimintie)'
 *
 * the returned results are ordered according to their distance from
 * the given reference point.
 *
 *
 *
 * 5 QNR search
 * ------------
 *
 * Performs a more or less conventional clucene search and tries to
 * find documents, in an ever widening circle, that fall within the
 * vicinity of a given GPS reference point.
 *
 * Prerequisite: the documents must have a QNR field prepared
 * (indevicecfg.h : QNR_FIELD) for this type of search to work. See
 * QUAD_FILTER.
 *
 * Syntax:
 *
 * '$quad<RADIUS,MAXHITCOUNT,RPLAT,RPLONG,CPLAT1,CPLONG1, ...>(QUERY)'.
 *
 * Where:
 *
 *        RADIUS is the maximum radius, in kilometers defining the
 *        vicinity. Actual distance of hits from the reference may be
 *        more, but not significantly. Mandatory argument.
 *
 *        MAXHITCOUNT is the maximym number of hits we are interested
 *        in. The searching that goes on for ever widening areas
 *        terminates when either the readius or the maxhitcount is
 *        exceeded. Mandatory argument.
 *
 *        RPLAT, RPLONG: the GPS coordinates of the reference point,
 *        according to which the hit resuls should be sorted based on
 *        the distance. Mandatory argument.
 *
 *        CPLAT1, CPLONG1: the GPS coordinates for the first center
 *        point around wich hits are being searched for. At least one
 *        pair of floats (GPS coordinate in decimal notation) is
 *        required, can be more, but always in pairs, obviously.
 *
 *        QUERY any clucene-type of query, if any, for some other
 *        fields, etc. Optional.
 *
 * Example:
 *
 * '$quad<4.0,10,60.154023,24.887724,0.0,0.0>'
 *
 * will look for at most 10 documents that are not significantly
 * further from the one center point GPS(0.0,0.0) than 4.0
 * kilometers. Any such hits are sorted based on their distance from
 * the reference point GPS(lat:60.154023,long:24.887724).
 *
 * Example:
 *
 * '$quad<4.0,10,60.154023,24.887724,0.0,0.0>(pizza)'
 *
 * The same as above, but now it looks for the term pizza in the field
 * specified to the query parser as a default search field.
 * 
 * 
 * 6 Prefix query
 * ------------
 *
 * 
 *
 *   '$Prefix(QRY)'
 *
 * these two have the same meaning, namely, dump all documents that
 * adhere the actual criteria given by QRY. In fact, these are
 * resolved as plain, clucene queries. In other words, these for query
 * syntaxes are equivalent:
 *
 *   '$Prefix(QRY)'
 *
 * The reason dump query is made special by delegating tasks to plain
 * clucene query is that clients may have easier time to construct
 * query strings. For instance, the first term might be a word the
 * user entered or '*' if she entered nothing, and depending on
 * whether there is some extra search criteria (like '_appclass:'root
 * file media jpg') can be concatenated at will. That is, the two
 * parts of a query string can be independently constructed based on
 * two unrelated variables (1: whether the user has entered anything,
 * 2: what context / extra criteria we have).
 *
 */


namespace Cpix
{
    // from qrytype/cluceneqrytype.cpp
    IQryType * CreateCLuceneQryType();

    // from qrytype/addressqrytype.cpp
    IQryType * CreateAddressQryType();

    // from qrytype/termsqrytype.cpp
    IQryType * CreateTermsQryType();

    // from qrytype/dumpqrytype.cpp
    IQryType * CreateDumpQryType();

    // from qrytype/quadqrytype.cpp
    IQryType * CreateQuadQryType();

    // from qrytype/prefixqrytype.cpp
    IQryType * CreatePrefixQryType();


    bool StartsWith(const wchar_t * qryStr,
                    wchar_t         wc)
    {
        while (*qryStr != 0
               && isspace(*qryStr))
            {
                ++ qryStr;
            }

        bool
            rv = (*qryStr == wc);

        return rv;
    }


    const wchar_t CLUCENE_QRYTYPEID[] = L"plain";
    const wchar_t DUMP_QRYTYPEID[]    = L"dump";

    const wchar_t UNIFIEDSEARCH_MARKER_CHAR = L'$';
    const wchar_t UNIFIEDSEARCH_MARKER_STR[] = { 
        UNIFIEDSEARCH_MARKER_CHAR,
        L'\0'
    };


    const wchar_t DUMPSEARCH_MARKER_CHAR = L'*';
    const wchar_t DUMPSEARCH_MARKER_STR[] = { 
        DUMPSEARCH_MARKER_CHAR,
        L'\0'
    };
    
}


namespace
{

    struct QryTypeInfo
    {
        const wchar_t      * qryTypeId_;
        Cpix::IQryType  * (* factory_)();
    };


    QryTypeInfo QryTypeInfos[] =
        {
            { Cpix::CLUCENE_QRYTYPEID,  &Cpix::CreateCLuceneQryType },
            { Cpix::DUMP_QRYTYPEID,     &Cpix::CreateDumpQryType    },
            { L"address",               &Cpix::CreateAddressQryType },
            { L"terms",                 &Cpix::CreateTermsQryType   },
            { L"quad",                  &Cpix::CreateQuadQryType    },
            { L"prefix",                &Cpix::CreatePrefixQryType  },

            // TODO more search plugins here

            { NULL,                   NULL }
        };

}


namespace Cpix
{

    Cpt::Lex::MultiTokenizer  & QryCall::tokenizer()
    {
        using namespace Cpt::Lex;
        using namespace std;
        using namespace Cpix;

        if (tokenizer_ == NULL)
            {
                // The tokenizers_ and tokenizer_ static member
                // functions once initialized, are never freed (so
                // it is not a leak). However, we should prevent
                // any leaks during constructing these two
                // statics.

                auto_ptr<Tokenizer>
                    dollar(new SymbolTokenizer(DOLLAR, 
                                               UNIFIEDSEARCH_MARKER_STR));
                auto_ptr<Tokenizer>
                    lessThan(new SymbolTokenizer(LESSTHAN, 
                                                 L"<"));
                auto_ptr<Tokenizer>
                    greaterThan(new SymbolTokenizer(GREATERTHAN, 
                                                    L">"));
               
                auto_ptr<Tokenizer>
					comma(new SymbolTokenizer(COMMA, 
                                              L","));
                
                auto_ptr<Tokenizer>
                    leftParenthesis(new SymbolTokenizer(LEFTPARENTHESIS, 
                                                        L"("));
                
                auto_ptr<Tokenizer>
                    rightParenthesis(new SymbolTokenizer(RIGHTPARENTHESIS, 
                                                         L")"));
                        
                auto_ptr<Tokenizer>
                    ws(new WhitespaceTokenizer);
                
                auto_ptr<Tokenizer>
                    id(new IdTokenizer);
                
                auto_ptr<Tokenizer>
                    lit(new LitTokenizer(L'\''));

                Tokenizer* tokenizers[] = {
                    dollar.get(), lessThan.get(), greaterThan.get(),
                    comma.get(), leftParenthesis.get(),
                    rightParenthesis.get(), ws.get(), id.get(),
                    lit.get(), NULL
                };

                tokenizer_ = new MultiTokenizer(tokenizers, true); // pass ownership to multitokenizer

                dollar.release();
                lessThan.release();
                greaterThan.release();
                comma.release();
                leftParenthesis.release();
                rightParenthesis.release();
                ws.release();
                id.release();
                lit.release();
            }

        return *tokenizer_;
    }

        
    QryCall::TransitionTable & QryCall::transitions()
    {
        if (transitions_ == NULL)
            {
                using namespace std;
                using namespace Cpt::Lex;

                transitions_ = new TransitionTable;
                (*transitions_)[StateSymbolPair(0, DOLLAR)]          = 1;
                (*transitions_)[StateSymbolPair(1, TOKEN_ID)]        = 2;
                (*transitions_)[StateSymbolPair(2, LESSTHAN)]        = 3;
                (*transitions_)[StateSymbolPair(2, LEFTPARENTHESIS)] = 7;
                (*transitions_)[StateSymbolPair(3, TOKEN_LIT)]       = 4;
                (*transitions_)[StateSymbolPair(4, GREATERTHAN)]     = 6;
                (*transitions_)[StateSymbolPair(4, COMMA)]           = 5;
                (*transitions_)[StateSymbolPair(5, TOKEN_LIT)]       = 4;
                (*transitions_)[StateSymbolPair(6, LEFTPARENTHESIS)] = 7;
            }

        return *transitions_;
    }


    QryCall::StateSet & QryCall::finalStates()
    {
        if (finalStates_ == NULL)
            {
                finalStates_ = new StateSet;

                finalStates_->insert(2);
                finalStates_->insert(6);

                // state #7 is NOT, in theory, a final state, but
                // with our special, manual handling it is treated
                // as such (there are no states #8 and #9).
                finalStates_->insert(7);
            }

        return * finalStates_;
    }


    /**
     * Parses only the extended syntax
     *
     *
     *  Regular expression
     *  ------------------
     *
     *   $ I (< L (, L)* >)? ({ Q })?
     *
     *   where
     *
     *     $ - token for '$'
     *     I - identifier token
     *     < - less than token
     *     > - greater than token
     *     L - literal token
     *     , - comma token
     *     { - left parenthesis ('(') token (!)
     *     } - right parenthesis (')') token (!)
     *     Q - clucene query (inner query)
     *
     *     ( ) * ? - usual regular expression operators
     *
     *
     *  Computation of transition table of automaton
     *  --------------------------------------------
     * 
     *  Dot in the dotted item is marked with '@', for instance
     * 
     *      $ @ I (< L (, L)* >)? ({ Q })?
     *
     *  meaning dollar token was consumed, and identifier token is
     *  expected.
     *
     *  States of the automaton are "labelled" with (= identified
     *  by) set of basic dotted items. (Basic dotted item: a
     *  dotted item that is about to consume a token.)
     *
     *  State #0 (initial state):
     *    @ $ I (< L (, L)* >)? ({ Q })?
     *
     *  State #1:
     *    $ @ I (< L (, L)* >)? ({ Q })?
     *
     *  State #2 (possible final state):
     *    $ I (@ < L (, L)* >)? ({ Q })?
     *    $ I (< L (, L)* >)? (@ { Q })?
     *    $ I (< L (, L)* >)? ({ Q })? @      ( <-- consumed input )
     *
     *  State #3:
     *    $ I (< @ L (, L)* >)? ({ Q })?
     *
     *  State #4:
     *    $ I (< L (@ , L)* >)? ({ Q })?
     *    $ I (< L (, L)* @ >)? ({ Q })?
     *
     *  State #5:
     *    $ I (< L (, @ L)* >)? ({ Q })?
     *
     *  State #6 (possible final state):
     *    $ I (< L (, L)* >)? (@ { Q })?
     *    $ I (< L (, L)* >)? ({ Q })? @      ( <-- consumed input )
     *
     *  State #7:
     *    $ I (< L (, L)* >)? ({ @ Q })?
     *
     *  State #8:
     *    $ I (< L (, L)* >)? ({ Q @ })?
     *
     *  State #9 (possible final state):
     *    $ I (< L (, L)* >)? ({ Q })? @      ( <-- consumed input )
     *
     *
     *  Transition table is function: (state,token) -> state,
     *  undefined slots mean error.
     *
     *        | $ | I | < | > | L | , | { | } | Q |
     *  ======+===+===+===+===+===+===+===+===+===
     *   #0   |#1 |   |   |   |   |   |   |   |   |
     *  ------+---+---+---+---+---+---+---+---+---+
     *   #1   |   |#2 |   |   |   |   |   |   |   |
     *  ------+---+---+---+---+---+---+---+---+---+
     *   #2   |   |   |#3 |   |   |   |#7 |   |   |
     *  ------+---+---+---+---+---+---+---+---+---+
     *   #3   |   |   |   |   |#4 |   |   |   |   |
     *  ------+---+---+---+---+---+---+---+---+---+
     *   #4   |   |   |   |#6 |   |#5 |   |   |   |
     *  ------+---+---+---+---+---+---+---+---+---+
     *   #5   |   |   |   |   |#4 |   |   |   |   |
     *  ------+---+---+---+---+---+---+---+---+---+
     *   #6   |   |   |   |   |   |   |#7 |   |   |
     *  ------+---+---+---+---+---+---+---+---+---+
     *   #7   |   |   |   |   |   |   |   |   |#8 |
     *  ------+---+---+---+---+---+---+---+---+---+
     *   #8   |   |   |   |   |   |   |   | #9|   |
     *  ------+---+---+---+---+---+---+---+---+---+
     *   #9   |   |   |   |   |   |   |   |   |   |
     *  ------+---+---+---+---+---+---+---+---+---+
     *   
     *  Set of final states: 2, 6, 9.
     * 
     *  NOTE: The Q token is referred here as it would be a
     *  primitive token, but in fact it is an entire inner query,
     *  which can be possible a full-fledged clucene
     *  query. Obviously, we are not going to write a parser that
     *
     *    o can recognize full clucene query syntax
     * 
     *    o and is flexible to be extended to recognize other
     *      inner query syntaxes (like address search syntax).
     *
     *  Fortunately, there is no need to, with a bit of manual
     *  hack. If the automaton gets to state #7, then basically it
     *  gets very simple and linear: it must read the inner query,
     *  then a right parenthesis, and then the input stream must
     *  end. So, in practice, in state #7, we will
     *
     *    o search for the right parenthesis (if none found, issue
     *      error)
     *
     *    o and say that anything between the left and right
     *      parentheses is Q, the inner query.
     *
     *  Therefore, there is not even going to be a state #8 and #9
     *  declared, and state #7 shall be handled exceptionally.
     *
     *  NOTE Actually, some other states need special handling to
     *  retrieve the value of the identifiers and literals during
     *  parsing. For instance, during state transition (#1,I)->#2
     *  we have a chance to retrieve and store the (query type)
     *  identifier. Or, during state transition (#3,L)->#4 we can
     *  retrieve store the first argument literal, and at
     *  (#5,L)->#4 we can retrieve and store further arguments. In
     *  this sense, state transtion (#7,Q)->#8 is only a bit
     *  weird.
     */
    void QryCall::parse(const wchar_t * qryStr)
    {
        using namespace Cpt;
        using namespace Cpt::Lex;
        using namespace std;
            
        Tokens
            source(tokenizer(),
                   qryStr);
        WhiteSpaceFilter
            tokens(source);

        State
            state = 0;
        bool
            hasFoundInnerQryStr = false;

        while (!hasFoundInnerQryStr && tokens)
            {
                Token
                    token = tokens++;

                TransitionTable::iterator
                    found = transitions().find(StateSymbolPair(state, 
                                                               token.type()));

                if (found == transitions().end())
                    {
                        THROW_CPIXEXC("Parse error - bad syntax '%S'",
                                      qryStr);
                    }

                state = (*found).second;
                    
                switch (state)
                    {
                    case 2:
                        setQryTypeId(token);
                        break;
                    case 4:
                        addNextArg(token);
                        break;
                    case 7:
                        setInnerQryStr(token,
                                       qryStr);
                        hasFoundInnerQryStr = true;
                        break;
                    default:
                        ;
                    };
            }

        if (finalStates().find(state) == finalStates().end())
            {
                THROW_CPIXEXC("Parse error - premature end of input '%S'",
                              qryStr);
                
            }
    }

    void QryCall::setQryTypeId(const wchar_t * tokenText)
    {
        qryTypeId_ = tokenText;
    }


    void QryCall::setQryTypeId(const Cpt::Lex::Token & token)
    {
        setQryTypeId(token.text().c_str());
    }

        
    void QryCall::addNextArg(const Cpt::Lex::Token & token)
    {
        args_.push_back(token.text());
    }

        
    void QryCall::setInnerQryStr(const wchar_t     * innerQryStrBegin,
                                 const wchar_t     * innerQryStrEnd)
    {
        innerQryStr_.assign(innerQryStrBegin,
                            innerQryStrEnd);
    }


    void QryCall::setInnerQryStr(const Cpt::Lex::Token   & token,
                                 const wchar_t           * qryStr)
    {
        // At this point we search for the ending ')' and say that
        // anything between the left-side-parenthesis ('(') and
        // the right-side-parenthesis is the inner query string.
        //
        // We don't want to use the tokenizers to find the
        // right-side-parenthesis, because the inner query string
        // may have whatever extra literals, symbols, etc that
        // would not be recognized by our parser that is meant to
        // parse the unified part of a query, not the specific
        // ones.
        const wchar_t
            * rangeStart = token.end(),
            * rangeEnd = qryStr + wcslen(qryStr) - 1;

        while (rangeEnd >= rangeStart
               && isspace(*rangeEnd))
            {
                --rangeEnd;
            }

        if (*rangeEnd == L')')
            {
                setInnerQryStr(token.end(),
                               rangeEnd);
            }
        else
            {
                // throw CpixExc("Parse error - unbalanced parenthesis");
                THROW_CPIXEXC("Parse error - unbalanced parenthesis '%S'",
                              qryStr);
            }
    }
    

    void QryCall::init()
    {
        // invoking these functions just to force them to create their
        // respective static member instances
        tokenizer();
        transitions();
        finalStates();
    }


    void QryCall::releaseCache() 
    {
        delete tokenizer_;
        tokenizer_ = 0; 
        delete transitions_;
        transitions_ = 0; 
        delete finalStates_;
        finalStates_ = 0; 
    }


    QryCall::QryCall(const wchar_t * qryStr)
    {
        using namespace Cpix;

        logDbgMsg("QryCall::QryCall('%S').",
                  qryStr == NULL ? L"<NULL>" : qryStr);

        /* OBS
        logDbgMsg("QryCall::QryCall('%S'). BEGIN",
                  qryStr == NULL ? L"<NULL>" : qryStr);
        */
        Cpt::StopperWatch
            stopperWatch;

        if (qryStr == NULL || *qryStr == 0)
            {
                THROW_CPIXEXC("Empty or NULL query string");
            }

        if (StartsWith(qryStr, DUMPSEARCH_MARKER_CHAR))
            {
                setQryTypeId(DUMP_QRYTYPEID);
                setInnerQryStr(qryStr,
                               qryStr + wcslen(qryStr));
            }
        else if (!StartsWith(qryStr, UNIFIEDSEARCH_MARKER_CHAR))
            {
                setQryTypeId(CLUCENE_QRYTYPEID);
                setInnerQryStr(qryStr,
                               qryStr + wcslen(qryStr));
            }
        else
            {
                parse(qryStr);
            }

        /* OBS
        logDbgMsg("QryCall::QryCall. END (elapsed: %ld ms)",
                  stopperWatch.elapsedMSecs());
        */
    }


    
    Cpt::Lex::MultiTokenizer   * QryCall::tokenizer_   = NULL;
    QryCall::TransitionTable   * QryCall::transitions_ = NULL;
    QryCall::StateSet          * QryCall::finalStates_ = NULL;

}


namespace Cpix
{

    IQryType * IQryType::parseQry(cpix_QueryParser * queryParser,
                                  const wchar_t    * qryStr)
    {
        IQryType
            * rv = NULL;

        QryCall
            qryCall(qryStr);

        QryTypeInfo
            * qti = QryTypeInfos;
        for (; qti->qryTypeId_ != NULL; ++qti)
            {
                if (qryCall.qryTypeId_ == qti->qryTypeId_)
                    {
                        break;
                    }
            }

        if (qti->qryTypeId_ == NULL)
            {
                THROW_CPIXEXC("Unknown qry type '%S'",
                              qryCall.qryTypeId_.c_str());
            }

        rv = qti->factory_();

        rv->setUp(queryParser,
                  qryCall.args_,
                  qryCall.innerQryStr_.c_str());

        return rv;
    }
    

    IQryType::IQryType()
    {
        ;
    }


    
    IQryType::~IQryType()
    {
        ;
    }
    
    IHits * CLuceneSearchIdxExc(IdxDbHndl               handle,
                                lucene::search::Query * qry)
    {
        IIdxDb* idx = IIdxDb::getPtr(handle);

        Version
            version = 0;
        lucene::search::Hits
            * hits = idx->search(qry,
                                 &version);

        return new LuceneHits(hits, qry, handle, version);
    }
    
}
