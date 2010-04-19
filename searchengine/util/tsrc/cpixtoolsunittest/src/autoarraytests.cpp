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


struct ConversionTest 
{
    const wchar_t        * wcs_;
    const char           * mbs_;
};


extern const struct ConversionTest ConversionTests[];


void TestWcsToMbs(Itk::TestMgr * mgr)
{
    using namespace Cpt;

    for (const ConversionTest * p = ConversionTests;
         p->wcs_ != NULL;
         ++p)
        {
            try 
                {
                    printf("Converting: wcs '%S', expecting: '%s'.\n",
                           p->wcs_,
                           p->mbs_);
                    
                    auto_array<char>
                        aa(p->wcs_);
                    
                    ITK_EXPECT(mgr,
                               strcmp(p->mbs_, aa.get()) == 0,
                               "Wcs to Mbs unexpected result");
                    
                    printf(" -> result: '%s'\n\n",
                           aa.get());
                }
            catch (std::exception & exc)
                {
                    ITK_EXPECT(mgr,
                               false,
                               "Conversion failed: %s",
                               exc.what());
                }
                
        }
}



void TestMbsToWcs(Itk::TestMgr * mgr)
{
    using namespace Cpt;

    for (const ConversionTest * p = ConversionTests;
         p->wcs_ != NULL;
         ++p)
        {
            try 
                {
                    printf("Converting: mbs '%s', expecting: '%S'.\n",
                           p->mbs_,
                           p->wcs_);
                    
                    auto_array<wchar_t>
                        aa(p->mbs_);
                    
                    ITK_EXPECT(mgr,
                               wcscmp(p->wcs_, aa.get()) == 0,
                               "Mbs to wcs unexpected result");
                    
                    printf(" -> result: '%S'\n\n",
                           aa.get());
                }
            catch (std::exception & exc)
                {
                    ITK_EXPECT(mgr,
                               false,
                               "Conversion failed: %s",
                               exc.what());
                }
                
        }
}



Itk::TesterBase * CreateAutoArrayTests()
{
    using namespace Itk;

    SuiteTester
        * aaTests = new SuiteTester("auto_array");

#define TEST "wcs2mbs"
    aaTests->add(TEST,
                 TestWcsToMbs,
                 TEST);
#undef TEST


#define TEST "mbs2wcs"
    aaTests->add(TEST,
                 TestMbsToWcs,
                 TEST);
#undef TEST


    // TODO add more

    return aaTests;
}




