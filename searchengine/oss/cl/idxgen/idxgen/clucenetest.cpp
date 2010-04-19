/*------------------------------------------------------------------------------
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

// INCLUDE FILES
#include <CLucene.h>
#include <Clucene/util/Reader.h>

#include <CLucene/store/RAMDirectory.h>

#include <iostream>
#include <fstream>
#include <exception>

#include <dirent.h>

#include <stdio.h>


#include "corpus.h"
#include "cpixstrtools.h"
#include "cpixfstools.h"
#include "indevicecfg.h"

using namespace std; 

using namespace lucene; 
using namespace lucene::index;
using namespace lucene::document;
using namespace lucene::analysis;
using namespace lucene::search;
using namespace lucene::util;
using namespace lucene::queryParser;
using namespace lucene::store;

/*
 * included from indevicecfg.h
#define CPIX_DOCUID_FIELD    "_docuid"
#define LCPIX_DOCUID_FIELD    L"_docuid"
#define CPIX_APPCLASS_FIELD  "_appclass"
#define LCPIX_APPCLASS_FIELD  L"_appclass"
#define CPIXEXCERPT_FIELD   "_excerpt"
#define LCPIX_EXCERPT_FIELD   L"_excerpt"
#define CPIX_MIMETYPE_FIELD     "_mimetype"
#define LCPIX_MIMETYPE_FIELD     L"_mimetype"
#define CPIX_DEFAULT_FIELD   "_aggregate"
#define LCPIX_DEFAULT_FIELD   L"_aggregate"


#define LFILE_BASEAPPCLASS  L"root file"
#define CONTENTS_FIELD      L"Contents"
*/

#define DECL_WSTR_FROM_STR(w,s) size_t size##w = strlen(s) + 1; Cpt::auto_array<wchar_t> w(new wchar_t[size##w]); w.get()[size##w - 1] = 0; mbstowcs(w.get(), s, size##w)

#define DECL_STR_FROM_WSTR(s,w) size_t size##s = wcslen(w) + 1; Cpt::auto_array<char> s(new char[size##s]); s.get()[size##s - 1] = 0; wcstombs(s.get(), w, size##s);

void initializeDoc(Document      * doc,
                   const wchar_t * docUid,
                   const wchar_t * appClass,
                   const wchar_t * excerpt,
                   const wchar_t * aggregate)
{
    doc->add(*new Field(LCPIX_DOCUID_FIELD,
                        docUid,
                        Field::STORE_YES | Field::INDEX_UNTOKENIZED));

    {
        DECL_STR_FROM_WSTR(cDocUidField, LCPIX_DOCUID_FIELD);
        DECL_STR_FROM_WSTR(cDocUid, docUid);
        /*
        printf(" ### initializing document: %s = %s\n",
               cDocUidField.get(),
               cDocUid.get());
        */
    }

    doc->add(*new Field(LCPIX_APPCLASS_FIELD,
                        appClass,
                        Field::STORE_YES | Field::INDEX_UNTOKENIZED));
    
    if (excerpt != NULL)
	{
		doc->add(*new Field(LCPIX_EXCERPT_FIELD,
							excerpt,
							Field::STORE_YES | Field::INDEX_NO));
	}
    
    doc->add(* new Field(LCPIX_DEFAULT_FIELD,
                         aggregate,
                         Field::STORE_NO | Field::INDEX_TOKENIZED));

    {
        DECL_STR_FROM_WSTR(cDefaultField, LCPIX_DEFAULT_FIELD);
        DECL_STR_FROM_WSTR(cDefault, aggregate);
        /*
        printf(" ### initializing document: %s = %s\n",
               cDefaultField.get(),
               cDefault.get());
        */
    }
}


class IIndexer
{
public:
    virtual void index(IndexWriter & idxWriter,
                       size_t        id,
                       const char  * line,
                       Analyzer    * analyzer) = 0;
    virtual ~IIndexer() { ; }
};


class FileIndexer : public IIndexer
{
public:

