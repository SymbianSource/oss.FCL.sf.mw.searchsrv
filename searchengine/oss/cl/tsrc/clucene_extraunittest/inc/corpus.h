/*------------------------------------------------------------------------------
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#ifndef CPIX_PERFMETRICS_CORPUS_H_
#define CPIX_PERFMETRICS_CORPUS_H_

#include <string>
#include <iosfwd>
#include <iterator>
#include <fstream>

class Corpus;


/**
 * This class defines an input iterator yielding strings (one line at
 * a time) from a corpus instance.
 */
class CorpusInputIt : public std::iterator<std::input_iterator_tag,
                                           std::string>
{
private:
    //
    // private members
    //
    Corpus       *  corpus_;
    std::string     curLine_;

public:
    //
    // public operators
    //

    /**
     * Copy operator
     */
    CorpusInputIt & operator=(const CorpusInputIt & that);
    

    //
    // Input iterator operators
    //
    CorpusInputIt &operator++();
    CorpusInputIt  operator++(int);

    reference operator*();

    //
    // public lifetime management
    //

    /**
     * Default constructor
     */
    CorpusInputIt();

    
    /**
     * Copy constructor
     */
    CorpusInputIt(const CorpusInputIt & that);


    
private:
    //
    // private methods
    //
    friend class Corpus;
    friend bool operator==(const CorpusInputIt &,
                           const CorpusInputIt &);


    /**
     * Constructor for real use.
     */
    CorpusInputIt(Corpus * owner);
};



/**
 * Comparision functions.
 */
bool operator==(const CorpusInputIt & left,
                const CorpusInputIt & right);
bool operator!=(const CorpusInputIt & left,
                const CorpusInputIt & right);



/**
 * This class provides an input iterator that iterates lines from the
 * corpus.
 */
class Corpus
{
private:
    //
    // private members
    //
    const std::string        filePath_;
    std::ifstream            ifs_;

    std::string              curLine_;
    bool                     hasMore_;
    
    // This class does not have value semantics
    Corpus(const Corpus &);
    Corpus & operator==(const Corpus &);

public:
    //
    // public operations
    //
    /**
     * Returns an iterator instance that will start reading lines from
     * the corpus from the current point.
     */
    CorpusInputIt begin();

    /**
     * Returns an iterator pointint to past-the-last one.
     */
    CorpusInputIt end();
    

    //
    // lifetime management
    //
    /**
     * Constructs a corpus instance reading from a text file.
     */
    Corpus(const char * filePath);

    
private:
    //
    // private methods
    //
    friend class CorpusInputIt;

    bool hasMore() const;
    std::string next();


    void readNext();
};

#endif
