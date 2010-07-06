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

#ifndef CPIX_IQRYTYPE_H
#define CPIX_IQRYTYPE_H

#include <wchar.h>

#include <list>
#include <string>

#include "cpixtools.h"
#include "cpixexc.h"
#include "common/refcountedbase.h"
#include "cpixparsetools.h"

namespace lucene
{
    namespace search
    {
        class Query;
    }
}

namespace Cpix
{
    // forward declaration of interface
    class IHits;


    /**
     * This interface makes the unified search possible. A search will
     * always start with creating a query object with the query
     * parser, and the wildly different query types are abstracted by
     * this interface.
     *
     * Syntax for unified search:
     *
     * "$QRYTYPE<ARG1,...,ARGN>(QRY)", or
     * "$QRYTYPE(QRY)" = "$QRYTYPE<>(QRY)", or
     * "$QRYTYPE<ARG1,...,ARGN>" = "$QRYTYPE<ARG1,...ARGN>()", or
     * "$QRYTYPE" = "QRYTYPE<>()", or
     * "QRY", or
     * "*"
     *
     *   where:
     *
     *     o  "QRYTYPE" identifies the query type
     *
     *     o  ARG1, ..., ARGN are the arguments, if any
     *
     *     o  QRY is the query string itself.
     *
     *   If QRY is empty string, then the whole clause (with the
     *   delimiting parenthese) may be missing, like "$QRYTYPE<ARG1,...,ARGN>"
     *   = "$QRYTYPE<ARG1,...,ARGN>()".
     *
     *   If there are no arguments to pass, then the whole clause
     *   (with the delimiting lt and gt sings) is missing, like
     *   "$QRYTYPE(QRY)" = "$QRYTYPE<>(QRY)".
     *
     *   Obviously, both query string and arguments may be missing, in
     *   which case it may just look like "$QRYTYPE" = "$QRYTYPE<>()".
     *
     *   There are two special syntaxes:
     *
     *   If QRYTYPE is missing, then default, clucene search is meant.
     *
     *   A single '*' char means dumping all documents in the index
     *   database.
     *   
     * @todo rename to IQry/IQuery and move parseQuery into separate IQryFactory
     */
    class IQryType : public RefCountedBase
    {
    public:

        /**
         * Parses the query string and creates the appropriate query
         * type for it - factory method. May throw or relay error
         * messages through queryParser instance.
         *
         * Once a query type is identified by its keyword, an instance
         * of it is created. Then useClQueryParser, setArgs and
         * setQryStr methods are called, in this order. If all these
         * methods return successfully, the query type instance should
         * be fully set up to perform a search.
         *
         * @param queryParser the constructed (clucene) query parser
         * wrapper instance with the native (clucene) query
         * parser. The native instance may be directly used if plain
         * (clucene) query type is used, or just the field information
         * may be used of it, or nothing at all. That is, the clucene
         * query parser is NOT wrapped, it is just directly used (or
         * part of it, or nothing of it) during this unified
         * pre-parsing phase.
         *
         * @param qryStr the query string to parse
         *
         * @return the newly created query type - to bwe owned by the
         * client
         */
        static IQryType * parseQry(cpix_QueryParser * queryParser,
                                   const wchar_t    * qryStr);


        /**
         * Sets up this query type, after which it should be ready to
         * do the search.
         *
         * @param queryParser the clucene query parser wrapped - no
         * ownership is transferred. Never NULL.
         *
         * @param args the list of arguments that this query type
         * implentation should use during actual search. May be an
         * empty list. If empty argument list is not acceptable or if
         * there is some "type" mismatch, the query type must signal
         * error (either via queryParser or by throwin).
         *
         * @param qryStr the qry string itself, may be NULL or empty
         * string. If NULL or empty string is not acceptable, the
         * query type must signal error (either via queryParser or by
         * throwin).
         */
        virtual void setUp(cpix_QueryParser              * queryParser,
                           const std::list<std::wstring> & args,
                           const wchar_t                 * qryStr) = 0;


        /**
         * Performs the actual search on the given index
         * searcher. This method may throw or signal error condition
         * through the given idxSearcher instance.
         *
         * @param idxSearcher the searcher to use for searching.
         *
         * @return the hits (result of the search)
         */
        virtual cpix_Hits * search(cpix_IdxSearcher * idxSearcher) = 0;


        /**
         * Query types with the most basic semantics are allowed to
         * perform searches on
         */
        virtual cpix_Hits * search(cpix_IdxDb * idxDb) = 0;

        //
        // lifetime management
        //

        /**
         * Constructor
         */
        IQryType();


        /**
         * Destructor
         */
        virtual ~IQryType() = 0;


    private:


        // these two are declared to prevent value semantics
        IQryType(const IQryType &);
        IQryType & operator=(const IQryType &);
    };
    

