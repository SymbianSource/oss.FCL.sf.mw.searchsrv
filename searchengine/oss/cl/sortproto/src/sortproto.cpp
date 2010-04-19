/*
 ============================================================================
 Name        : sortproto.cpp
 Author      : 
 Copyright   : Your copyright notice
 Description : Main application class
 ============================================================================
 */
 
 /*------------------------------------------------------------------------------
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

// INCLUDE FILES
#include <stdio.h>

#include <fstream>
#include <memory>
#include <vector>

#include <CLucene.h>

#include <cpixtools.h>

#include "idx.h"
#include "customsort.h"


// This is a GCCE toolchain workaround needed when compiling with GCCE
// and using main() entry point
#ifdef __GCCE__
#include <staticlibinit_gcce.h>
#endif


template<typename CLASS>
class PtrVectorSentry
{
private:
    std::vector<CLASS*>     & vector_;
    bool                      own_;
    

public:
    PtrVectorSentry(std::vector<CLASS*> & v)
        : vector_(v),
          own_(true)
    {
        ;
    }

    void disOwn()
    {
        own_ = false;
    }

    ~PtrVectorSentry()
    {
        if (own_)
            {
                using namespace std;
                
                vector<CLASS*>::iterator
                    i = vector_.begin(),
                    end = vector_.end();
                for (; i != end; ++i)
                    {
                        delete *i;
                    }

                vector_.clear();
            }
    }
    
};



template<typename APPDATA>
void IndexTestData(const APPDATA            * appData,
                   const char               * idxDbPath,
                   const FieldDesc<APPDATA> * schema)
{
    printf("\tIndexing to %s ...\n",
           idxDbPath);

    int
        result = Cpt::mkdirs(idxDbPath,
                             0666);

    if (result != 0)
        {
            throw idxDbPath;
        }

    Idx<APPDATA>
        idx(idxDbPath,
            schema);

    idx.indexItems(appData);

    printf("\t... done.\n");
}


const char CalendarIdxDbPath[] = "c:\\Data\\sortproto\\calendar";
const char ContactIdxDbPath[]  = "c:\\Data\\sortproto\\contact";
const char ContactAuxIdxDbPath[] = "c:\\data\\sortproto\\contactaux";
const char MessageIdxDbPath[]  = "c:\\data\\sortproto\\message";

// TODO ADD MORE


const char * IdxDbPaths[] = {
    MessageIdxDbPath,
    ContactIdxDbPath,
    CalendarIdxDbPath,
    ContactAuxIdxDbPath,
    
    // TODO ADD MORE

    NULL
};


struct CalendarData CalendarStuff[] = {
    {   L"cal000",       ROOTCALENDAR,          40.0, // 1.0,
        L"Sprint #1 review",
        L"Sprint #1 review with customer",
        L"5th floor, meeting room #1",
        L"20080813140000"
    },

    {   L"cal001",       ROOTCALENDAR,          1.0,
        L"Sprint #2 planning",
        L"Sprint #2 planning with UI team",
        L"4th floor, meeting room #13",
        L"20080817090000"
    },

    {   L"cal002",       ROOTCALENDAR,          1.0,
        L"Lunch",
        L"Lunch with 007 Sir Roger Moore",
        L"Ravintola Persilja",
        L"20090601113000"
    },
 
    {   L"cal003",       ROOTCALENDAR,          1.0,
        L"Lunch",
        L"Lunch with 666 Mr Baal-se B.",
        L"Ravintola Helvetti",
        L"20090606060606"
    },

    {   NULL,            NULL,          1.0,
        NULL,
        NULL,
        NULL,
        NULL
    }
};


struct ContactData ContactStuff[] = {
    { L"cont000",        ROOTCONTACT,          30.0, // 1.0,
      L"John",
      L"Doe",
      L"+358555001",
      L"20081013080910"
    },

    { L"cont001",        ROOTCONTACT,          1.0,
      L"Jake",
      L"Doe",
      L"+358555002",
      L"20091013080910"
    },

    { L"cont002",        ROOTCONTACT,          1.0, // 30.0,
      L"Jane",
      L"Doe",
      L"+358555003",
      L"20080813080910"
    },
    
    { L"cont003",        ROOTCONTACT,          1.0,
      L"Jennifer",
      L"Doe",
      L"+358555004",
      L"20090613080910"
    },

    {   NULL,            NULL,          1.0,
        NULL,
        NULL,
        NULL
    }
};


struct ContactAuxData ContactAuxStuff[] = {
    { L"caux000",        ROOTCONTACT,          1.0,
      L"0:root contact",
      L"cont002",
      L"20081214120000"
    },

    { L"caux001",        ROOTCONTACT,          1.0,
      L"0:root contact",
      L"cont001",
      L"20090414120000"
    },

    { NULL,              NULL,          1.0,
      NULL,
      NULL,
      NULL
    }
};


struct MessageData MessageStuff[] = {
    { L"msg000",         ROOTMSG,          1.0,
      L"+358555002",
      L"+358000000",
      L"Hi Jake, let's meet at the office in 15 mins",
      L"20090303142800",
    },

    { L"msg001",         ROOTMSG,          50.0,
      L"+358000000",
      L"+358555003",
      L"No I don't.",
      L"20090404142800",
    },

    { NULL,              NULL,          1.0,
      NULL,
      NULL,
      NULL,
      NULL
    }
};

// TODO ADD MORE


void IndexAllTestData()
{
    printf("INDEXING ALL TEST DATA ...\n");

    IndexTestData(MessageStuff,
                  MessageIdxDbPath,
                  MessageSchema);

    IndexTestData(ContactStuff,
                  ContactIdxDbPath,
                  ContactSchema);

    IndexTestData(CalendarStuff,
                  CalendarIdxDbPath,
                  CalendarSchema);

    IndexTestData(ContactAuxStuff,
                  ContactAuxIdxDbPath,
                  ContactAuxSchema);

    printf(" ... DONE.\n");
}


std::auto_ptr<lucene::search::Searcher> 
GetSearcher(std::vector<lucene::search::Searchable*> subSearchables)
{
    using namespace std;

    using namespace lucene::index;
    using namespace lucene::search;

    const char 
        ** idp = IdxDbPaths;
    for (; *idp != NULL; ++idp)
        {
            // well, not exception safe, but this is a test code
            // (don't copy-paste this anywhere)
            subSearchables.push_back(new IndexSearcher(*idp));
        }
    subSearchables.push_back(NULL);

    auto_ptr<Searcher>
        rv(new MultiSearcher(&subSearchables[0]));

    return rv;
}


void PrintHits(lucene::search::Hits * hits,
               std::wostream        & os)
{
    using namespace std;

    using namespace lucene::document;
    using namespace lucene::search;

    int32_t
        hitCount = hits->length();

    printf("Number of hits: %d\n",
           hitCount);
    os << L"Number of hits: " << hitCount << endl;

    for (int32_t i = 0; i < hitCount; ++i)
        {
            Document
                * doc(&hits->doc(i));

            /*
            auto_ptr<DocumentFieldEnumeration>
                dfe(doc->fields());

            printf("\tDOCUMENT:\n");

            while (dfe->hasMoreElements())
                {
                    Field
                        * field(dfe->nextElement());

                    printf("\t o %S: %S\n",
                           field->name(),
                           field->stringValue());
                }
            */
            printf("\t DOC %S: %S\n",
                   doc->get(DOCUID),
                   doc->get(AGGREGATE));
            os << L"\t DOC " << doc->get(DOCUID) << L": "
               << doc->get(AGGREGATE) << endl;

            const wchar_t
                * dateTime = doc->get(DATETIME);
            if (dateTime != NULL)
                {
                    os << "\t   dt: " << dateTime << endl;
                }
        }
}


