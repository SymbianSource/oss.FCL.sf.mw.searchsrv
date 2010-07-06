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

#include "CLucene.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"

#include "cpixtools.h"

#include "cpixhits.h"
#include "cpixsearch.h"
#include "cpixidxdb.h"
#include "cpixexc.h"
#include "iidxdb.h"
#include "initparams.h"
#include "cpixutil.h"
#include "iqrytype.h"

#include "indevicecfg.h"


namespace Cpix
{
    // from cluceneqrytype.cpp
    IQryType * CreateCLuceneQryType();
}


namespace
{

	const wchar_t STAR[] = L"*";
    const wchar_t AND1[] = L"and1";
    const wchar_t AND2[] = L"and2";

    /**
     * Parses the syntax "* ( ( AND | && ) QRY )?", setting the member
     * clQryStr_ (clucene query) to QRY, if any.
     */
    class DumpQryCall
    {
    private:
        
        Cpt::Lex::MultiTokenizer     * tokenizer_;
        

        // for the transition table definition, see comments for parse()
        typedef int State;
        typedef Cpt::Lex::token_type_t Symbol;
        typedef std::pair<State, Symbol> StateSymbolPair;
        typedef std::map<StateSymbolPair, State> TransitionTable;
        TransitionTable                transitions_;

        typedef std::set<State> StateSet;
        StateSet                       finalStates_;

        std::wstring                   clQryStr_;

    public:
        DumpQryCall()
            : tokenizer_(NULL)
        {
            transitions_[StateSymbolPair(0, STAR)] = 1;
            transitions_[StateSymbolPair(1, AND1)] = 2;
            transitions_[StateSymbolPair(2, AND2)] = 2;

            finalStates_.insert(1);

            // state #2 is NOT, in theory, a final state, but with
            // manual handling it is treated as such (as ther is no
            // state #3).
            finalStates_.insert(2);

            using namespace std;
            using namespace Cpt::Lex;
            using namespace Cpix;

            auto_ptr<Tokenizer>
                star(new SymbolTokenizer(STAR,
                                         DUMPSEARCH_MARKER_STR)),
                and1(new SymbolTokenizer(AND1,
                                         L"AND")),
                and2(new SymbolTokenizer(AND2,
                                         L"&&")),
                ws(new WhitespaceTokenizer);


            Tokenizer * tokenizers[] = {
                star.get(), and1.get(), and2.get(), ws.get(), NULL
            };

            auto_ptr<MultiTokenizer>
                tokenizer(new MultiTokenizer(tokenizers, true));
            tokenizer_ = tokenizer.get();

            star.release();
            and1.release();
            and2.release();
            ws.release();
            tokenizer.release();
        }


        ~DumpQryCall()
        {
            delete tokenizer_;
        }


        const std::wstring & clQryStr() const
        {
            return clQryStr_;
        }
        
      
        /**
         * TODO - this is the second manual regexp -> automaton
         * computation, perhaps a generic regexp tool should be
         * used. BUT: consider that both state machines have manual
         * handling of embedded queries.
         *
         * Parses the syntax "* ( ( AND | && ) Q )?".
         *
         * Regular expression
         * ------------------
         *
         *  * ( ( AND | && ) Q )?
         *
         * where
         *
         *  *   - token for '*' (dump query shorthand identifier)
         *  AND - clucene boolean operand 'AND'
         *  &&  - clucene boolean operand '&&', alternative to 'AND'
         *  Q   - clucene query
         *  ( ) | ? - usual regular expression operators
         * 
         * Computation of transition table of automaton
         * --------------------------------------------
         *
         *  Dot in the dotted item is marked with '@', for instance
         * 
         *      * @ ( ( AND | && ) Q )?
         *
         *  meaning dollar token was consumed, and identifier token is
         *  expected.
         *
         *  States of the automaton are "labelled" with (= identified
         *  by) set of basic dotted items. (Basic dotted item: a
         *  dotted item that is about to consume a token.)
         *
         *  State #0 (initial state):
         *    @ * ( ( AND | && ) Q )?
         *
         *  State #1 (possible final state):
         *   * ( ( @ AND | && ) Q )?
         *   * ( ( AND | @ && ) Q )?
         *   * ( ( AND | && ) Q )? @        ( <-- consumed input)
         *
         *  State #2:
         *   * ( ( AND | && ) @ Q )?
         *
         *  State #3 (final state):
         *   * ( ( AND | && ) Q )? @        ( <-- consumed input)
         *
         *
         *  Transition table is function: (state,token) -> state,
         *  undefined slots mean error.
         *
         *        | * | A | & | Q |
         *  ======+===+===+===+===+
         *   #0   |#1 |   |   |   |
         *  ------+---+---+---+---+
         *   #1   |   |#2 |#2 |   |
         *  ------+---+---+---+---+
         *   #2   |   |   |   |#3 |
         *  ------+---+---+---+---+
         *   #3   |   |   |   |   |
         *  ------+---+---+---+---+
         *   
         *  Set of final states: 1, 3.
         *
         *  NOTE: Whatever is stated about the inner query in
         *  iqrytype.cpp, is also true here. Therefore we are going to
         *  apply the same solution to the same problem.
         *
         *  NOTE: Because of this, there is not going to be a state #3
         *  declared, after state #2 the special handling will kick
         *  in.
         */
        void parse(const wchar_t * qryStr)
        {
            using namespace Cpt;
            using namespace Cpt::Lex;
            using namespace std;
            
            Tokens
                source(*tokenizer_,
                       qryStr);
            StdFilter
                tokens(source);

            State
                state = 0;
            bool
                hasFoundClQryStr = false;

            while (!hasFoundClQryStr && tokens)
                {
                    Token
                        token = tokens++;

                    TransitionTable::iterator
                        found = transitions_.find(StateSymbolPair(state, 
                                                                  token.type()));

                    if (found == transitions_.end())
                        {
                            THROW_CPIXEXC("Parse error - bad syntax '%S'.",
                                          qryStr);
                        }

                    state = (*found).second;
                    
                    switch (state)
                        {
                        case 2:
                            // the rest of the qry is the clucene qry
                            clQryStr_ = token.end();
                            hasFoundClQryStr = true;                            break;
                        default:
                            ;
                        };
                }

            if (finalStates_.find(state) == finalStates_.end())
                {
                    THROW_CPIXEXC("Parse error - premature end of input '%S'",
                                  qryStr);
                }
        }

    };

}

