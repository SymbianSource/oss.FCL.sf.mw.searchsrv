
// INCLUDE FILES
#include <stdio.h>
#include <iostream.h>

// This is a GCCE toolchain workaround needed when compiling with GCCE
// and using main() entry point
#ifdef __GCCE__
#include <staticlibinit_gcce.h>
#endif

#include "cpixidxdb.h"
#include "cpixdoc.h"
#include "cpixsearch.h"
#include "cpixidxdb.h"
#include "rotlogger.h"
#include "common/cpixlog.h"


#define TEST_DOCUMENT_QBASEAPPCLASS "@0:root test document"
#define TEST_DOCUMENT_INDEXDB_PATH "c:\\Data\\indexing\\test\\"

#define LDOCUID1 L"document1"
#define LDOCUID2 L"document2"


#define FIELD_ALPHA L"Alpha"
#define FIELD_ALPHA1 L"Alpha1"


#define DOC1CONTENT L"mary had little lamb issue its 9740069217 9999ss ssss7 sad76asd 12222ds asdfa23sdf234sdf anirban fleece was black as coal"
#define DOC2CONTENT L"sri rama jeyam and it  3gpp_70.jpg 170(kb).jpg is ss asd shankar.rajendran@yahoo.co.in then www.google.com U.S.A. file.txt"


// The term that will be present in multiple documents.
#define SEARCH_TERM L"$prefix(\"9999ss\")"


int testInit(cpix_Analyzer **analyzer_, cpix_IdxDb **idxDb_)
    {
    
    cpix_Result result;
    cpix_InitParams
    * initParams = cpix_InitParams_create(&result);
    
    
    if (cpix_Failed(&result))
        {
        printf("Failed to initialize Cpix\n");
        return 0;
        }

    if (cpix_Failed(initParams))
        {
        printf("Failed to set max idle sec\n");
        return 0;
        }

    cpix_init(&result,
            initParams);

    if ( !( *analyzer_ = cpix_Analyzer_create(&result,L"standard") ) )
        {
        printf("Analyzer could not be created \n");
        return 0;
        }
    cpix_IdxDb_defineVolume(&result, 
            TEST_DOCUMENT_QBASEAPPCLASS, 
            TEST_DOCUMENT_INDEXDB_PATH);
    if (cpix_Failed(&result))
        {
        printf("Failed to define test index db\n");
        return 0;
        }

    *idxDb_ = cpix_IdxDb_openDb(&result,
            TEST_DOCUMENT_QBASEAPPCLASS,
            cpix_IDX_OPEN);
    if (cpix_Failed(&result))
        {
        printf("Failed to open indexDb\n");
        return 0;
        }
    return 1;
    }

int createDocument(const wchar_t* docUid, const wchar_t* data,cpix_Analyzer **analyzer_, cpix_IdxDb **idxDb_)
    {
    cpix_Document *doc;
    cpix_Field field;
    cpix_Field field1;
    cpix_Field field2;
    cpix_Field field3;
    cpix_Result result;
    
    doc = cpix_Document_create(&result,docUid,NULL,      // app class
            NULL,      // excerpt
            NULL);     // mime type
    
    if (cpix_Failed(&result))
        {
        printf("Failed to create a document\n");
        return 0;
        }

    cpix_Field_initialize(&field,
            FIELD_ALPHA,
            data, 
            cpix_STORE_YES |cpix_INDEX_UNTOKENIZED | cpix_AGGREGATE_YES | cpix_FREE_TEXT );
    
    cpix_Field_initialize(&field1,
                FIELD_ALPHA1,
                data, 
                cpix_STORE_YES |cpix_INDEX_UNTOKENIZED | cpix_AGGREGATE_YES);
    
    
    cpix_Field_initialize(&field2,
                L"ALPHA2",
                L"This is shankar and I am working for nokia", 
                cpix_STORE_YES |cpix_INDEX_TOKENIZED | cpix_AGGREGATE_YES | cpix_FREE_TEXT );
    
    
    cpix_Field_initialize(&field3,
                L"ALPHA3",
                L"This is shankar and I am working for nokia", 
                cpix_STORE_NO |cpix_INDEX_TOKENIZED | cpix_AGGREGATE_YES);

    if (cpix_Failed(&field))
        {
        cpix_Document_destroy(doc);
        printf("Failed to initialize the field \n");
        return 0;
        }
    cpix_Document_add(doc,&field);
    cpix_Document_add(doc,&field1);
    cpix_Document_add(doc,&field2);
    cpix_Document_add(doc,&field3);
    cpix_IdxDb_add(*idxDb_,doc,*analyzer_);

    cpix_Document_destroy(doc);
    cpix_IdxDb_flush(*idxDb_);
    return 1;
    }
