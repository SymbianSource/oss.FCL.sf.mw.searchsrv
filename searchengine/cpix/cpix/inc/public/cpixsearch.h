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

#ifndef CPIX_CPIXSEARCH_H
#define CPIX_CPIXSEARCH_H


#include "cpixinit.h"
#include "cpixerror.h"
#include "cpixdoc.h"
#include "cpixanalyzer.h"


#ifdef __cplusplus
extern "C" {
#endif



    /**
     ** Internal (implementation detail) type, not useful in
     ** the public API.
     */                 
    typedef unsigned int IdxDbHndl;
    


    /************************************************************************
     * Class cpix_Query
     */
    struct cpix_Query_
    {
        // pointer to native (CLucene) impl
        void       * ptr_;
        
        // Last error, if any, that resulted from executing the
        // last operation on this cpix_XXX object
        // Use macros cpix_Failed, cpix_Succeeded and cpix_ClearError.
        // Do not attempt releasing it.
        cpix_Error * err_;
    };
    typedef cpix_Query_ cpix_Query;

    /**
     * Destroys this query instance. Never fails.
     */
    void cpix_Query_destroy(cpix_Query * thisQuery);
    


    /************************************************************************
     * Interface cpix_BoostMap
     */
    struct cpix_BoostMap_
    {
        // pointer to native (CLucene) impl
        void       * ptr_;
        
        // Last error, if any, that resulted from executing the
        // last operation on this cpix_XXX object
        // Use macros cpix_Failed, cpix_Succeeded and cpix_ClearError.
        // Do not attempt releasing it.
        cpix_Error * err_;
    };
    typedef cpix_BoostMap_ cpix_BoostMap;


    /**
     * Creates a boostmap (field name (wchar_t*) -> float values map).
     *
     * A boost map is a map to associated float values to a field
     * name. Such associated value is used to boost the weight of a
     * hit based on what field the term was found.
     *
     * @param result conveys error messages that may have arisen
     */
    cpix_BoostMap *
    cpix_BoostMap_create(cpix_Result * result);
    

    /**
     * Puts a new fieldName - value pair to this boost map.
     *
     * @param thisMap this boost map to put the new associated to
     *
     * @param fieldName the name of the field
     */
    void cpix_BoostMap_put(cpix_BoostMap * thisMap,
                           const wchar_t * fieldName,
                           float           boost);



    /**
     * Destroys this boost map instance.
     *
     * @param thisMap this boost map instance to destroy
     */ 
    void cpix_BoostMap_destroy(cpix_BoostMap * thisMap);



    /************************************************************************
     * Interface cpix_QueryParser
     */
    struct cpix_QueryParser_
    {
        // pointer to native (CLucene) impl
        void       * ptr_;
        
        // Last error, if any, that resulted from executing the
        // last operation on this cpix_XXX object
        // Use macros cpix_Failed, cpix_Succeeded and cpix_ClearError.
        // Do not attempt releasing it.
        cpix_Error * err_;
    };
    typedef cpix_QueryParser_ cpix_QueryParser;

    /**
     * Constructs a cpix_QueryParser instance.
     *
     * @param fieldName the name of the field to search, must not be
     *        NULL, ownership not transferred.
     *
     * @param analyzer the analyzer to use, must not be NULL,
     *        ownership not transferred.
     *
     * @return new instance of query parser, ownership is transferred
     * to the called. NULL is returned if creation failed for some
     * reason - details are not given.
     */
    cpix_QueryParser * 
    cpix_QueryParser_create(cpix_Result   * result,
                            const wchar_t * fieldName,
                            cpix_Analyzer * analyzer);
    
    
    cpix_QueryParser * 
    cpix_CreatePrefixQueryParser(cpix_Result   * result, 
								 const wchar_t * fieldName); 

    /**
     * Constructs a special type of query parser, a multi-field query
     * parser. For instance, specifying two fiels: 'title' and 'body'
     * and giving a query string with two terms 'term1' and 'term2'
     * will get you a query:
     *
     * '(title:term1 body:term1) (title:term2 body:term2)'
     *
     * Setting the default operator to AND will result in:
     *
     * '+(title:term1 body:term1) +(title:term2 body:term2)'
     *
     * meaning that all the terms must appear but it does not matter
     * in what field they appear.
     *
     * @param result conveys error results
     *
     * @param fieldNames an array of field names to use, must not be
     * NULL, and items in the array must not be NULL either.
     *
     * @param fieldCount the number of items in fieldNames
     *
     * @param analyzer the analyzer to use. Must not be NULL, no ownership
     * transferred.
     *
     * @param boosts the weights for the different fields. There MUST
     * be as many items in boosts many fields there are, and the
     * boosting weights correspond to the fields of the same index
     * position in the arrays. MUST NOT be NULL.
     *
     * @return a query parser that can do the special query
     */
    cpix_QueryParser *
    cpix_CreateMultiFieldQueryParser(cpix_Result   * result,
                                     const wchar_t * fieldNames[],
                                     cpix_Analyzer * analyzer,
                                     cpix_BoostMap * boosts);

    /**
     * The default operator on a query parser - how to interpret
     * enumerations of terms - are they OR-ed or AND-ed?
     */
    enum cpix_QP_Operator_
        {
            cpix_QP_OR_OPERATOR  = 0,
            cpix_QP_AND_OPERATOR = 1
        };
    typedef cpix_QP_Operator_ cpix_QP_Operator;

    
    /**
     * Sets the default operator on this query parser. The default
     * operator (OR or AND) can be left out from query
     * strings. E.g. if default operator is OR then query string "foo
     * bar" means "foo OR bar".
     *
     * @param thisQueryParser the query parser to set the default
     * operator of
     *
     * @param op the operator to set as default operator to use.
     */
    void 
    cpix_QueryParser_setDefaultOperator(cpix_QueryParser * thisQueryParser,
                                        cpix_QP_Operator   op);


    // TODO declare and implement
    // cpix_QueryParser_getDefaultOperator() - when/if needed


    /**
     * Parses the query string and creates a query object
     * out of it.
     *
     * @param thisQueryParser the query parser that should
     *        parse
     *
     * @param queryStr the query string to parse, must not be NULL,
     *        ownership not transferred. A single wildcard character
     *        will create a query that will dump the actual content of
     *        the index, so be careful (in this case the name of the
     *        field given to this query parser does not matter).
     *
     * @return NULL or a new Query object, ownership is transferred
     * to the owner. NULL is returned if the operation failed or
     * if the operation succeeded, but the query was considered 'useless'. 
     * Query e.g. may be reduced to empty string by the stop word filter. 
     * In this situation in any case, searching with the query would produce
     * zero hits.
     */
    cpix_Query * 
    cpix_QueryParser_parse(cpix_QueryParser * thisQueryParser,
                           const wchar_t    * queryStr);
    
    /**
     * Destroys this query parser instance. Never fails.
     */
    void cpix_QueryParser_destroy(cpix_QueryParser * thisQueryParser);
    

    /************************************************************************
     * Class cpix_Hits
     */
    struct cpix_Hits_
    {
        // pointer to native (CLucene) impl
        void       * ptr_;
        
        // Last error, if any, that resulted from executing the
        // last operation on this cpix_XXX object
        // Use macros cpix_Failed, cpix_Succeeded and cpix_ClearError.
        // Do not attempt releasing it.
        cpix_Error * err_;
    };
    
    typedef cpix_Hits_ cpix_Hits;
    
    /**
     * @return the total number of available hits in this set -
     * snapshot taken at the time of the hits construction.
     *
     * NOTE While fetching documents may force recommitting the query
     * if the underlying database have changed, asking for its length
     * will never recommit.
     */
    int32_t cpix_Hits_length(cpix_Hits * thisHits);


    /**
     * Gets the nth document in this set. May fail.
     *
     * @param thisHits this Hits instance to use
     *
     * @param index the index of the document to get.
     *
     * @param target to put the field to. NOTE no new native instance
     * is created for the target, and therefore cpix_Document_destroy
     * must NOT be called on it. The instance 'target' is best to have
     * as a local variable (on the stack). See description of lifetime
     * management in the beginning of this header. May fail.
     */
    void cpix_Hits_doc(cpix_Hits     * thisHits,
                       int32_t         index,
                       cpix_Document ** target,
                       int32_t         count);



    /** OBS
     * A hits collection may be invalidated whenever the underlying
     * index manipulator changes substantially. That means, that
     * cpix_Hits_length and cpix_Hits_doc may fail at any point for no
     * apparent reasons. This function can test if a hits collection
     * is still valid. Note, that even if it is valid, it may not be
     * by the time you call your next operation on it. If it is
     * invalid, then there is only one thing you can do with it: call
     * cpix_Hits_destroy on it.
     *
     *
     * @param thisHits this hits collection
     *
     * @returns true (1) if it is still valid, and false (0)
     * otherwise. This call never fails.
     * /
    int cpix_Hits_isValid(cpix_Hits * thisHits);
    */


    /**
     * Destroys this hits instance. Never fails.
     *
     */
    void cpix_Hits_destroy(cpix_Hits * thisHits);

    /**
      * cpix_IdxDb
      */
     struct cpix_IdxSearcher_
     {
         // instead of a pointer to native (CLucene) impl, there is a
         // handle here only. The type here must be equivalent to 
         IdxDbHndl    handle_;
         
         // Last error, if any, that resulted from executing the
         // last operation on this cpix_XXX object
         // Use macros cpix_Failed, cpix_Succeeded and cpix_ClearError.
         // Do not attempt releasing it.
         cpix_Error * err_;
     };
     typedef cpix_IdxSearcher_ cpix_IdxSearcher;
     



     /**
      * Creates a cpix_IdxDb instance through which a client can search
      * the index database responsible for a given base app
      * class. 
      * Search can involve multiple on disk index databases.
      *
      * @param result pointer through which error details are
      * communicated back
      *
      * @param domainSelector a string that is non-empty, a comma
      * (',') separated list of appclasses. Appclasses can be
      * (unqualified) base app classes too, and qualified base app
      * classes as well.
      *
      * @retur pointer to the cpix_IdxDb instance that can be used for
      * searching and modifying the index.
      */
     cpix_IdxSearcher * cpix_IdxSearcher_openDb(cpix_Result * result,
                                                const char  * domainSelector);


     /**
      * Releases the native index database manipulator (from the caller
      * client). (Actually, it only decreases its reference count, as
      * index database manipulator is kept around for performance as
      * long as it is needed.) It also frees up the wrapper.
      */
     void cpix_IdxSearcher_releaseDb(cpix_IdxSearcher * thisIdxSearcher);
     

     /**
      * Searches the index.
      *
      * NOTE: Keep the original query parser and query instances alive
      * as long as you use the result of the search - in general, they
      * may be needed.
      *
      * @param thisIdxDb the index db instance that should
      *        perform the search, must not be NULL.
      *
      * @param query the query itself in a structured form, must not be
      *        NULL, its ownership does not change.
      *
      * @return the hits instance with the search results, that will be
      * owned by the calling client.
      */
     cpix_Hits * 
     cpix_IdxSearcher_search(cpix_IdxSearcher * thisIdxSearcher,
                             cpix_Query       * query);
     
    



#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */



#endif /* CPIX_CPIXSEARCH_H */