    void indexFileDoc(IndexWriter   & idxWriter,
                      const wchar_t * docUid,
                      const wchar_t * excerpt,
                      const wchar_t * content,
                      const wchar_t * aggregate,
                      Analyzer      * analyzer)
    {
        std::auto_ptr<Document>
            doc(new Document());

        initializeDoc(doc.get(),
                      docUid,
                      LFILE_BASEAPPCLASS,
                      excerpt,
                      aggregate);

        doc->add(* new Field(CONTENTS_FIELD,
                             content,
                             Field::STORE_NO | Field::INDEX_TOKENIZED));
        idxWriter.addDocument(doc.get());
        doc.reset();
    }


    virtual void index(IndexWriter & idxWriter,
                       size_t        id,
                       const char  * line,
                       Analyzer    * analyzer)
    {
        DECL_WSTR_FROM_STR(wline, line);

        char
            docUid[32];
        memset(docUid,
               0,
               sizeof(docUid));

        snprintf(docUid,
                 sizeof(docUid) / sizeof(char),
                 "%08d",
                 id);

        DECL_WSTR_FROM_STR(wDocUid, docUid);

        /* OBS
        printf("#FILE excerpt / aggregate: %s, %s\n",
               line,
               line);
        */

        indexFileDoc(idxWriter,
                     wDocUid.get(),
                     wline.get(),
                     wline.get(),
                     wline.get(),
                     analyzer);
    }

};


struct FieldDesc {
    const wchar_t * name_;
    int             flags_;
};


/*******************************************************************
 *
 * POI
 *
 */


const FieldDesc PoiFields[] = {
    { L"gpslong",
      Field::STORE_NO | Field::INDEX_UNTOKENIZED
    },

    { L"gpslat",
      Field::STORE_NO | Field::INDEX_UNTOKENIZED
    },

    { L"whatnot",
      Field::STORE_NO | Field::INDEX_UNTOKENIZED
    },

    { L"id-und",
      Field::STORE_NO | Field::INDEX_UNTOKENIZED
    },
    
    { L"field01",
      Field::STORE_NO | Field::INDEX_TOKENIZED
    },

    { L"field02",
      Field::STORE_NO | Field::INDEX_TOKENIZED
    },

    { L"field03",
      Field::STORE_NO | Field::INDEX_TOKENIZED
    },

    { L"field04",
      Field::STORE_NO | Field::INDEX_TOKENIZED
    },

    { L"field05",
      Field::STORE_NO | Field::INDEX_TOKENIZED
    },

    { L"field06",
      Field::STORE_NO | Field::INDEX_TOKENIZED
    },

    { L"field07",
      Field::STORE_NO | Field::INDEX_TOKENIZED
    },

    { L"field08",
      Field::STORE_NO | Field::INDEX_TOKENIZED
    },

    { L"field09",
      Field::STORE_NO | Field::INDEX_TOKENIZED
    },

    { L"field10",
      Field::STORE_NO | Field::INDEX_TOKENIZED
    },

    { L"field11",
      Field::STORE_NO | Field::INDEX_TOKENIZED
    },

    { L"field12",
      Field::STORE_NO | Field::INDEX_TOKENIZED
    },

};

#define POI_FIELD_NUM sizeof(PoiFields) / sizeof(FieldDesc)
#define POI_APPCLASS L"root maps poi"

#define BLANKS " \t\r\n"


/*******************************************************************
 *
 * CSV indexer
 *
 */


class CsvIndexer : public IIndexer
{
private:
    const FieldDesc *  fieldDescs_;
    size_t             fieldNum_;
    char               fieldDelimiter_;
    const wchar_t   *  baseAppClass_;


public:
    
    /**
     * @param baseAppClass : ownership is not transferred, this
     * instance will only refer to the string given.
     */
    CsvIndexer(const FieldDesc * fieldDescs,
               size_t            fieldNum,
               char              fieldDelimiter,
               const wchar_t   * baseAppClass)
        : fieldDescs_(fieldDescs),
          fieldNum_(fieldNum),
          fieldDelimiter_(fieldDelimiter),
          baseAppClass_(baseAppClass)
    {
        ;
    }


