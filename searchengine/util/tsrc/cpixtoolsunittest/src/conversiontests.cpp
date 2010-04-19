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

#include <string>
#include <iostream>

#include "itk.h"

#include "cpixstrtools.h"


template<typename V>
void testConvertDefault(Itk::TestMgr  * testMgr,
                                     V             * v,
                                     const wchar_t * pass[],
                                     const wchar_t * fail[])
{
    using namespace Cpt;
    using namespace std;

    wcout << L"pass:" << endl;
    for (const wchar_t ** p = pass; *p != NULL; ++p)
        {
            const char
                * result = wconvert(v,
                                    *p);

            ITK_EXPECT(testMgr,
                       result == NULL,
                       "Should have passed: %S",
                       *p);

            wcout << L"SUCCESS: " << *p << ": ";
            wcout << *v << endl;
        }

    wcout << L"fail:" << endl;
    for (const wchar_t ** p = fail; *p != NULL; ++p)
        {
            const char
                * result = wconvert(v,
                                    *p);

            ITK_EXPECT(testMgr,
                       result != NULL,
                       "Should NOT have passed: %S",
                       *p);

            wcout << L"FAILURE: " << *p << ": " << flush;
            cout << result << endl << flush;
        }
}


template<typename UINT>
void testConvertOctal(Itk::TestMgr  * testMgr,
                      UINT          * v,
                      const wchar_t * pass[],
                      const wchar_t * fail[])
{
    using namespace Cpt;
    using namespace std;

    wcout << L"pass:" << endl;
    for (const wchar_t ** p = pass; *p != NULL; ++p)
        {
            const char
                * result = wconvertOctal(v,
                                         *p);

            ITK_EXPECT(testMgr,
                       result == NULL,
                       "Should have passed: %S",
                       *p);

            wcout << L"SUCCESS: " << *p << ": ";
            wcout << *v << endl;
        }

    wcout << L"fail:" << endl;
    for (const wchar_t ** p = fail; *p != NULL; ++p)
        {
            const char
                * result = wconvertOctal(v,
                                    *p);

            ITK_EXPECT(testMgr,
                       result != NULL,
                       "Should NOT have passed: %S",
                       *p);

            wcout << L"FAILURE: " << *p << ": " << flush;
            cout << result << endl << flush;
        }
}


template<typename UINT>
void testConvertHexadecimal(Itk::TestMgr  * testMgr,
                            UINT          * v,
                            const wchar_t * pass[],
                            const wchar_t * fail[])
{
    using namespace Cpt;
    using namespace std;

    wcout << L"pass:" << endl;
    for (const wchar_t ** p = pass; *p != NULL; ++p)
        {
            const char
                * result = wconvertHexadecimal(v,
                                               *p);

            ITK_EXPECT(testMgr,
                       result == NULL,
                       "Should have passed: %S",
                       *p);

            wcout << L"SUCCESS: " << *p << ": ";
            wcout << *v << endl;
        }

    wcout << L"fail:" << endl;
    for (const wchar_t ** p = fail; *p != NULL; ++p)
        {
            const char
                * result = wconvertHexadecimal(v,
                                               *p);

            ITK_EXPECT(testMgr,
                       result != NULL,
                       "Should NOT have passed: %S",
                       *p);

            wcout << L"FAILURE: " << *p << ": ";
            wcout << result << endl << flush;
        }
}



void testConvertInteger(Itk::TestMgr * testMgr)
{
    printf("TESTING integer (generic, default) formatting. Target: int\n");

    static const wchar_t * pass[] = {
        L"0",
        L"123",
        L"-123",
        L"0xdead",
        L"0x100",
        L"0100",
        L"100",
        NULL
    };


    static const wchar_t * fail[] = {
        L"0a",
        L"123a",
        L"-123a",
        L"0xdeadg",
        L"0x100g",
        L"01009",
        L"100.",
        NULL
    };

    int
        v;

    testConvertDefault(testMgr,
                       &v,
                       pass,
                       fail);
}


void testConvertInteger_8(Itk::TestMgr * testMgr)
{
    printf("TESTING integer (octal) formatting. Target: unsigned int\n");

    static const wchar_t * pass[] = {
        L"0",
        L"123",
        L"0100",
        L"100",
        NULL
    };


    static const wchar_t * fail[] = {
        L"0a",
        L"123a",
        L"01009",
        L"100.",
        NULL
    };

    unsigned int
        v;

    testConvertOctal(testMgr,
                     &v,
                     pass,
                     fail);
}

void testConvertInteger_16(Itk::TestMgr * testMgr)
{
    printf("TESTING integer (hexadecimal) formatting. Target: unsigned int\n");

    static const wchar_t * pass[] = {
        L"ff",
        L"beef",
        L"123",
        L"0100",
        L"100",
        NULL
    };


    static const wchar_t * fail[] = {
        L"0z",
        L"123g",
        L"0100g",
        L"100.",
        NULL
    };

    unsigned int
        v;

    testConvertHexadecimal(testMgr,
                           &v,
                           pass,
                           fail);
}


void testConvertFloat(Itk::TestMgr * testMgr)
{
    printf("TESTING float (generic, default) formatting. Target: float\n");

    static const wchar_t * pass[] = {
        L"0",
        L"0.5",
        L".5",
        L"-.5",
        L"+.5",
        L"+12.345",
        NULL
    };


    static const wchar_t * fail[] = {
        L"a",
        L".-5",
        L"zaza",
        L"3.1415a",
        NULL
    };

    float
        v;

    testConvertDefault(testMgr,
                       &v,
                       pass,
                       fail);
}


void testConvertDouble(Itk::TestMgr * testMgr)
{
    printf("TESTING double (generic, default) formatting. Target: double\n");

    static const wchar_t * pass[] = {
        L"0",
        L"0.5",
        L".5",
        L"-.5",
        L"+.5",
        L"+12.345",
        NULL
    };


    static const wchar_t * fail[] = {
        L"a",
        L".-5",
        L"zaza",
        L"3.1415a",
        NULL
    };

    double
        v;

    testConvertDefault(testMgr,
                       &v,
                       pass,
                       fail);
}


Itk::TesterBase * CreateConversionTests()
{
    using namespace Itk;

    SuiteTester
        * conversionTests = new SuiteTester("conversion");

#define TEST "integer"
    conversionTests->add(TEST,
                         testConvertInteger,
                         TEST);
#undef TEST

#define TEST "integer_8"
    conversionTests->add(TEST,
                         testConvertInteger_8,
                         TEST);
#undef TEST

#define TEST "integer_16"
    conversionTests->add(TEST,
                         testConvertInteger_16,
                         TEST);
#undef TEST

#define TEST "float"
    conversionTests->add(TEST,
                         testConvertFloat,
                         TEST);
#undef TEST

#define TEST "double"
    conversionTests->add(TEST,
                         testConvertDouble,
                         TEST);
#undef TEST


        
    // ... add more tests to suite
    
    return conversionTests;
}

