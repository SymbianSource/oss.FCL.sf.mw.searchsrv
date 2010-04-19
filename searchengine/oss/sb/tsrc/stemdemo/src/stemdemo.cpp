/*
 ============================================================================
 Name        : stemdemo.cpp
 Author      : 
 
 Description : Simple sample program demonstrating libstemmer library 
               (from snowball)
 ============================================================================
 */

/*

Copyright (c) 2001, Dr Martin Porter
Copyright (c) 2002, Richard Boulton
All rights reserved.

Redistribution and use in source and binary forms, with 
or without modification, are permitted provided that the 
following conditions are met:

*	Redistributions of source code must retain the 
	above copyright notice, this list of conditions and 
	the following disclaimer. 
*	Redistributions in binary form must reproduce 
	above copyright notice, this list of conditions and
 	the following disclaimer in the documentation and/or 
	other materials provided with the distribution. 
*	Neither the name of the <ORGANIZATION> nor the 
	names of its contributors may be used to endorse or 
	promote products derived from this software without
	specific rior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE.

*/

// INCLUDE FILES
// OBS #include <stdio.h>

// OBS #include <stdlib.h> /* for malloc, free */
// OBS #include <string.h> /* for memmove */
// OBS #include <ctype.h>  /* for isupper, tolower */

#include <wchar.h>



#include <iostream>

#include <iterator>
#include <algorithm>
#include <string>
#include <vector>

#include "libstemmer.h"

// This is a GCCE toolchain workaround needed when compiling with GCCE
// and using main() entry point
#ifdef __GCCE__
#include <staticlibinit_gcce.h>
#endif


template<typename OI>
class PunctuationFilterIterator 
    : public std::iterator<std::output_iterator_tag,
                           std::string>
{
private:
    OI   oi_;

public:
    PunctuationFilterIterator(OI oi)
        : oi_(oi)
    {
        ;
    }

    
    PunctuationFilterIterator & operator*()
    {
        return *this;
    }


    PunctuationFilterIterator & operator=(const std::string & s)
    {
        using namespace std;

        static const char
            * punctuators = ".,;:?!-";

        string::size_type
            last = s.find_last_not_of(punctuators);
        if (last == string::npos)
            {
                if (s.length() > 0
                    && s.find_first_of(punctuators) != 0)
                    {
                        *oi_ = s;
                    }
            }
        else //if (last >= 0)
            {
                string
                    strToStore = s.substr(0,
                                          last + 1);
                if (strToStore.length() > 0)
                    {
                        *oi_ = strToStore;
                    }
            }
        return *this;
    }

    PunctuationFilterIterator & operator++() 
    {
        return *this;
    }


    PunctuationFilterIterator operator++(int)
    {
        return *this;
    }
    
};


template<typename OI>
PunctuationFilterIterator<OI> PunctuationFilter(OI oi)
{
    return PunctuationFilterIterator<OI>(oi);
}


template<typename OI>
class SideEffectPrinterIterator
    : public std::iterator<std::output_iterator_tag,
                           void, void, void, void>
{
private:
    std::ostream  * os_;
    OI              oi_;


public:
    SideEffectPrinterIterator(std::ostream & os,
                              OI             oi)
        : os_(&os),
          oi_(oi)
    {
        ;
    }


    SideEffectPrinterIterator & operator*()
    {
        return *this;
    }


    template<typename T>
    SideEffectPrinterIterator & operator=(const T & v)
    {
        *os_ << "Storing string: '" << v << "'" << std::endl;
        *oi_ = v;
        return *this;
    }

    
    SideEffectPrinterIterator & operator++()
    {
        return *this;
    }


    SideEffectPrinterIterator operator++(int)
    {
        return *this;
    }

};


template<typename OI>
SideEffectPrinterIterator<OI> SideEffectPrinter(std::ostream & os,
                                                OI             oi)
{
    return SideEffectPrinterIterator<OI>(os,
                                         oi);
}



