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

// INCLUDE FILES
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// This is a GCCE toolchain workaround needed when compiling with GCCE
// and using main() entry point
#ifdef __GCCE__
#include <staticlibinit_gcce.h>
#endif


#include "itk.h"
#include "cpixidxdb.h"
#include "cpixdoc.h"
#include "cpixsearch.h"
#include "cpixidxdb.h"

#define TEST_DOCUMENT_QBASEAPPCLASS "@0:root test document"
#define TEST_SAMPLE_QBASEAPPCLASS "@:root test sample"
#define TEST_CORRUPTTEST1_QBASEAPPCLASS "@:root test corrupttest1"
#define TEST_CORRUPTTEST2_QBASEAPPCLASS "@:root test corrupttest2"

#define TEST_DOCUMENT_INDEXDB_PATH "c:\\Data\\indexing\\test\\"
#define TEST_SAMPLE_INDEXDB_PATH "c:\\Data\\indexing\\sample\\"
#define TEST_CORRUPTTEST1_INDEXDB_PATH "c:\\Data\\indexing\\corrupttest1\\"
#define TEST_CORRUPTTEST2_INDEXDB_PATH "c:\\Data\\indexing\\corrupttest2\\"

#define LDOCUID1 L"document1"
#define LDOCUID2 L"document2"


#define FIELD_ALPHA L"Alpha"


#define DOC1CONTENT L"mary had a little lamb its fleece was black as coal"
#define DOC2CONTENT L"mary had a little little little lamb its fleece was as white as snow"


// The term that will be present in multiple documents.
#define SEARCH_TERM L"little"