    /**
     * @return true if the given query string starts with the given
     * character, not taking the blank/space characters into account.
     */
    bool StartsWith(const wchar_t * qryStr,
                    wchar_t         wc);


    extern const wchar_t CLUCENE_QRYTYPEID[];
    extern const wchar_t DUMP_QRYTYPEID[];

    extern const wchar_t UNIFIEDSEARCH_MARKER_CHAR;
    extern const wchar_t UNIFIEDSEARCH_MARKER_STR[];

    extern const wchar_t DUMPSEARCH_MARKER_CHAR;
    extern const wchar_t DUMPSEARCH_MARKER_STR[];

    /**
     * Exception throwing version of the previous
     */
    IHits * CLuceneSearchIdxExc(IdxDbHndl             handle,
                                lucene::search::Query * qry);

    /**
     * template paramater IDX can cpix_IdxDb or cpix_IdxSearcher.
     */
    template<typename IDX>
    cpix_Hits * CLuceneSearchIdx(IDX                   		* idx,
                                 lucene::search::Query 		* qry)
    {
        cpix_Hits
            * rv = NULL;
            
        using namespace lucene::search;
        using namespace Cpix;
         
        IHits
            * hits = XlateExc(idx,
                              CallFreeFunc(&CLuceneSearchIdxExc,
                                           idx->handle_, 
                                           qry));
        
        if (cpix_Succeeded(idx)) 
            {
                CreateWrapper(hits,
                              idx,
                              rv);
            }
        
        return rv;
        
    }



    /**
     * Invokes the Cpt::wconvert function (family), throwing on
     * failure. The message of the exception is the string returned by
     * the conversion function telling about the type of value/format
     * configuration that was used to convert the string.
     */
    template<typename V>
    void wconvertThrowing(V             * to,
                          const wchar_t * fromStr)
    {
        const char
            * result = Cpt::wconvert(to,
                                     fromStr);
        if (result != NULL)
            {
                THROW_CPIXEXC("Conversion from '%S' failed: %s",
                              fromStr,
                              result);
            }
    }

}


namespace Cpix
{
    // This class is an implementation detail, logically local to
    // iqrytype.cpp, but for testing purposes, it is here
    class QryCall
    {
    private:

        /**
         * This static member (tokenizer_) has const-usage
         * conceptually, except it is created on demand, so it cannot
         * be declared const. It needs no mutex protection.
         */
        static Cpt::Lex::MultiTokenizer  * tokenizer_;
        static Cpt::Lex::MultiTokenizer  & tokenizer();

        
        /**
         * This static member (transitions_) has const-usage
         * conceptually, except it is created on demand, so it cannot
         * be declared const. It needs no mutex protection.
         *
         * For the transition table definition, see comments for
         * parse().
         */ 
        typedef int State;
        typedef Cpt::Lex::token_type_t Symbol;
        typedef std::pair<State, Symbol> StateSymbolPair;
        typedef std::map<StateSymbolPair, State> TransitionTable;
        static TransitionTable  * transitions_;
        static TransitionTable & transitions();

        /**
         * This static member (finalStates_) has const-usage
         * conceptually, except it is created on demand, so it cannot
         * be declared const. It needs no mutex protection.
         *
         * For final states, see comments for parse().
         */ 
        typedef std::set<State> StateSet;
        static StateSet * finalStates_;
        static StateSet & finalStates();



    public:
        //
        // public operators
        //

        /**
         * The query type id string - never empty string
         */
        std::wstring                qryTypeId_;

        /**
         * The list of arguments, may be an emptry list
         */
        std::list<std::wstring>     args_;

        /**
         * The "inner" query string, if any, may be empty string.
         */
        std::wstring                innerQryStr_;

        /**
         * Constructs a QryCall instance, parsing the given string. It
         * assumes the syntax described in parse(), cf below. May
         * throw on parse failure.
         *
         * On successful parsing, qryTypeId_, args_ and innerQryStr_
         * members are initialized.
         */
        QryCall(const wchar_t * qryStr);


        /**
         * Initializes the QryCall parser internals (static
         * members). Not thread-safe, and it should not need to be, as
         * this is called from library init code. Not thread-safe and
         * it should not need to be, as it is called from library init
         * code.
         */
        static void init();


        /**
         * Releases static member variables.  Called from
         * shutdown. Not thread-safe, and it should not need to be.
         */
        static void releaseCache(); 

    private:
        //
        // private implementation details
        //
        void setQryTypeId(const wchar_t * tokenText);
        void setQryTypeId(const Cpt::Lex::Token & token);
        
        void addNextArg(const Cpt::Lex::Token & token);
        
        void setInnerQryStr(const wchar_t     * innerQryStrBegin,
                            const wchar_t     * innerQryStrEnd);
        void setInnerQryStr(const Cpt::Lex::Token   & token,
                            const wchar_t           * qryStr);


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
        void parse(const wchar_t * qryStr);
    };

}


#endif
