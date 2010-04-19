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
#include "cpixstrtools.h"
#include <e32std.h>


namespace Cpt
{

    void splitstring(const char             * orig,
                     const char             * delimiters,
                     std::list<std::string> & target)
    {
        using namespace std;

        target.clear();

        if (orig == NULL)
            {
                return;
            }

        const char
            * curBegin = orig,
            * curEnd = strpbrk(curBegin,
                               delimiters);

        while (curBegin != NULL)
            {
                if (curEnd == NULL)
                    {
                        target.push_back(string(curBegin,
                                                strlen(curBegin)));
                        curBegin = NULL;
                    }
                else
                    {
                        target.push_back(string(curBegin,
                                                curEnd));
                        curBegin = curEnd + 1;
                        if (*curBegin != 0)
                            {
                                curEnd = strpbrk(curBegin,
                                                 delimiters);
                            }
                        else
                            {
                                curBegin = NULL;
                            }
                    }
            } 
    }

    int wsnprintdouble(wchar_t* target, size_t n, double number, int decimals)
	{
		// NOTE, this printing mechanism is FAR from perfect
		// It tries to avoid integer/long overflowing, but 
		// at the same time it is vulnerable to double -> int 
		// conversion inaccuracies.
		int integer_part = static_cast<int>(number); 
		
		// print integer part and dot 
		int loc = 
			snwprintf(target, n, L"%d", integer_part);
		loc += snwprintf(target + loc, n-loc, L".");
		number -= integer_part; // Number is in form 0.12345
		number *= number < 0 ? -1. : 1.;  
				
		while (decimals-- > 0) {
			// extract one decimal out of number
			number*=10.; // Number is in form 1.234 
			int decimal = static_cast<int>(number);
			number -= decimal; // Returned into form 0.123
			
			loc += 
				snwprintf(target+loc, n-loc, L"%d", decimal); 
		}
		return loc; 
	}

    uint32_t getUsedDynamicMemory()
    {
        // TODO this is symbian specific code here

        TInt 
            largestBlock;
        TInt 
            mem = User::Heap().Size() - User::Heap().Available(largestBlock);

        return static_cast<uint32_t>(mem);
    }



    // conversion information for the convert... family of functions
    const ValueType ValueTypes[] = {
        
        // 0 : integer, %i
        { L"%i%n",       "integer (generic format)" },
            
        // 1 : integer, %d
        { L"%d%n",       "integer (decimal format)" },

        // 2 : unsigned integer, %o
        { L"%o%n",       "unsigned integer (octal format)" },

        // 3 : unsigned integer, %u
        { L"%u%n",       "unsigned integer (decimal)" },

        // 4 : unsigned integer, %x
        { L"%x%n",       "unsigned integer (hexadecimal format)" },

        // 5 : float, %f
        { L"%f%n",       "float" },

        // 6 : double, %lf
        { L"%lf%n",       "double" },

    };
    
}





//
// Implementation detail functions for proper, wchar_t -> char and
// char -> wchar_t copy-conversions
//
namespace Cpt
{
    //
    //
    // ConversionExc
    //
    //
    const char * ConversionExc::what() const throw()
    {
        return what_.c_str();
    }
    


    ConversionExc::ConversionExc(const char * format,
                                 ...)
    {
        char
            msg[96];

        va_list
            args;
        va_start(args,
                 format);

        vsnprintf(msg,
                  sizeof(msg),
                  format,
                  args);

        va_end(args);

        what_ = msg;
    }
        
        

    namespace Impl
    {
        //
        // Conversion functions
        //
        void ProperWcsToMbs(char            * & dst,
                            const wchar_t     * src,
                            size_t              length)
        {
            enum { INVALID_MB_PER_WC_RATE  = 0 };

            // we don't want to allocate MB_LEN_MAX number of bytes
            // for each wide character, because that many is certainly
            // not needed for western languages. (For instance,
            // Symbian example codes of OpenC usage only use 2 as a
            // multibyte-per-widechar factor, which is wrong
            // generally, as a unicode code point may require at most
            // 4 bytes in a utf8 sequence.)
            const static size_t
                MB_PER_WC_RATES[] = {
                // first we alloc 2 bytes for each wide char, as
                // recommended by symbian examples
                2,

                // then we use the theoretical max
                MB_LEN_MAX,

                // this should not happen - we should not need more
                // than the theoretical max
                INVALID_MB_PER_WC_RATE
            };
            
            const size_t
                * mbPerWcRate = MB_PER_WC_RATES;

            size_t
                srcSize = std::min(wcslen(src), length);

            const wchar_t
                * curSrc = src;

            size_t
                tmpSize = *mbPerWcRate * srcSize;
            auto_array<char>
                tmp(new char[tmpSize + 1]);

            char
                * curDst = tmp.get(),
                * curLastDst = tmp.get() + tmpSize;
            
            while (curSrc - src < srcSize)
                {
                    // a single wide-char can take up MB_LEN_MAX bytes
                    // at most: we have to make sure to have that
                    // amount of space
                    if (curLastDst - curDst >= MB_LEN_MAX)
                        {
                            int
                                result = wctomb(curDst,
                                                *curSrc);

                            if (result < 0)
                                {
                                    throw ConversionExc("Could not convert w string %S at position %d: %d",
                                                        src,
                                                        curSrc - src,
                                                        result);
                                }

                            curDst += result;

                            if (curLastDst <= curDst)
                                {
                                    throw ConversionExc("ASSERT wctomb() / MB_LEN_MAX(%d) are inconsistent",
                                                        MB_LEN_MAX);
                                }

                            ++curSrc;
                        }
                    else
                        {
                            ++mbPerWcRate;

                            if (*mbPerWcRate == INVALID_MB_PER_WC_RATE)
                                {
                                    throw ConversionExc("ASSERT Too many reallocs during wc->mb conversion, shouldn't happen: %S",
                                                        src);
                                }

                            size_t
                                newTmpSize = *mbPerWcRate * srcSize;
                            auto_array<char>
                                newTmp(new char[newTmpSize + 1]);
                            memcpy(newTmp.get(),
                                   tmp.get(),
                                   curDst - tmp.get());

                            curDst = newTmp.get() + (curDst - tmp.get());
                            curLastDst = newTmp.get() + newTmpSize;

                            tmp.reset(newTmp.release());
                            tmpSize = newTmpSize;
                        }
                }

            *curDst = '\0';

            dst = tmp.release();
        }



        void ProperMbsToWcs(wchar_t         * & dst,
                            const char        * src,
                            size_t              length)
        {
            size_t
                size = std::min(length, strlen(src)) + 1;

            dst = new wchar_t[size];

            int
                result = mbstowcs(dst,
                                  src,
                                  size);

            if (result < 0)
                {
                    throw ConversionExc("Could not convert %s: %d",
                                        src,
                                        errno);
                }
        }
        

    } // ns
} // ns