void cleanUp(cpix_Analyzer ** analyzer_, cpix_QueryParser ** queryParser_, cpix_Query ** query_ , cpix_IdxDb ** idxDb_ , cpix_Hits ** hits_)
    {
    cpix_Analyzer_destroy(*analyzer_);
    analyzer_ = NULL;

    cpix_QueryParser_destroy(*queryParser_);
    queryParser_ = NULL;

    cpix_Query_destroy(*query_);
    query_ = NULL;

    cpix_IdxDb_releaseDb(*idxDb_);
    idxDb_ = NULL;

    cpix_Hits_destroy(*hits_);
    hits_ = NULL;

    }

void printHit(cpix_Document * doc)
{
    using namespace std;
       
    const wchar_t 
        * value = cpix_Document_getFieldValue(doc,FIELD_ALPHA);
    
       
    fprintf(stdout, 
            "%S\n",
            value);
}

void printHits(cpix_Hits    * hits)
{
    using namespace std;

    int32_t
        hitCount = cpix_Hits_length(hits);

    if (cpix_Failed(hits))
        {
            printf("Hits failed\n");
            return;
        }

    cout << "Number of hits: " << hitCount << endl;
    while (1)
        {
    int fromDoc;
    int docCount;
    printf("\nFrom: ");
    scanf("%d",&fromDoc);
    printf("\nHow many doc: ");
    scanf("%d",&docCount);
    
    cpix_Document
        **doc;
    
    doc = (cpix_Document **) malloc (sizeof(cpix_Document *) * docCount);
    for(int j = 0 ; j < docCount; j++)
        {
            doc[j] = (cpix_Document *) malloc (sizeof(cpix_Document));
            doc[j]->ptr_ = NULL;
        }
    
            cpix_Hits_doc(hits,
                          fromDoc,
                          doc,
                          docCount);
            
     for(int j = 0 ; docCount>j && doc[j]->ptr_ != NULL; j++)
         {
            printf("%d \n", j);
            printHit(doc[j]);
         }
     
     for(int j = 0 ; j < docCount; j++)
             free(doc[j]);

     
      free(doc);
       
        }
    logDbgMsg("Shankar Ha hahah");
}

int main(void)
    {
    
        cpix_Result result;
        cpix_Analyzer * analyzer_ = NULL ;
        cpix_QueryParser * queryParser_;
        cpix_Query * query_;
        cpix_IdxDb * idxDb_ = NULL;
        cpix_Hits * hits_;

      

        testInit(&analyzer_,&idxDb_);   
        for(int j = 0; j < 10; j++)
            {
        createDocument(LDOCUID1,DOC1CONTENT, &analyzer_,&idxDb_);
        createDocument(LDOCUID2,DOC2CONTENT, &analyzer_,&idxDb_);
            }
                
                        
        queryParser_ = cpix_QueryParser_create(&result,
                L"_aggregate",
                analyzer_);
        if (queryParser_ == NULL)
            {
            printf("Could not create query parser\n");
            }                

        query_ = cpix_QueryParser_parse(queryParser_, SEARCH_TERM);

        if ( query_ == NULL) { // sometimes the query parser returns NULL then we crash
            return 0;
        }
        if (cpix_Failed(queryParser_))
            {
            printf("Could not create query parser \n");
            return 0;
            }

        hits_ = cpix_IdxDb_search(idxDb_, query_);
        printHits( hits_); 


        int32_t hits_len = cpix_Hits_length(hits_);
        
        cleanUp(&analyzer_, &queryParser_,&query_,&idxDb_,&hits_);

        printf("Press a character to exit!\n");
        int c = getchar();
        return 0;
    }