    void CsvIndexer::getFields(const char              * fieldsLine,
                               std::list<std::string>  & fields)
    {
        /* OBS
        printf(" #### line: %s\n",
               fieldsLine);
        */

        fields.clear();

        // scan the whole fieldsLine
        for (const char * p = fieldsLine;
             *p != 0;
             ++p)
            {
                const char
                    * fieldStart = p;
            
                // scan for next field
                // OBS while (*p != 0 && *p != '|')
                while (*p != 0 && *p != fieldDelimiter_)
                    {
                        if (*p == '=')
                            {
                                fieldStart = p + 1;
                            }

                        ++p;
                    }

            
                // if a non-null field is found, widen it and push it to
                // the list
                if (p != fieldStart)
                    {
                        std::string
                            tmp(fieldStart,
                                p - fieldStart);

                        size_t
                            pos = tmp.find_first_not_of(BLANKS);
                        if (pos == std::string::npos)
                            {
                                continue;
                            }
                        if (pos != 0)
                            {
                                tmp = tmp.substr(pos);
                            }
                        pos = tmp.find_last_not_of(BLANKS);
                        if (pos != 0)
                            {
                                tmp = tmp.substr(0,
                                                 pos + 1);
                            }

                        /* 
                        printf(" ### field: %s\n",
                               tmp.c_str());
                        */

                        fields.push_back(tmp);
                    }

                if (*p == 0)
                    {
                        break;
                    }
            }
    }


    virtual void index(IndexWriter & idxWriter,
                       size_t        id,
                       const char  * line,
                       Analyzer    * analyzer)
    {
        std::list<std::string>
            fieldValues;

        getFields(line,
                  fieldValues);

        std::list<std::string>::const_iterator
            i = fieldValues.begin(),
            end = fieldValues.end();

        std::string
            aggregate,
            excerpt;
    
        // OBS for (size_t f = 0; i != end && f < POI_FIELD_NUM; ++i, ++f)
        for (size_t f = 0; i != end && f < fieldNum_; ++i, ++f)
            {
                if (i->length() > 0)
                    {
                        aggregate += i->c_str();
                        aggregate += ' ';
                        if (f >= 4)
                            {
                                excerpt += i->c_str();
                                excerpt += ' ';
                            }
                    }
            }
    
        char
            docUid[32];
        memset(docUid,
               0,
               sizeof(docUid));

        snprintf(docUid,
                 sizeof(docUid) / sizeof(char),
                 "%08d",
                 id);

        std::auto_ptr<Document>
            doc(new Document());

        /*
        printf("#FILE excerpt / aggregate: %s, %s\n",
               excerpt.c_str(),
               aggregate.c_str());
        */

        DECL_WSTR_FROM_STR(wDocUid, docUid);
        DECL_WSTR_FROM_STR(wExcerpt, excerpt.c_str());
        DECL_WSTR_FROM_STR(wAggregate, aggregate.c_str());

        initializeDoc(doc.get(),
                      wDocUid.get(),
                      // POI_APPCLASS,
                      baseAppClass_,
                      wExcerpt.get(),
                      wAggregate.get());

        i = fieldValues.begin();
        // OBS for (size_t f = 0; i != end && f < POI_FIELD_NUM; ++i, ++f)
        for (size_t f = 0; i != end && f < fieldNum_; ++i, ++f)
            {
                DECL_WSTR_FROM_STR(wField, i->c_str());

                /* OBS
                doc->add(* new Field(PoiFields[f].name_,
                                     wField.get(),
                                     PoiFields[f].flags_));
                */
                doc->add(* new Field(fieldDescs_[f].name_,
                                     wField.get(),
                                     fieldDescs_[f].flags_));


                {
                    // OBS DECL_STR_FROM_WSTR(cField, PoiFields[f].name_);
                    DECL_STR_FROM_WSTR(cField, fieldDescs_[f].name_);
                    DECL_STR_FROM_WSTR(cValue, wField.get());
                    /*
                    printf(" ### setting field: %s = %s\n",
                           cField.get(),
                           cValue.get());
                    */
                }
            }

        idxWriter.addDocument(doc.get());
        doc.reset();
    }
};



/*******************************************************************
 *
 * Address (Streets)
 *
 */