class NegativeTests : public Itk::ITestContext
{
private:
    
void CreateAnalyser(Itk::TestMgr  * testMgr,
                    cpix_Analyzer **analyzer_)
{
    
    cpix_Result result;
    cpix_InitParams
                * initParams = cpix_InitParams_create(&result);
    if (cpix_Failed(&result))
        {
        ITK_EXPECT(testMgr,
                   false,
                   "Failed to initialize Cpix");        
        }

    ITK_ASSERT(testMgr,
               cpix_Succeeded(initParams),
               "Failed to set max idle sec");    

    cpix_init(&result,
            initParams);

    *analyzer_ = cpix_CreateSimpleAnalyzer(&result);
    
    if ( !*analyzer_ )
        {
        ITK_PANIC("Analyzer could not be created");
        }
}

void testInit(cpix_IdxDb **iDb_, cpix_IDX_OpenMode_ mode,
            const char *baseAppClass, const char *path)
{
    cpix_Result result;
    
    cpix_IdxDb_defineVolume(&result, 
                baseAppClass, 
                path);
    
    if (cpix_Failed(&result))
        {
        ITK_PANIC("Failed to define test index db");
        }

    *iDb_ = cpix_IdxDb_openDb(&result,
            baseAppClass,
            mode);
    
    if (cpix_Failed(&result))
        {
        ITK_PANIC("Failed to open indexDb");
        }    
}

void createDocument(Itk::TestMgr * testMgr, const wchar_t* data,
                cpix_Analyzer **analyzer_, cpix_IdxDb **iDb_, const wchar_t* content)
{
    cpix_Document *doc;
    cpix_Field field;
    cpix_Result result;
    
    doc = cpix_Document_create(&result,data,NULL,      // app class
            NULL,      // excerpt
            NULL);     // mime type
    
    ITK_ASSERT(testMgr,
               cpix_Succeeded(&result),
               "Creating document %S failed",
               data);

    cpix_Field_initialize(&field,
            FIELD_ALPHA,
            content, 
            cpix_STORE_YES |cpix_INDEX_TOKENIZED);
    
    if (cpix_Failed(&field))
        {
        cpix_Document_destroy(doc);        
        }
    
    ITK_ASSERT(testMgr,
               cpix_Succeeded(&field),
               "Could not create field");
    
    cpix_Document_add(doc,&field);
    
    bool
        succeeded = true;
    if (cpix_Failed(doc))
        {
            cpix_Document_destroy(doc);
            cpix_Field_release(&field);
            succeeded = false;
        }

    ITK_ASSERT(testMgr,
               succeeded,
               "Could not add field to document");
    
    cpix_IdxDb_add(*iDb_, doc, *analyzer_);

    cpix_Document_destroy(doc);    
    
    cpix_IdxDb_flush(*iDb_);    
}

void cleanUp(cpix_Analyzer ** analyzer_, cpix_IdxDb ** iDb_ )
{
    cpix_Analyzer_destroy(*analyzer_);
    analyzer_ = NULL;
    
    cpix_IdxDb_releaseDb(*iDb_);
    *iDb_ = NULL;
     
}

int corruptIndex(const char *path)
{
        char tempPath[250];
        FILE *fp = NULL;
        
        strcpy(tempPath,path);
        strcat(tempPath,"\\_0\\_0.cfs");
        if(access(tempPath,F_OK)== 0)
          fp = fopen(tempPath,"wb");
        
        if(!fp)
            {
               fclose(fp);
               memset(tempPath,0L,250);
               strcpy(tempPath,path);
               strcat(tempPath,"\\_1\\_0.cfs");
               if(access(tempPath,F_OK)== 0)
                   fp = fopen(tempPath,"wb");
               if(!fp)
                   {
                     return -1;
                   }
            }
        fseek(fp,0,SEEK_SET);
        fputs("This is to corrupt the index file.                                       ",fp);
        fclose(fp);
        return 0;
}

int search(cpix_Analyzer ** analyzer_,
           cpix_IdxDb ** iDb_, const wchar_t *searchTerm  )
{
    
    cpix_QueryParser * queryParser_;
    cpix_Query * query_;
    cpix_Hits * hits_;
    cpix_Result result;
    int32_t hits_len=0;
    
    queryParser_ = cpix_QueryParser_create(&result,
                    L"_aggregate",
                    *analyzer_);
    
    if (queryParser_ == NULL)
        {
        ITK_PANIC("Could not create query parser");
        }                

    query_ = cpix_QueryParser_parse(queryParser_, searchTerm);

    if (cpix_Failed(queryParser_))
        {
        ITK_PANIC("Could not create query parser");
        }
    
    hits_ = cpix_IdxDb_search(*iDb_, query_);
    
    if (cpix_Succeeded(hits_))
        {
        hits_len = cpix_Hits_length(hits_); 
        }
    
    cpix_Query_destroy(query_);
    query_ = NULL;
    
    cpix_QueryParser_destroy(queryParser_);
    queryParser_ = NULL;
        
    cpix_Hits_destroy(hits_);
    hits_ = NULL;
    
    return hits_len;    
}

~NegativeTests()
{
}
public:

NegativeTests()
{   
}
virtual void setup() throw (Itk::PanicExc)
{

}

virtual void tearDown() throw()
{

}

void testCreateDb(Itk::TestMgr* /*testMgr */)
{
    cpix_IdxDb   *itestDb_ = NULL, *inewDb_ = NULL;                
    cpix_Result  result;
    
    testInit(&itestDb_, cpix_IDX_CREATE,
            TEST_SAMPLE_QBASEAPPCLASS, TEST_SAMPLE_INDEXDB_PATH);
    
    // try to recreate the same database
    if (itestDb_)
        {
           inewDb_ = cpix_IdxDb_openDb(&result,
                                       TEST_SAMPLE_QBASEAPPCLASS,
                                       cpix_IDX_CREATE);
        }
    if (cpix_Failed(&result))
        {
        ITK_PANIC("Failed to open indexDb");
        }
     
     if ( itestDb_ ) 
         {
            cpix_IdxDb_releaseDb(itestDb_);
            itestDb_ = 0; 
         }
     
     if (inewDb_)
         {
            cpix_IdxDb_releaseDb(inewDb_);
            inewDb_ = 0;
         }
     cpix_IdxDb_undefineVolume(TEST_SAMPLE_QBASEAPPCLASS);
}

void testOpenDb(Itk::TestMgr * testMgr)
{
        cpix_Result  result;            
        cpix_IdxDb   *idxDb_ = NULL;
        
        //try to open a Db an of undefined volume
        idxDb_ = cpix_IdxDb_openDb(&result,
                                       "@0:root test1",
                                       cpix_IDX_CREATE);
        if(idxDb_ == NULL )
            {
            ITK_MSG(testMgr, "Tried to open a database of undefined volume");             
            }
        
        if (idxDb_) 
            {
                cpix_IdxDb_releaseDb(idxDb_);
                idxDb_ = 0; 
            }        
}

void testCorruptedIndex(Itk::TestMgr * testMgr)
{
     cpix_Analyzer * analyzer_ = NULL ;       
     cpix_IdxDb * iCrptDb1_ = NULL, *iCrptDb2_ = NULL;
     
     CreateAnalyser(testMgr,
                    &analyzer_);
     //creat an index and search for the term
     testInit(&iCrptDb1_, cpix_IDX_CREATE, TEST_CORRUPTTEST1_QBASEAPPCLASS,
                     TEST_CORRUPTTEST1_INDEXDB_PATH);    
     createDocument(testMgr, LDOCUID1,&analyzer_,&iCrptDb1_,DOC1CONTENT);
     int hit_len1 = search(&analyzer_,&iCrptDb1_, SEARCH_TERM);
     
     //creat another index and search for the term
     testInit(&iCrptDb2_, cpix_IDX_CREATE, TEST_CORRUPTTEST2_QBASEAPPCLASS,
                     TEST_CORRUPTTEST2_INDEXDB_PATH);
     createDocument(testMgr, LDOCUID1,&analyzer_,&iCrptDb2_,DOC2CONTENT);
     int hit_len2 = search(&analyzer_,&iCrptDb2_, SEARCH_TERM);     
     
     if(iCrptDb1_)
          {
          cpix_IdxDb_releaseDb(iCrptDb1_);
          iCrptDb1_ = 0;
          }
     cpix_IdxDb_undefineVolume(TEST_CORRUPTTEST1_QBASEAPPCLASS);
     
      
     //Release the second Db
     if(iCrptDb2_)
         {
         cpix_IdxDb_releaseDb(iCrptDb2_);
         iCrptDb2_ = 0;
         }
     cpix_IdxDb_undefineVolume(TEST_CORRUPTTEST2_QBASEAPPCLASS);
     //Corrupt the second index
     corruptIndex(TEST_CORRUPTTEST2_INDEXDB_PATH);
     testInit(&iCrptDb1_, cpix_IDX_OPEN, TEST_CORRUPTTEST1_QBASEAPPCLASS,
                     TEST_CORRUPTTEST1_INDEXDB_PATH);
     if(iCrptDb1_)
         int hit_search1 = search(&analyzer_,&iCrptDb1_, SEARCH_TERM);
     
     testInit(&iCrptDb2_, cpix_IDX_OPEN, TEST_CORRUPTTEST2_QBASEAPPCLASS,
                 TEST_CORRUPTTEST2_INDEXDB_PATH);
     int hit_search2=0;
     if(iCrptDb2_)
         hit_search2= search(&analyzer_,&iCrptDb2_, SEARCH_TERM);
     else
         ITK_MSG(testMgr, "Corrupted database, search not possible");              
     
     cleanUp(&analyzer_,&iCrptDb1_);
     
     if(iCrptDb2_)
         {
         cpix_IdxDb_releaseDb(iCrptDb2_);
         iCrptDb2_ = 0;
         }
     
     cpix_IdxDb_undefineVolume(TEST_CORRUPTTEST1_QBASEAPPCLASS);
     cpix_IdxDb_undefineVolume(TEST_CORRUPTTEST2_QBASEAPPCLASS);
 }

};

Itk::TesterBase * CreateNegativeTests()
{
    using namespace Itk;
    
    NegativeTests
            *negativeTests = new NegativeTests;
    ContextTester
            * contextTester = new ContextTester("Negative Tests",
                                                negativeTests);    
#define TEST "CreateDb"
    contextTester->add(TEST,
                    negativeTests,
                    &NegativeTests::testCreateDb);
#undef TEST    

#define TEST "OpenDbofUndefinedVolume"
    contextTester->add(TEST,
                    negativeTests,
                    &NegativeTests::testOpenDb);
#undef TEST 
    
#define TEST "CorruptedIndex"
    contextTester->add(TEST,
                    negativeTests,
                    &NegativeTests::testCorruptedIndex);
#undef TEST     
    
  return contextTester;
}