const struct ConversionTest ConversionTests[] = {
    
    // empty string
    { L"",
      "" 
    },


    // english text
    { L"hello",
      "hello"
    },

    { L"Three rings for the Elven kings under the stars",
      "Three rings for the Elven kings under the stars"
    },


    // russian text
    { L"\x420\x443\x441\x441\x43a\x438\x439\x20\x44f\x437\x44b\x43a",
      "\xd0\xa0\xd1\x83\xd1\x81\xd1\x81\xd0\xba\xd0\xb8\xd0\xb9\x20\xd1\x8f\xd0\xb7\xd1\x8b\xd0\xba"
    },

    { L"\x41f\x43e\x434\x430\x440\x43e\x43a\x20\x43e\x431\x43e\x448\x435\x43b\x441\x44f\x20\x432\x20\x24\x31\x32\x20\x442\x44b\x441\x44f\x447",
      "\xd0\x9f\xd0\xbe\xd0\xb4\xd0\xb0\xd1\x80\xd0\xbe\xd0\xba\x20\xd0\xbe\xd0\xb1\xd0\xbe\xd1\x88\xd0\xb5\xd0\xbb\xd1\x81\xd1\x8f\x20\xd0\xb2\x20\x24\x31\x32\x20\xd1\x82\xd1\x8b\xd1\x81\xd1\x8f\xd1\x87"
    },

    { L"\x413\x43e\x43b\x43b\x438\x432\x443\x434\x441\x43a\x438\x439\x20\x430\x43a\x442\x435\x440\x20\x411\x440\x44d\x434\x20\x41f\x438\x442\x442\x20\x43f\x43e\x43f\x430\x43b\x20\x432\x20\x430\x432\x430\x440\x438\x44e\x20\x432\x20\x41b\x43e\x441\x2d\x410\x43d\x434\x436\x435\x43b\x435\x441\x435",
      "\xd0\x93\xd0\xbe\xd0\xbb\xd0\xbb\xd0\xb8\xd0\xb2\xd1\x83\xd0\xb4\xd1\x81\xd0\xba\xd0\xb8\xd0\xb9\x20\xd0\xb0\xd0\xba\xd1\x82\xd0\xb5\xd1\x80\x20\xd0\x91\xd1\x80\xd1\x8d\xd0\xb4\x20\xd0\x9f\xd0\xb8\xd1\x82\xd1\x82\x20\xd0\xbf\xd0\xbe\xd0\xbf\xd0\xb0\xd0\xbb\x20\xd0\xb2\x20\xd0\xb0\xd0\xb2\xd0\xb0\xd1\x80\xd0\xb8\xd1\x8e\x20\xd0\xb2\x20\xd0\x9b\xd0\xbe\xd1\x81\x2d\xd0\x90\xd0\xbd\xd0\xb4\xd0\xb6\xd0\xb5\xd0\xbb\xd0\xb5\xd1\x81\xd0\xb5"
    },

    { L"\x41e\x43d\x430\x20\x437\x430\x43a\x430\x437\x430\x43b\x430\x20\x44\x75\x63\x61\x74\x69\x20\x4d\x6f\x6e\x73\x74\x65\x72\x20\x31\x31\x30\x30\x53\x20\x54\x69\x74\x61\x6e\x69\x75\x6d\x20\x432\x20\x418\x442\x430\x43b\x438\x438",
      "\xd0\x9e\xd0\xbd\xd0\xb0\x20\xd0\xb7\xd0\xb0\xd0\xba\xd0\xb0\xd0\xb7\xd0\xb0\xd0\xbb\xd0\xb0\x20\x44\x75\x63\x61\x74\x69\x20\x4d\x6f\x6e\x73\x74\x65\x72\x20\x31\x31\x30\x30\x53\x20\x54\x69\x74\x61\x6e\x69\x75\x6d\x20\xd0\xb2\x20\xd0\x98\xd1\x82\xd0\xb0\xd0\xbb\xd0\xb8\xd0\xb8"
    },
    

    // finnish text
    { L"\x53\x75\x6f\x6d\x69",
      "\x53\x75\x6f\x6d\x69"
    },

    { L"\xc4\xe4\x20\xd6\xf6\x20\xc5\xe5\x2e",
      "\xc3\x84\xc3\xa4\x20\xc3\x96\xc3\xb6\x20\xc3\x85\xc3\xa5\x2e"
    },

    { L"\x4b\x61\x69\x6b\x6b\x69\x20\x65\x72\x69\x74\x79\x69\x73\x72\x79\x68\x6d\xe4\x74\x20\x6f\x6e\x20\x72\x6f\x6b\x6f\x74\x65\x74\x74\x75\x20\x76\x75\x6f\x64\x65\x6e\x20\x6c\x6f\x70\x70\x75\x75\x6e\x20\x6d\x65\x6e\x6e\x65\x73\x73\xe4\x2e",
      "\x4b\x61\x69\x6b\x6b\x69\x20\x65\x72\x69\x74\x79\x69\x73\x72\x79\x68\x6d\xc3\xa4\x74\x20\x6f\x6e\x20\x72\x6f\x6b\x6f\x74\x65\x74\x74\x75\x20\x76\x75\x6f\x64\x65\x6e\x20\x6c\x6f\x70\x70\x75\x75\x6e\x20\x6d\x65\x6e\x6e\x65\x73\x73\xc3\xa4\x2e"
    },

    { L"\x52\x6f\x6b\x6f\x74\x74\x65\x69\x74\x61\x20\x6f\x6e\x20\x74\xe4\x6c\x6c\xe4\x20\x76\x69\x69\x6b\x6f\x6c\x6c\x61\x20\x6b\xe4\x79\x74\xf6\x73\x73\xe4\x20\x34\x33\x37\x20\x30\x30\x30\x2c\x20\x6a\x6f\x6b\x61\x20\x76\x69\x69\x6b\x6b\x6f\x20\x74\x75\x6c\x65\x65\x20\x31\x30\x30\x20\x30\x30\x30\x20\x2d\x20\x32\x30\x30\x20\x30\x30\x30\x20\x61\x6e\x6e\x6f\x73\x74\x61\x20\x6c\x69\x73\xe4\xe4\x2e",
      "\x52\x6f\x6b\x6f\x74\x74\x65\x69\x74\x61\x20\x6f\x6e\x20\x74\xc3\xa4\x6c\x6c\xc3\xa4\x20\x76\x69\x69\x6b\x6f\x6c\x6c\x61\x20\x6b\xc3\xa4\x79\x74\xc3\xb6\x73\x73\xc3\xa4\x20\x34\x33\x37\x20\x30\x30\x30\x2c\x20\x6a\x6f\x6b\x61\x20\x76\x69\x69\x6b\x6b\x6f\x20\x74\x75\x6c\x65\x65\x20\x31\x30\x30\x20\x30\x30\x30\x20\x2d\x20\x32\x30\x30\x20\x30\x30\x30\x20\x61\x6e\x6e\x6f\x73\x74\x61\x20\x6c\x69\x73\xc3\xa4\xc3\xa4\x2e"
    },


    // hungarian text - TODO
    { L"\x23\x23\x23\x20\x4d\x61\x67\x79\x61\x72",
      "\x23\x23\x23\x20\x4d\x61\x67\x79\x61\x72"
    },

    { L"\xc1\xe1\x20\xc9\xe9\x20\xcd\xed\x20\xd3\xf3\x20\xd6\xf6\x20\xd5\xf5\x20\xda\xfa\x20\xdc\xfc\x20\x170\x171\x2e",
      "\xc3\x81\xc3\xa1\x20\xc3\x89\xc3\xa9\x20\xc3\x8d\xc3\xad\x20\xc3\x93\xc3\xb3\x20\xc3\x96\xc3\xb6\x20\xc3\x95\xc3\xb5\x20\xc3\x9a\xc3\xba\x20\xc3\x9c\xc3\xbc\x20\xc5\xb0\xc5\xb1\x2e"
    },

    { L"\x4e\x61\x67\x79\x20\x6d\xe9\x72\x65\x74\x171\x2c\x20\x69\x7a\x7a\xf3\x2c",
      "\x4e\x61\x67\x79\x20\x6d\xc3\xa9\x72\x65\x74\xc5\xb1\x2c\x20\x69\x7a\x7a\xc3\xb3\x2c"
    },

    { L"\x6d\x61\x6a\x64\x20\x61\x20\x66\xf6\x6c\x64\x65\x74\x20\xe9\x72\xe9\x73\x20\x75\x74\xe1\x6e\x20\x6c\xe1\x6e\x67\x62\x61\x20\x62\x6f\x72\x75\x6c\xf3\x2c",
      "\x6d\x61\x6a\x64\x20\x61\x20\x66\xc3\xb6\x6c\x64\x65\x74\x20\xc3\xa9\x72\xc3\xa9\x73\x20\x75\x74\xc3\xa1\x6e\x20\x6c\xc3\xa1\x6e\x67\x62\x61\x20\x62\x6f\x72\x75\x6c\xc3\xb3\x2c"
    },

    { L"\x65\x67\x79\x65\x6c\x151\x72\x65\x20\x61\x7a\x6f\x6e\x6f\x73\xed\x74\x61\x74\x6c\x61\x6e\x20\x74\xe1\x72\x67\x79\x20\x63\x73\x61\x70\xf3\x64\x6f\x74\x74\x20\x62\x65\x20\x5b\x31\x5d\x20\x4c\x65\x74\x74\x6f\x72\x73\x7a\xe1\x67\x62\x61\x6e\x2c\x20\x4d\x61\x7a\x73\x61\x6c\x61\x63\x61\x20\x5b\x32\x5d\x20",
      "\x65\x67\x79\x65\x6c\xc5\x91\x72\x65\x20\x61\x7a\x6f\x6e\x6f\x73\xc3\xad\x74\x61\x74\x6c\x61\x6e\x20\x74\xc3\xa1\x72\x67\x79\x20\x63\x73\x61\x70\xc3\xb3\x64\x6f\x74\x74\x20\x62\x65\x20\x5b\x31\x5d\x20\x4c\x65\x74\x74\x6f\x72\x73\x7a\xc3\xa1\x67\x62\x61\x6e\x2c\x20\x4d\x61\x7a\x73\x61\x6c\x61\x63\x61\x20\x5b\x32\x5d\x20"
    },
    
    { L"\x6b\x69\x73\x76\xe1\x72\x6f\x73\x20\x6d\x65\x6c\x6c\x65\x74\x74\x20\x65\x67\x79\x20\x65\x6c\x68\x61\x67\x79\x61\x74\x6f\x74\x74\x20\x6c\x65\x67\x65\x6c\x151\x6e\x20\x76\x61\x73\xe1\x72\x6e\x61\x70\x72\xf3\x6c\x20\x68\xe9\x74\x66\x151\x72\x65\x20\x76\x69\x72\x72\x61\x64\xf3\x20\xe9\x6a\x6a\x65\x6c\x2c\x20\x6b\xf6\x7a\xf6\x6c\x74\x65\x20\x61\x20\x68\x65\x6c\x79\x69\x20\x6b\x61\x74\x61\x73\x7a\x74\x72\xf3\x66\x61\x65\x6c\x68\xe1\x72\xed\x74\xe1\x73\x2e",
      "\x6b\x69\x73\x76\xc3\xa1\x72\x6f\x73\x20\x6d\x65\x6c\x6c\x65\x74\x74\x20\x65\x67\x79\x20\x65\x6c\x68\x61\x67\x79\x61\x74\x6f\x74\x74\x20\x6c\x65\x67\x65\x6c\xc5\x91\x6e\x20\x76\x61\x73\xc3\xa1\x72\x6e\x61\x70\x72\xc3\xb3\x6c\x20\x68\xc3\xa9\x74\x66\xc5\x91\x72\x65\x20\x76\x69\x72\x72\x61\x64\xc3\xb3\x20\xc3\xa9\x6a\x6a\x65\x6c\x2c\x20\x6b\xc3\xb6\x7a\xc3\xb6\x6c\x74\x65\x20\x61\x20\x68\x65\x6c\x79\x69\x20\x6b\x61\x74\x61\x73\x7a\x74\x72\xc3\xb3\x66\x61\x65\x6c\x68\xc3\xa1\x72\xc3\xad\x74\xc3\xa1\x73\x2e"
    },


    // chinese text - TODO
    { L"\x4e2d\x6587",
      "\xe4\xb8\xad\xe6\x96\x87"
    },

    { L"\x89e3\x653e\x519b\x4eca\x5e74\x6f14\x4e60\x5f3a\x5ea6\x5bc6\x5ea6\x22\x7f55\x89c1\x22\x20\x5f00\x653e\x900f\x660e\x5ea6\x7a7a\x524d\x20\x9ad8\x6e05",
      "\xe8\xa7\xa3\xe6\x94\xbe\xe5\x86\x9b\xe4\xbb\x8a\xe5\xb9\xb4\xe6\xbc\x94\xe4\xb9\xa0\xe5\xbc\xba\xe5\xba\xa6\xe5\xaf\x86\xe5\xba\xa6\x22\xe7\xbd\x95\xe8\xa7\x81\x22\x20\xe5\xbc\x80\xe6\x94\xbe\xe9\x80\x8f\xe6\x98\x8e\xe5\xba\xa6\xe7\xa9\xba\xe5\x89\x8d\x20\xe9\xab\x98\xe6\xb8\x85"
    },

    { L"\x7a0e\x52a1\x603b\x5c40\xff1a\x5e74\x5e95\x5c06\x529b\x4e89\x5b9e\x73b0\x6263\x7f34\x5355\x4f4d\x660e\x7ec6\x7533\x62a5\x4e2a\x7a0e\x7ba1\x7406",
      "\xe7\xa8\x8e\xe5\x8a\xa1\xe6\x80\xbb\xe5\xb1\x80\xef\xbc\x9a\xe5\xb9\xb4\xe5\xba\x95\xe5\xb0\x86\xe5\x8a\x9b\xe4\xba\x89\xe5\xae\x9e\xe7\x8e\xb0\xe6\x89\xa3\xe7\xbc\xb4\xe5\x8d\x95\xe4\xbd\x8d\xe6\x98\x8e\xe7\xbb\x86\xe7\x94\xb3\xe6\x8a\xa5\xe4\xb8\xaa\xe7\xa8\x8e\xe7\xae\xa1\xe7\x90\x86"
    },

    { L"\x53d1\x6539\x59d4\x5236\x5b9a\x7684\x6536\x5165\x5206\x914d\x8c03\x8282\x65b9\x6848\x20\x5df2\x7ecf\x4e0a\x62a5\x56fd\x52a1\x9662\x5f85\x6279",
      "\xe5\x8f\x91\xe6\x94\xb9\xe5\xa7\x94\xe5\x88\xb6\xe5\xae\x9a\xe7\x9a\x84\xe6\x94\xb6\xe5\x85\xa5\xe5\x88\x86\xe9\x85\x8d\xe8\xb0\x83\xe8\x8a\x82\xe6\x96\xb9\xe6\xa1\x88\x20\xe5\xb7\xb2\xe7\xbb\x8f\xe4\xb8\x8a\xe6\x8a\xa5\xe5\x9b\xbd\xe5\x8a\xa1\xe9\x99\xa2\xe5\xbe\x85\xe6\x89\xb9"
    },


    // japanese text - TODO
    { L"\x65e5\x672c\x8a9e",
      "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e"
    },

    { L"\x31\x30\x6708\x32\x35\x65e5\xff08\x65e5\x66dc\x65e5\xff09\x3001",
      "\x31\x30\xe6\x9c\x88\x32\x35\xe6\x97\xa5\xef\xbc\x88\xe6\x97\xa5\xe6\x9b\x9c\xe6\x97\xa5\xef\xbc\x89\xe3\x80\x81"
    },

    { L"\x30a4\x30e9\x30af\x30fb\x30d0\x30b0\x30c0\x30c3\x30c9\x306b\x304a\x3044\x3066\x9023\x7d9a\x7206\x767a\x30c6\x30ed\x304c\x767a\x751f\x3057\x3001",
      "\xe3\x82\xa4\xe3\x83\xa9\xe3\x82\xaf\xe3\x83\xbb\xe3\x83\x90\xe3\x82\xb0\xe3\x83\x80\xe3\x83\x83\xe3\x83\x89\xe3\x81\xab\xe3\x81\x8a\xe3\x81\x84\xe3\x81\xa6\xe9\x80\xa3\xe7\xb6\x9a\xe7\x88\x86\xe7\x99\xba\xe3\x83\x86\xe3\x83\xad\xe3\x81\x8c\xe7\x99\xba\xe7\x94\x9f\xe3\x81\x97\xe3\x80\x81"
    },

    { L"\x6570\x767e\x540d\x306e\x6b7b\x50b7\x8005\x304c\x51fa\x305f\x3053\x3068\x306b\x3001\x6211\x304c\x56fd\x306f\x5f37\x3044\x885d\x6483\x3068\x61a4\x308a\x3092\x899a\x3048\x307e\x3059\x3002",
      "\xe6\x95\xb0\xe7\x99\xbe\xe5\x90\x8d\xe3\x81\xae\xe6\xad\xbb\xe5\x82\xb7\xe8\x80\x85\xe3\x81\x8c\xe5\x87\xba\xe3\x81\x9f\xe3\x81\x93\xe3\x81\xa8\xe3\x81\xab\xe3\x80\x81\xe6\x88\x91\xe3\x81\x8c\xe5\x9b\xbd\xe3\x81\xaf\xe5\xbc\xb7\xe3\x81\x84\xe8\xa1\x9d\xe6\x92\x83\xe3\x81\xa8\xe6\x86\xa4\xe3\x82\x8a\xe3\x82\x92\xe8\xa6\x9a\xe3\x81\x88\xe3\x81\xbe\xe3\x81\x99\xe3\x80\x82"
    },
    

    // END OF TESTS
    { NULL,
      NULL 
    }
};