template<typename OI>
class CodePageCheckerIterator
    : public std::iterator<std::output_iterator_tag,
                           std::string>
{
private:
    OI           oi_;
    const char * extraChars_;
    size_t       length_;
    bool         extendingAscii_;

public:
    /**
     * @param extraChars is not copied - must exist during
     * the lifetime of this instance
     */
    CodePageCheckerIterator(OI           oi,
                            const char * extraChars,
                            bool         extendingAscii)
        : oi_(oi),
          extraChars_(extraChars),
          length_(0),
          extendingAscii_(extendingAscii)
    {
        if (extraChars != 0)
            {
                length_ = strlen(extraChars);
            }
    }


    CodePageCheckerIterator & operator*()
    {
        return *this;
    }


    CodePageCheckerIterator & operator=(const std::string & s)
    {
        using namespace std;

        string::const_iterator
            i = s.begin(),
            end = s.end();

        bool
            failed = false;

        // TODO brute force, not optimal algorithm - is there any better?
        for (; i != end; ++i)
            {
                if (!extendingAscii_ || (static_cast<unsigned char>(*i) 
                                         > static_cast<unsigned char>('\x7f')))
                    {
                        failed = true;
                        size_t
                            j = 0;
                        for (; j < length_; ++j)
                            {
                                if (*i == extraChars_[j])
                                    {
                                        failed = false;
                                        break;
                                    }
                            }
                    }

                if (failed)
                    {
                        break;
                    }
            }

        if (failed)
            {
                cout 
                    << "!!! SUSPICIOUS encoding in '"
                    << s
                    << "' at position "
                    << (i - s.begin())
                    << endl;
            }

        *oi_ = s;
        return *this;
    }


    CodePageCheckerIterator & operator++()
    {
        return *this;
    }


    CodePageCheckerIterator operator++(int)
    {
        return *this;
    }
    
};



template<typename OI>
CodePageCheckerIterator<OI> CodePageChecker(OI           oi,
                                            const char * extraChars,
                                            bool         extendingAscii)
{
    return CodePageCheckerIterator<OI>(oi,
                                       extraChars,
                                       extendingAscii);
}


const char FINNISH_EXTRA_CHARS_ISO_8859_1[] = { 
    '\xe4', // ä
    '\xf6', // ö
    0
};

const char HUNGARIAN_EXTRA_CHARS_ISO_8859_1[] = { 
    '\xe1', // a'
    '\xe9', // e'
    '\xed', // i'
    '\xf3', // o'
    '\xf6', // o"
    '\xf5', // oq
    '\xfa', // u'
    '\xfc', // u"
    '\xfb', // uq
    0
};

const char RUSSIAN_EXTRA_CHARS_KOI8_R[] = {
    '\xC1', // a
    '\xC2', // b
    '\xD7', // v
    '\xC7', // g
    '\xC4', // d
    '\xC5', // e
    '\xD6', // zh
    '\xDA', // z
    '\xC9', // i
    '\xCA', // i'
    '\xCB', // k
    '\xCC', // l
    '\xCD', // m
    '\xCE', // n
    '\xCF', // o
    '\xD0', // p
    '\xD2', // r
    '\xD3', // s
    '\xD4', // t
    '\xD5', // u
    '\xC6', // f
    '\xC8', // kh
    '\xC3', // ts
    '\xDE', // ch
    '\xDB', // sh
    '\xDD', // shch
    '\xDF', // "
    '\xD9', // y
    '\xD8', // '
    '\xDC', // e'
    '\xC0', // iu
    '\xD1', // ia
    0
};


const char ENGLISH_EXTRA_CHARS[] = {
    0
};


struct LangCfg {
    const char * cmd_;
    const char * name_;
    const char * extraChars_;
    const char * charEncoding_;
    bool         extendingAscii_;

    LangCfg(const char * cmd,
            const char * name,
            const char * extraChars,
            const char * charEncoding,
            bool         extendingAscii)
        : cmd_(cmd),
          name_(name),
          extraChars_(extraChars),
          charEncoding_(charEncoding),
          extendingAscii_(extendingAscii)
    {
        ;
    }


    void print(std::ostream & os) const
    {
        using namespace std;

        os << " Language Configuration: " << cmd_ << endl;
        os << "  o  " << name_ << endl;
        if (charEncoding_ != NULL)
            os << "  o  " << charEncoding_ << endl;
    }
};


LangCfg Languages[] = {
    LangCfg("en",
            "English",
            ENGLISH_EXTRA_CHARS,
            NULL,
            true),

    LangCfg("fi",
            "Finnish",
            FINNISH_EXTRA_CHARS_ISO_8859_1,
            "ISO_8859_1",
            true),

    LangCfg("hu",
            "Hungarian",
            HUNGARIAN_EXTRA_CHARS_ISO_8859_1,
            "ISO_8859_1",
            true),

    LangCfg("ru",
            "Russian",
            RUSSIAN_EXTRA_CHARS_KOI8_R,
            "KOI8_R",
            false)
      
};


const char TESTING_CMD[] = "--testing";

const char EXIT_CMD[]    = "x";


void usage(const char * exeName)
{
    using namespace std;

    cout << "Usage: " << endl;
    cout << exeName << " (";

    for (int i = 0; i < sizeof(Languages) / sizeof(LangCfg); ++i)
        {
            if (i > 0)
                {
                    cout << "|";
                }

            cout << Languages[i].cmd_;
        }

    cout << ") [" << TESTING_CMD << "]" << endl;
    cout << "Supported language configurations:" << endl;

    for (int i = 0; i < sizeof(Languages) / sizeof(LangCfg); ++i)
        {
            Languages[i].print(cout);
        }
    cout << endl;
    cout << "When in testing mode (--testing), enter the string '"
         << EXIT_CMD
         << "' to complete typing words"
         << endl;
}