void DumpIndexes()
{
    printf("DUMPING INDEXES ...\n");

    using namespace std;

    using namespace lucene::analysis;
    using namespace lucene::analysis::standard;
    using namespace lucene::search;
    using namespace lucene::queryParser;
    using namespace lucene::index;

    vector<Searchable*>
        subSearchables;
    PtrVectorSentry<Searchable>
        sss(subSearchables);

    auto_ptr<Searcher>
        searcher(GetSearcher(subSearchables));

    auto_ptr<Analyzer>
        analyzer(new StandardAnalyzer());

    auto_ptr<QueryParser>
        queryParser(new QueryParser(DUMP,
                                    analyzer.get()));

    auto_ptr<Query>
        query(queryParser->parse(DUMP));

    auto_ptr<Sort>
        sort(new Sort());

    vector<SortField*>
        sortFields;
    PtrVectorSentry<SortField>
        sfs(sortFields);

    /*
    sortFields.push_back(new SortField(DOCUID,
                                       SortField::STRING,
                                       false));
    */

    auto_ptr<AppDomainSortFactory>
        adsf(new AppDomainSortFactory);

    sortFields.push_back(new SortField(APPCLASSTOKEN,
                                       adsf.get(),
                                       false));

    /*
    sortFields.push_back(new SortField(DATETIME,
                                       SortField::STRING,
                                       false));
    */
    
    /* 
    sortFields.push_back(new SortField(L"dummy-dummy-relevance",
                                       SortField::DOCSCORE,
                                       false));
    */

    auto_ptr<UpcomingEventSortFactory>
        uesf(new UpcomingEventSortFactory(L"20090114160000"));
    
    sortFields.push_back(new SortField(DATETIME,
                                       uesf.get(),
                                       false));
                                       
    sortFields.push_back(NULL);

    sort->setSort(&sortFields[0]);

    auto_ptr<Hits>
        hits(searcher->search(query.get(),
                              sort.get()));

    std::wofstream
        ofs("c:\\Data\\sortproto\\hits.txt");

    PrintHits(hits.get(),
              ofs);

    printf("... DONE.\n");
}


int main(void)
{
    try
        {
            IndexAllTestData();
            DumpIndexes();
        }
    catch (const char * err)
        {
            printf("Caught exception: %s\n",
                    err);
        }
    catch (...)
        {
            printf("Caught unknown exception\n");
        }

    int
        c = getchar();
    
    return 0;
}
