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
#ifndef CPIX_PERFMETRICS_CORPUSTESTER_H_
#define CPIX_PERFMETRICS_CORPUSTESTER_H_

#include <string>
#include <vector>

#include "itk.h"

#include "idxutil.h"

#include "corpus.h"


class CorpusTester : public Cpt::IFileVisitor
{
private:
    //
    // private members
    //
    Corpus                   corpus_;
    int                      numOfPreIndexedItems_;
    const std::string        runName_;
    int                      bufferSize_;
    int                      maxInputSize_;
    int                      lumpSize_;
    int                      numOfFirstItems_;
    int                      numOfLastItems_;
    std::list<std::string>   queryTerms_;
    int                      numOfSearches_;
    int                      numOfIncrementalSearches_;
    int                      minIncrementalLength_;
    int                      numOfSuggestionSearches_;
    int                      minSuggestionLength_;

    // should take snapshot of index database or not 
    bool                     takeIdxSnapshots_;
    
    // if numOfLasts_ > 0 and addLasts_, then it performs adding lasts
    bool                     addLasts_;

    // if numOfLasts_ > 0 and delReAddLasts_, then it performs deletion
    // and re-adding lasts
    bool                     delReAddLasts_;

    // if numOfLasts_ > 0 and updateLasts_, then it performs updating
    // lasts
    bool                     updateLasts_;

    Itk::TestMgr         *   testMgr_;
    IdxUtil              *   idxUtil_;
    CorpusInputIt            it_;
    int                      processed_;

    int                      indexSize_; // size of the index
public:
    //
    // public operators
    //
    /**
     * @param corpus the corpus to read lines from
     *
     * @param numOfPreIndexedItems the number of items to put into the
     * index before making any actual measurements. Negative value
     * means that this many items are read from the corpus, but
     * discarded - useful when indexing to an existing index database
     * from a certain point in the corpus.
     *
     * @param runName the name of the run that will contribute to
     * measurement definition variable names
     *
     * @param maxInputSize - read at most this many input lines from
     * corpus (after the pre-indexing, that is, altogether
     * numOfPreIndexedItems and maxInputSize number of items will be
     * processed).
     *
     * @param lumpSize - the test run performs its run in loops, the
     * size of an input sequence that is handled together is defined
     * with this
     *
     * @param numOfFirstItems - the first N items in a lump will have
     * their indexing measured. May be zero, in which case no
     * measurement is done. It can also be negative, in which case
     * that many items are read from the corpus, and internal counters
     * show as if they were added to the index, but they are actually
     * discarded. Comes handy when one wants to create a test vector
     * where one wants to search but perform no write/delete operation
     * on an existing index.
     *
     * @param numOfLastItems - the last N items of a lump are added
     * (not measured), then removed (measured), then re-added
     * (measured). May be zero in which case no removal / re-adding /
     * measuring is done.
     *
     * @param queryTerms a NULL-terminated array of pointers to C-strings
     * that define the terms to search for (and measure)
     *
     * @param numOfSearches the number of times a term from the
     * queryTerm set is searched repeatedly (measured)
     *
     * @param numOfIteraticeSearches the number of times a term from
     * the queryTerm set is searched for in an incremental manner
     * repeateadly (measured).
     *
     * @param minIncrementalLength is the minimal length of prefix
     * from a queryTerm to form a wildcard search.
     *
     * @param takeIdxSnapshot if true, then a snapshot of the index is
     * copied to a supdirectory (named with the running number of the
     * lump) at the end of every lump.
     *
     * @param addLasts if true, lasts are added (provided numOfLasts
     * is positive).
     *
     * @param delReAddLasts if true, lasts are deleted and re-added
     * (provided numOfLasts is positive)
     *
     * @param updateLasts if true, lasts are updated (provided
     * numOfLasts is positive)
     */
    CorpusTester(const char                   * corpusPath,
                 int                            numOfPreIndexedItems,
                 const std::string            & runName,
                 int                            bufferSize,
                 int                            maxInputSize,
                 int                            lumpSize,
                 int                            numOfFirstItems,
                 int                            numOfLastItems,
                 const std::list<std::string> & queryTerms,
                 int                            numOfSearches,
                 int                            numOfIncrementalSearches,
                 int                            minIncrementalLength,
                 int                            numOfSuggestionSearches,
                 int                            minSuggestionLength,
                 bool                           takeIdxSnapshot,
                 bool                           addLasts,
                 bool                           delReAddLasts,
                 bool                           updateLasts);

    void run(Itk::TestMgr * testMgr,
             IdxUtil      * idxUtil);

public:
    // from Cpt::IFileVisitor 
    virtual bool visitFile(const char * path);
    virtual DirVisitResult visitDirPre(const char * path);
    virtual bool visitDirPost(const char * path);

private:
    //
    // private methods
    //

    /**
     * Indexes a number of docs (numOfPreindexedItems) before
     * embarking on actual tasks and measurements.
     */
    void preIndex();

    /**
     * The main loop for measurements - invokes as many "lump"
     * operations as necessary to reach maxInputSize_ number of
     * documents processed altogether.
     */
    void runLoop();

    /**
     * Running a lump means processing lumpoSize number of items. Of
     * these, numOfFirstItems are indexed (and measured), then some
     * more items are indexed (unmeasured), finally numOfLastItems
     * number of items are indexed (measured), deleted (measured) and
     * re-added (measured). Finally, searches (terms in queryTerms for
     * numOfSearches times) and incremental searches are performed
     * (terms in queryTerms for numOfIncrementalSearches times) are
     * performed, both of them are measured.
     */
    void runLump(const char * lumpName);

    //
    // these methods implement different tasks for a lump
    //
    void addFirstItems(const char * lumpName);
    void addMiddleItems();
    void addLastItems(const char                * lumpName,
                      std::vector<std::string>  & lastItems);
    void deleteLastItems(const char * lumpName);
    void reAddLastItems(const char                      * lumpName,
                        const std::vector<std::string>  & lastItems);
    void updateLastItems(const char                      * lumpName,
                         const std::vector<std::string>  & lastItems);
    void flushLastItems(const char                      * lumpName);
    void search(const char * lumpName);
    void incrementalSearch(const char * lumpName);
    void suggestSearch(const char * lumpName);

    // used by search and incrementalSearch
    void doQuery(const char * metricName,
                 cpix_Query * query);
    void doSuggestQuery(const char    * metricName,
                        // OBS const wchar_t * fieldName,
                        // OBS const wchar_t * wildcard);
                        cpix_Query    * termsQry);


    void takeIdxSnapshot();
};


#endif