const FieldDesc StreetFields[] = {
    { L"street",
      Field::STORE_NO | Field::INDEX_TOKENIZED
    },

    { L"zip",
      Field::STORE_NO | Field::INDEX_UNTOKENIZED
    },

    { L"whatnot1",
      Field::STORE_NO | Field::INDEX_UNTOKENIZED
    },

    { L"city",
      Field::STORE_NO | Field::INDEX_TOKENIZED
    },

    { L"whatnot2",
      Field::STORE_NO | Field::INDEX_UNTOKENIZED
    },

    { L"countrycode",
      Field::STORE_NO | Field::INDEX_TOKENIZED
    },

    { L"citycountry",
      Field::STORE_NO | Field::INDEX_TOKENIZED
    },

    { L"whatnot3",
      Field::STORE_NO | Field::INDEX_UNTOKENIZED
    },

    { L"gps",
      Field::STORE_NO | Field::INDEX_UNTOKENIZED
    },

    { L"whatnot4",
      Field::STORE_NO | Field::INDEX_UNTOKENIZED
    },

};

#define STREET_FIELD_NUM sizeof(StreetFields) / sizeof(FieldDesc)
#define STREET_APPCLASS L"root maps street"




/*******************************************************************
 *
 * IdxGenerator
 *
 */
class IdxGenerator
{
private:
    Corpus       corpus_;
    std::string  indexPath_;
    std::string  snapshotBaseDir_;
    size_t       numOfPreIndexedItems_;
    size_t       maxInputSize_;
    size_t       lumpSize_;

    IIndexer   & indexer_;

    std::string  idxToContinue_;


    IndexWriter    * idxWriter_;
    SimpleAnalyzer   analyzer_;

    CorpusInputIt    it_;
    size_t           idx_;

public:
    IdxGenerator(const char * corpusPath,
                 const char * indexPath,
                 const char * snapshotBaseDir,
                 size_t       numOfPreIndexedItems,
                 size_t       maxInputSize,
                 size_t       lumpSize,
                 IIndexer   & indexer,
                 const char * idxToContinue)
        : corpus_(corpusPath),
          indexPath_(indexPath),
          snapshotBaseDir_(snapshotBaseDir),
          numOfPreIndexedItems_(numOfPreIndexedItems),
          maxInputSize_(maxInputSize),
          lumpSize_(lumpSize),
          indexer_(indexer),
          idxToContinue_(idxToContinue == NULL ? "" : idxToContinue),
          idxWriter_(NULL),
          idx_(0)
    {
        ;
    }

    
    void run()
    {
        printf("Generating idx / snapshots\n");
        printf("  o corpus %s\n",
               corpus_.filePath());
        printf("  o idx %s\n",
               indexPath_.c_str());
        printf("  o to -> %s\n",
               snapshotBaseDir_.c_str());

        printf("  o pre: %d, max: %d, lump: %d\n",
               numOfPreIndexedItems_,
               maxInputSize_,
               lumpSize_);

        bool
            create = numOfPreIndexedItems_ >= 0;

        idxWriter_ = new IndexWriter(indexPath_.c_str(),
                                     &analyzer_,
                                     create);
        
        runLoop();

        idxWriter_->close();
        delete idxWriter_;
        idxWriter_ = NULL;
    }


    void runLoop()
    {
        it_ = corpus_.begin();

        preIndex();

        idx_ = 0;

        while (idx_ < maxInputSize_ && it_ != corpus_.end())
            {
                runLump();
            }
    }


    void preIndex()
    {
        bool
            skip = false;
        int
            count = numOfPreIndexedItems_;
        if (count < 0)
            {
                count = -count;
                skip = true;
            }
        CorpusInputIt
            end = corpus_.end();

        for (; count > 0 && it_ != end; ++it_, --count, ++idx_)
            {
                std::string
                    line = *it_;
                
                if (!skip)
                    {
                        indexer_.index(*idxWriter_,
                                       idx_,
                                       line.c_str(),
                                       &analyzer_);
                    }
            }
    }    