template<typename II,
         typename OI>
void copy_until(II                                            first,
                II                                            last,
                OI                                            out,
                typename std::iterator_traits<II>::value_type exit)
{
    while (first != last)
        {
            typename std::iterator_traits<II>::value_type
                current = *first;

            if (current == exit) 
                {
                    break;
                }

            *out = current;
            ++out;
            ++first;
        }
}


void readSilently(const LangCfg            * langCfg,
                  std::vector<std::string> & strings)
{
    using namespace std;

    copy_until(istream_iterator<string>(cin),
               istream_iterator<string>(),
               PunctuationFilter(CodePageChecker(back_inserter(strings),
                                                 langCfg->extraChars_,
                                                 langCfg->extendingAscii_)
                                 ),
               EXIT_CMD
         );
}


void readTesting(const LangCfg            * langCfg,
                 std::vector<std::string> & strings)
{
    using namespace std;

    copy_until(istream_iterator<string>(cin),
               istream_iterator<string>(),
               PunctuationFilter(CodePageChecker(SideEffectPrinter(cout,
                                                                   back_inserter(strings)),
                                                 langCfg->extraChars_,
                                                 langCfg->extendingAscii_)
                                 ),
               EXIT_CMD
         );

}


void printAndAlign(const std::string & text)
{
    const int COLWIDTH = 32;

    using namespace std;
    
    cout << text;
    for (int i = COLWIDTH - text.length(); i > 0; --i)
        {
            cout << ' ';
        }
}


int stemAll(const LangCfg            * langCfg,
            std::vector<std::string> & allStrings)
{
    using namespace std;

    sort(allStrings.begin(),
         allStrings.end());

    vector<string>
        strings;

    unique_copy(allStrings.begin(),
                allStrings.end(),
                back_inserter(strings));

    struct sb_stemmer
        * stemmer = sb_stemmer_new(langCfg->cmd_,
                                   langCfg->charEncoding_);

    cout << "Number of words: " << strings.size() << endl;
    
    printAndAlign("ORIGINALS (sorted)");
    cout << "STEMS" << endl;
    printAndAlign("------------------");
    cout << "-----" << endl;

    vector<string>::const_iterator
        i = strings.begin(),
        end = strings.end();
    for (; i != end; ++i)
        {
            const sb_symbol
                * stemmed = sb_stemmer_stem(stemmer,
                                            reinterpret_cast<const sb_symbol*>(i->c_str()),
                                            i->length());

            if (stemmed == NULL)
                {
                    return -1;
                }

            printAndAlign(*i);
            cout << reinterpret_cast<const char*>(stemmed)
                 << endl;
        }

    sb_stemmer_delete(stemmer);

    return 0;
}


int process(const LangCfg * langCfg,
             bool            testing)
{
    using namespace std;

    langCfg->print(cout);
    cout << "Testing: " << (testing ? "yes" : "no") << endl;

    vector<string>
        strings;

    if (testing)
        {
            readTesting(langCfg,
                        strings);
        }
    else
        {
            readSilently(langCfg,
                         strings);
        }

    return stemAll(langCfg,
                   strings);
}


int main(int          argc,
         const char * argv[])
{
    using namespace std;

    if (argc > 3)
        {
            usage(argv[0]);
            exit(1);
        }

    for (int i = 0; i < argc; ++i)
        {
            printf("arg %d: %s\n",
                   i,
                   argv[i]);
        }

    LangCfg
        * langCfg = NULL;
    const char
        * cmd = argc > 1 ? argv[1] : "en";

    for (int i = 0; i < sizeof(Languages) / sizeof(LangCfg); ++i)
        {
            if (strcmp(Languages[i].cmd_, cmd) == 0)
                {
                    langCfg = Languages + i;
                    break;
                }
        }

    if (langCfg == NULL)
        {
            cout << "Unknown langauge: " << cmd << endl;
            usage(argv[0]);
            exit(2);
        }
    
    bool
        testing = false;

    if (argc == 3)
        {
        if (strcmp(argv[2], TESTING_CMD) == 0)
            {
                testing = true;
            }
        else
            {
                cout << "Unknown option: " << argv[2] << endl;
                usage(argv[0]);
                exit(3);
            }
        }

    
    int
        rv = process(langCfg,
                     testing);

    if (testing)
        {
            int
                c = getchar();
            c = getchar();
        }

    return rv;
}
