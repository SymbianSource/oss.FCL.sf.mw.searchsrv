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


#include <wchar.h>
#include <stddef.h>
#include <iostream>
#include "itk.h"

#include "config.h"
#include "testutils.h"

#include "std_log_result.h"

#include "cpixexc.h"
#include "cpixhits.h"
#include "cpixidxdb.h"


#include "..\..\..\cpix\src\cpixerror.cpp"
#include "..\..\..\cpix\src\cpixanalyzer.cpp"
#include "..\..\..\cpix\src\fileparser\fileparser.cpp"
#include "..\..\..\cpix\src\fileparser\pdffileparser.cpp"
#include "..\..\..\cpix\src\qrytypes\cluceneqrytype.cpp"

const cpix_LangCode cpix_LANG_JA = { "ja" };
void TestAnalyzer(Itk::TestMgr *)
    {
    const cpix_LangCode **C_Lang = cpix_supportedLanguages();
    const wchar_t* toWchar = cpix_ToWideLangCode(cpix_LANG_EN);
    cpix_Result result;
    cpix_Analyzer * c_analyzer = cpix_CreateSnowballAnalyzer(&result,cpix_LANG_JA);
    }
void TestSetError(Itk::TestMgr *)
    {
    struct ErrorInfo serrorinfo;
    serrorinfo.setInfo(ET_CPIX_EXC);
    serrorinfo.setInfo(ET_CPIX_EXC, "");
    cpix_Error *err1 = CreateError(ET_OS_EXC, L"");
    cpix_Error *err2 = CreateError(ET_CPIX_EXC, "");
    cpix_Error_report(NULL,L"",0);
    }

void TestCpixExc(Itk::TestMgr * )
    {
    char *xml_file = (char*)__FUNCTION__;
    assert_failed = 0;
    CpixExc *exc1 = new CpixExc(/*(const wchar_t *)NULL*/L"", "misc.cpp", __LINE__);
    CpixExc *exc2 = new CpixExc("", "misc.cpp", __LINE__);
    CpixExc exc3 = *exc1;
    exc3 = *exc2;
    exc2->file();
    exc2->line();
    exc2->wWhat();
    free(exc1);
    free(exc2);
    testResultXml(xml_file);
    }

void TestHitDocumentList(Itk::TestMgr * )
    {
    Cpix::HitDocumentList *hitdoclist = new Cpix::HitDocumentList;
    hitdoclist->remove(0);
    free(hitdoclist);
    }

void TestTermCreateDestroy(Itk::TestMgr * )
    {
    cpix_Result result;
    cpix_Term *term = cpix_Term_create(&result, CONTENTS_FIELD, L"Hello");
    cpix_Term_destroy(term);
    }

void TestPdfFileParser(Itk::TestMgr * )
    {
    bool isParse1 = Cpix::isFileAllowedToParse(L"");
    bool isParse2 = Cpix::isFileAllowedToParse(L"c:\\data\\cpixunittestcorpus\\pdf\\ctutor");
//     getPDFExcerpt is unnamed namespace so cant call ...
//    std::wstring line;
//    int result = getPDFExcerpt(L"c:\\data\\cpixunittestcorpus\\pdf\\ctutor\\empty.pdf", &line);
    
    char *buffer = "Hello this is test for find string";

    Cpix::FindStringInBuffer(buffer, "find", strlen(buffer));
    
    Cpix::FindStringInBuffer(buffer, "no", strlen(buffer));
    
    Cpix::seen2("find", "If its for find");
    
    Cpix::ExtractNumber("hello calculate 123   ", strlen("hello calculate 123   "));
    
    //Cpix::convertPDFToText("c:\\data\\cpixunittestcorpus\\pdf\\ctutor.pdf");
    
    
    }
void TestLuceneQryType(Itk::TestMgr * )
    {
    cpix_Result result;
    
    cpix_Analyzer* analyzer = cpix_CreateSimpleAnalyzer(&result);
    if ( cpix_Failed( &result) ) 
        {
            ITK_PANIC("Analyzer could not be created");
            assert_failed = 1;
        }
    cpix_QueryParser
        * queryParser = cpix_QueryParser_create(&result,
                                                LCPIX_DEFAULT_FIELD,
                                                analyzer );
    if (queryParser == NULL)
        {
            cpix_Analyzer_destroy( analyzer );
            ITK_PANIC("Could not create query parser");
        }
    Cpix::LuceneQryType *QryType = new Cpix::LuceneQryType();
    std::list<std::wstring> list (2, L"no argumnet");
    //QryType->setUp(queryParser,list,L"Find this");
    
    }


Itk::TesterBase * CreateMiscTests()
    {
    
    using namespace Itk;

    SuiteTester
        * misc = new SuiteTester("misc");

    misc->add("testanalyzer",&TestAnalyzer,"testanalyzer");
    
    misc->add("SetError",&TestSetError,"SetError");
    
    misc->add("CpixExc", &TestCpixExc, "CpixExc");

    misc->add("doclist", &TestHitDocumentList, "doclist");
    
    misc->add("termCD", &TestTermCreateDestroy, "termCD");

    misc->add("pdfparser", &TestPdfFileParser, "pdfparser");
    
    misc->add("lucenetype", &TestLuceneQryType, "lucenetype");
    
    return misc;
    }