    void runLump()
    {
        printf ("DOING LUMP %d\n",
                idx_ / lumpSize_);

        CorpusInputIt
            end = corpus_.end();
        int
            count = lumpSize_;
        
        for (; 
             it_ != end && idx_ < maxInputSize_ && count > 0; 
             --count, ++it_, ++idx_)
            {
                std::string
                    line = *it_;
                
                indexer_.index(*idxWriter_,
                               idx_,
                               line.c_str(),
                               &analyzer_);
            }

        continueIdx();

        idxWriter_->optimize();
        idxWriter_->close();
        delete idxWriter_;
        idxWriter_ = NULL;

        takeSnapshot();

        idxWriter_ = new IndexWriter(indexPath_.c_str(),
                                     &analyzer_,
                                     false);
    }


    void continueIdx()
    {
        static bool
            add = true;

        if (add && idxToContinue_.length() > 0)
            {
                using namespace lucene::store;

                printf("Trying to continue (add) existing idx %s ...\n",
                       idxToContinue_.c_str());
                    
                FSDirectory
                    * dirToAdd = FSDirectory::getDirectory(idxToContinue_.c_str(),
                                                           false); // dont'create
                    
                if (dirToAdd != NULL)
                    {
                        Directory
                            * dirsToAdd[] = {
                            dirToAdd,
                            NULL
                        };

                        idxWriter_->addIndexes(dirsToAdd);
                        _CLDECDELETE(dirToAdd);

                        printf("... DONE.\n");
                    }
                else
                    {
                        printf("... FAILED.\n");
                        throw 0;
                    }
                    
                add = false;
            }
    }


    void takeSnapshot()
    {
        int
            lump = (idx_ - 1) / lumpSize_;
        
        char
            lumpStr[5];
        snprintf(lumpStr,
                 sizeof(lumpStr),
                 "%04d",
                 lump);
        
        std::string
            dstDir(snapshotBaseDir_);
        dstDir += lumpStr;
        dstDir += '/';
        
        int 
            result = Cpt::copy(dstDir.c_str(),
                               indexPath_.c_str(),
                               false); // exclusive: don't copy src dir,
        if (result != 0)
            {
                printf("Could not take snapshot %s, %d\n",
                       dstDir.c_str(),
                       result);
                exit(1);
            }
    }


    ~IdxGenerator()
    {
        if (idxWriter_ != NULL)
            {
                idxWriter_->close();
            }
        delete idxWriter_;
    }

};






#define CORPUSDIR    "corpus/"
#define IDXDIR       "idx/"
#define SNAPSHOTSDIR "snapshots/"

#define FILE_CORPUS   CORPUSDIR "hugetestcorpus.txt"
#define POI_CORPUS    CORPUSDIR "pois.txt"
#define STREET_CORPUS CORPUSDIR "streets.csv"

    int main(int argc,
             const char * argv[])
    {
        try
            {
                /* 
                   FileIndexer
                   indexer;
                */

                /* OBS
                CsvIndexer
                    indexer(PoiFields,
                            POI_FIELD_NUM,
                            '|',
                            POI_APPCLASS);
                */

                CsvIndexer
                    indexer(StreetFields,
                            STREET_FIELD_NUM,
                            '|',
                            STREET_APPCLASS);

                IdxGenerator
                    idxGenerator(// FILE_CORPUS,
                                 // POI_CORPUS,
                                 STREET_CORPUS,

                                 IDXDIR,
                                 SNAPSHOTSDIR,
                                 0,

                                 // 20,
                                 // 1209101, // poi
                                 // 147729, // file
                                 // 1665000,
                                 10000000, // streets

                                 // 5,
                                 // 5000,
                                 100000,

                                 indexer,

                                 // add this idx to the first lump:
                                 // effectively appending this index
                                 // with the new corpus
                                 // "/cygdrive/c/wrk/clucene/clperfmetrics/snapshots-3/0483/");
                                 NULL);

                idxGenerator.run();
            }
        catch (LuceneError & err)
            {
                printf("Caught clucene exception %s\n",
                       err.what());
            }
        catch (std::exception & exc)
            {
                printf("Caught std exception %s\n",
                       exc.what());
            }
        catch (...)
            {
                printf("Caught some unknown exception\n");
            }

        return 0;
    }