namespace Cpix
{

    /**
     * Format and semantics are described in iqrytype.cpp
     */
    class DumpQryType : public IQryType
    {
    private:
        //
        // private members
        //

        // for queries delegated to clucene
        IQryType               * clQry_;
        
        // for dump queries
        lucene::search::Query  * dumpQry_;
        

    public:
        //
        // public operators
        //
            
            
        DumpQryType()
            : clQry_(NULL),
              dumpQry_(NULL)
        {
            ;
        }

        
        ~DumpQryType()
        {
            delete clQry_;
            delete dumpQry_;
        }


        //
        // from interface IQryType
        //
        virtual void setUp(cpix_QueryParser              * queryParser,
                           const std::list<std::wstring> & args,
                           const wchar_t                 * qryStr)
        {
            std::wstring
                clQryStr;

            if (StartsWith(qryStr, DUMPSEARCH_MARKER_CHAR))
                {
                    // the original syntax is "*" or "* AND QRY" and
                    // the qryStr is the whole original query string
                    DumpQryCall
                        dqc; // TODO: should this be turned into a
                             // singleton?

                    dqc.parse(qryStr);

                    clQryStr = dqc.clQryStr();
                }
            else
                {
                    // the original syntax was "$dump" or "$dump(QRY)"
                    // and qryStr is the inner (clucene) query, if any

                    clQryStr = qryStr;
                }
            if (clQryStr.length() > 0)
                {
                    std::auto_ptr<IQryType>
                        clQry(CreateCLuceneQryType());
                    clQry->setUp(queryParser,
                                 args,
                                 clQryStr.c_str());
                    clQry_ = clQry.release();
                }
            else
                {
                    using namespace lucene::search;
                    using namespace lucene::index;

                    Term
                        * term = _CLNEW Term(LCPIX_APPCLASS_FIELD,
											 L"root");
                    dumpQry_ = new TermQuery(term);
                    
                    _CLDECDELETE(term);
                }
        }


        virtual cpix_Hits * search(cpix_IdxSearcher * idxSearcher)
        {
            return searchIDX(idxSearcher);
        }


        virtual cpix_Hits * search(cpix_IdxDb * idxDb)
        {
            return searchIDX(idxDb);
        }


    private:
        //
        // private implementation details
        //
        template<typename IDX>
        cpix_Hits * searchIDX(IDX * idx)
        {
            cpix_Hits
                * rv = NULL;

            if (clQry_ == NULL)
                {
                    // perform our own dump query
                    rv = CLuceneSearchIdx(idx,
                                          dumpQry_);
                }
            else
                {
                    // delegate to clQry_
                    rv = clQry_->search(idx);
                }

            return rv;
        }
    };
    


    IQryType * CreateDumpQryType()
    {
        return new DumpQryType;
    }

}

