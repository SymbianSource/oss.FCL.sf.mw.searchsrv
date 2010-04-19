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
#ifndef CPIXTOOLS_CPIXSTRTOOLS_H_
#define CPIXTOOLS_CPIXSTRTOOLS_H_

#include <list>
#include <string>

namespace Cpt
{


    /**
     * Breaks down a string on certain delimiters.
     *
     * @param orig the original string
     *
     * @param delimiters the zero terminated string containing the
     * delimiter characters
     *
     * @param target the list to put the substrings into
     */
    void splitstring(const char             * orig,
                     const char             * delimiters,
                     std::list<std::string> & target);


    /**
     * @param first the first string - this should be the substring of
     * the second or equal to it
     *
     * @param second the second string
     *
     * @return true if the first string is the substring of the second
     * or equal to it, false otherwise
     */
    template<typename CTYPE>
    bool issubstroforequal(const CTYPE * first,
                           const CTYPE * second)
    {
        for ( ; 
              *first != static_cast<CTYPE>(0) 
                  && *second != static_cast<CTYPE>(0) 
                  && *first == *second;
              ++first, ++second)
            ;
        
        return *first == static_cast<CTYPE>(0);
    }


    /**
     * Poor man's substitute to OpenC's leaking printf("%f") formatting 
     * function, because printf float formatting leak memory. 
     * 
     * NOTE: Last number WILL NOT be rounded properly. It will 
     * be rounded up, when it should be rounded to closest.
     */
    int wsnprintdouble(wchar_t* buf, size_t n, double number, int decimals);



    /**
     * Exception class to tell about conversion failures.
     */
    class ConversionExc : public std::exception
    {
    private:
        //
        // private members
        //

        std::string    what_;

    public:
        //
        // public operators
        //

        /**
         * Overriden from std::exception
         */
        virtual const char * what() const throw();


        //
        // Lifecycle management
        //
            
        /**
         * Constructs this exception with printf style format string
         * and potential detail arguments.
         */
        ConversionExc(const char * format,
                      ...);
    };



    /**
     * A very simple lifetime mgmt class for arrays. (std::auto_ptr
     * should not be used on C-strings or arrays of a type, as it uses
     * delete and not delete[] to clean up.
     */
    template<typename T>
    class auto_array
    {
    private:
        //
        // private members
        //
        T * p_;


        // no value semantics
        auto_array(const auto_array &);
        auto_array & operator=(const auto_array &);

    public:
        //
        // public operators
        //

        
        /**
         * Gives up ownership of stored array to the caller.
         *
         * @returns the pointer to the array - the caller will have to
         * manage it from now on.
         */
        T * release();


        /**
         * Resets the array owned by this, any old arrays are
         * discarded.
         */
        void reset(T * p);
        

        /**
         * @returns the stored array - ownership is not transferred
         */
        T * get();


        /**
         * Default constructor
         */
        auto_array();


        /**
         * Constructor taking ownership of given array pointer by p.
         */
        auto_array(T * p);



        /**
         * Named constant for the default (auto) case for string
         * length parameter in the conversion constructor.
         *
         * The value should be max of size_t, and unfortunately
         * std::numeric_limits<size_t>::max() is no use here as it is
         * a function providing a value at runtime and not a constant
         * expression you can use at compile time. So much about the
         * usefulness of numeric_limits. So, using the fact that
         * size_t is always an unsigned type, we are going to use -1
         * cast to size_t as MAX. Bit obscure but should provide the
         * numerical value for the maximal unsigned number size_t can
         * represent.
         */
        enum { AUTO_STRLEN = static_cast<size_t>(-1) };

        
        /**
         * Constructor for the case when both T and CHAR are character
         * types but they are not the same (wchar_t, char).
         *
         * In this case, auto_array will behave as a simple string
         * class that was initialized by copying and converting an
         * original string of different character type.
         *
         * @param src a c-style string (char or wchar_t).
         *
         * @param length if not AUTO_STRLEN (default value), then
         * min(length, STRLEN(src)) amount of characters will be
         * copied to this auto_array instance. STRLEN(src) means
         * either strlen(src) or wcslen(src) depending on whether CHAR
         * is plain or wide character.
         *
         * NOTE: T has to be also char or wchar_t.
         *
         * NOTE: DO NOT attempt to use this constructor in the case
         * where the types T and CHAR are the same. In that case, the
         * overloaded constructor will be applied (cf above) by the
         * C++ compiler and that has completely different ownership
         * semantics: it does not copy (and convert), it takes
         * ownership of given string!
         *
         * @throws ConversionExc if conversion fails for some reason
         * (invalid byte sequences etc)
         */
        template<typename CHAR>
        explicit auto_array(const CHAR * src,
                            size_t       length = AUTO_STRLEN);
        


        /**
         * Destructor
         */
        ~auto_array();
    };


    /**
     * @return the used dynamic memory in bytes.
     */
    uint32_t getUsedDynamicMemory();



    /*******************************************************
     *
     *  Conversion utilities (wide string -> value types)
     *
     */

    /**
     * Attempts to convert the given wide string to a value. For
     * integer types, this will be equivalent to wconvertInteger call.
     *
     * @param to the pointer to the value to put the results, must not
     * be NULL
     *
     * @param fromStr wide string, must not be NULL, and it must
     * contain only the string representation for the value, nothing
     * more. Failure to eat all of the string and put it to the value
     * shall result in error.
     *
     * @return NULL if successful, and some string about what sort of
     * value/formatting was attempted on failure.
     */
    template<typename V>
    const char * wconvert(V             * to,
                          const wchar_t * fromStr);


    /**
     * Attempts to convert a given wide string to an integer type
     * (char, short, integer, long, long long). The string is
     * interpreted in decimal notation.
     *
     * @param to the pointer to the value to put the results, must not
     * be NULL
     *
     * @param fromStr wide string, must not be NULL, and it must
     * contain only the string representation for the value, nothing
     * more. Failure to eat all of the string and put it to the value
     * shall result in error.
     *
     * @return NULL if successful, and some string about what sort of
     * value/formatting was attempted on failure.
     */
    template<typename INT>
    const char * wconvertDecimal(INT           * to,
                                 const wchar_t * fromStr);


    /**
     * Attempts to convert a given wide string to an integer type
     * (char, short, integer, long, long long). The string is
     * interpreted generally: 0x prefix will cause hexadecimal
     * interpretation, 0 prefix octal and otherwise decimal base will
     * be used.
     *
     * @param to the pointer to the value to put the results, must not
     * be NULL
     *
     * @param fromStr wide string, must not be NULL, and it must
     * contain only the string representation for the value, nothing
     * more. Failure to eat all of the string and put it to the value
     * shall result in error.
     *
     * @return NULL if successful, and some string about what sort of
     * value/formatting was attempted on failure.
     */
    template<typename INT>
    const char * wconvertInteger(INT           * to,
                                 const wchar_t * fromStr);


    /**
     * Attempts to convert a given wide string to an unsigned integer
     * type. Octal base is used.
     *
     * @param to the pointer to the value to put the results, must not
     * be NULL
     *
     * @param fromStr wide string, must not be NULL, and it must
     * contain only the string representation for the value, nothing
     * more. Failure to eat all of the string and put it to the value
     * shall result in error.
     *
     * @return NULL if successful, and some string about what sort of
     * value/formatting was attempted on failure.
     */
    template<typename INT>
    const char * wconvertOctal(INT           * to,
                               const wchar_t * fromStr);


    /**
     * Attempts to convert a given wide string to an unsigned integer
     * type. Decimal base is used.
     *
     * @param to the pointer to the value to put the results, must not
     * be NULL
     *
     * @param fromStr wide string, must not be NULL, and it must
     * contain only the string representation for the value, nothing
     * more. Failure to eat all of the string and put it to the value
     * shall result in error.
     *
     * @return NULL if successful, and some string about what sort of
     * value/formatting was attempted on failure.
     */
    template<typename INT>
    const char * wconvertUnsignedInteger(INT           * to,
                                         const wchar_t * fromStr);

    /**
     * Attempts to convert a given wide string to an unsigned integer
     * type. Hexadecimal base is used.
     *
     * @param to the pointer to the value to put the results, must not
     * be NULL
     *
     * @param fromStr wide string, must not be NULL, and it must
     * contain only the string representation for the value, nothing
     * more. Failure to eat all of the string and put it to the value
     * shall result in error.
     *
     * @return NULL if successful, and some string about what sort of
     * value/formatting was attempted on failure.
     */
    template<typename INT>
    const char * wconvertHexadecimal(INT           * to,
                                     const wchar_t * fromStr);

}




/**************************************************************
 *
 *  IMPLEMENTATION OF TEMPLATES
 *
 */
namespace Cpt
{

    //////////////////////////////////////////////////////////
    //
    // auto_array
    //
    template<typename T>
    T * auto_array<T>::release()
    {
        T
            * rv = p_;

        p_ = NULL;

        return rv;
    }


    template<typename T>
    void auto_array<T>::reset(T * p)
    {
        if (p_ != p)
            {
                delete[] p_;
                p_ = p;
            }
    }
        

    template<typename T>
    T * auto_array<T>::get()
    {
        return p_;
    }


    template<typename T>
    auto_array<T>::auto_array()
        : p_(NULL)
    {
        ;
    }


    template<typename T>
    auto_array<T>::auto_array(T * p)
        : p_(p)
    {
        ;
    }


    //
    // namespace for auto_array copy-and-convert-string ctor impl
    // details
    namespace Impl
    {
        template<typename T>
        struct AssertCharType
        {
            ~AssertCharType()
            {
                // For non-character types, we don't support the
                // special auto_array templated conversion
                // constructor. For decent compilers, a compile time
                // error can be generated, for non-decent compilers
                // like the old ARMCC version we use, we have to do
                // with a runtime exception generated.
#ifdef __ARMCC__
                throw ConversionExc("PANIC auto_array conversion constructor is used for non-char type %s",
                                    typeid(T).name());
#else
                compile_error(Template_type_T_is_not_a_character_type);
#endif
            }
        };


        template<>
        struct AssertCharType<char>
        {
            ~AssertCharType()
            {
                ;
            }
        };


        template<>
        struct AssertCharType<wchar_t>
        {
            ~AssertCharType()
            {
                ;
            }
        };


        /**
         * This template class (or rather, some of its specializations
         * for enabled types) will create a new buffer, copy an
         * original string into it converting.
         *
         * Allocation happens with new[] (you need to use delete[]).
         */
        template<typename DST_CHAR,
                 typename SRC_CHAR>
        struct CopyConverter
        {
        };


        /**
         * The OpenC wcstombs is impossible to use correctly AND
         * effectively (RAM consumptionwise) at the same time. It does
         * not tell how much original wchar_t-s it has consumed, and
         * it will not put a terminating zero at the end if there is
         * not place left. So we need our own (non-trivial) logic.
         *
         * Copies and converts the whole src string, and sets the dst
         * to the newly acquired buffer.
         *
         * @throws ConversionExc
         */
        void ProperWcsToMbs(char            * & dst,
                            const wchar_t     * src,
                            size_t              length);


        /**
         * Copies and converts the whole src string, and sets the dst
         * to the newly acquired buffer.
         *
         * @throws ConversionExc
         */
        void ProperMbsToWcs(wchar_t         * & dst,
                            const char        * src,
                            size_t              length);


        template<>
        struct CopyConverter<char, wchar_t>
        {
            static void CopyConvert(char            * & dst,
                                    const wchar_t     * src,
                                    size_t              length)
                {
                    ProperWcsToMbs(dst,
                                   src,
                                   length);
                }
        };


        template<>
        struct CopyConverter<wchar_t, char>
        {
            static void CopyConvert(wchar_t         * & dst,
                                    const char        * src,
                                    size_t              length)
                {
                    ProperMbsToWcs(dst,
                                   src,
                                   length);
                }
        };

    } // ns


    
    template<typename T>
    template<typename CHAR>
    auto_array<T>::auto_array(const CHAR * src,
                              size_t       length)
        : p_(NULL)
    {
        Impl::AssertCharType<T>
            a1;
        Impl::AssertCharType<CHAR>
            a2;

        Impl::CopyConverter<T, CHAR>::CopyConvert(p_,
                                                  src,
                                                  length);
    }



    template<typename T>
    auto_array<T>::~auto_array()
    {
        reset(NULL);
    }


    //////////////////////////////////////////////////////////
    //
    // conversion functions
    //


    /**
     * Implementation detail struct storing the conversion string for
     * scanf type of functions and the displayname of the type being
     * converted to (for error messages).
     */
    struct ValueType
    {
        const wchar_t *    formatStr_;
        const char    *    displayName_;
    };


    /**
     * Table of value-type structs specifying the conversion format
     * strings and display names.
     */
    extern const ValueType ValueTypes[];


    /************
     *
     * The table ValueTypes contains format specifications, but for a
     * typename-inputformat pair (V,INPUTFORMAT) we have to know what
     * is the index in the ValueTypes table where we can find the
     * format string and the display name for that tuple.
     *
     * For instance, format string for floats and int will look
     * different, as well well format strings for integers in decimal
     * notations and integers in hexadecimal notations will differ.
     */
    template<typename V,
             int      INPUTFORMAT>
    struct ValueTypeIndex
    {
        // If during compilation, you find that there is no IDX
        // member, then it means that you are attempting to convert a
        // value type - inputformat tuple that is not supported.
        //
        // There are two cases.
        //
        // It can be so, that you tuple (V,INPUTFORMAT) does not make
        // any sense. For instance, scanf expects unsigned integers
        // for hecadecimal strings, so you can't dream to put there
        // signed ints or floats.
        //
        // The other case is that the conversion is okay, but it is
        // just not enabled / supported yet. Steps for enabling:
        //
        //   1 Go to definition of valuetypes in cpixstrtools.cpp, and
        //     add a new entry with the proper scanf format and
        //     display name. NOTE: there will be two conversion
        //     specifiers, first the ones you define and the second a
        //     mandatory %n.
        //
        //   2 Define a template specialication below for your tuple
        //     and define the IDX enum value to be the index of your
        //     new entry in the ValueTypes table.
        //
        // enum { IDX = ??? };
    };


    /**
     * In the case of integer value types (char, short, int, long,
     * long long, and their unsigned counterpart), there are several
     * formats the string can use, these can be controlled with the
     * enums here.
     *
     * Most types will just used the default (INPUTFORMAT_DEFAULT).
     */
    enum InputFormat
        {
            // default, for integer types it is equivalent to
            // INPUTFORMAT_INTEGER
            INPUTFORMAT_DEFAULT     = 0,

            // target variable is signed
            INPUTFORMAT_DECIMAL     = 1,

            // target variable is signed
            INPUTFORMAT_INTEGER     = 2,

            // target variable is unsigned
            INPUTFORMAT_OCTAL       = 3,

            // target variable is unsigned
            INPUTFORMAT_UINTEGER    = 4,

            // target variable is unsigned
            INPUTFORMAT_HEXADECIMAL = 5
        };


    template<>
    struct ValueTypeIndex<int,
                          int(INPUTFORMAT_DEFAULT)>
    {
        enum { IDX = 0 };
    };
    
    template<>
    struct ValueTypeIndex<int,
                          int(INPUTFORMAT_DECIMAL)>
    {
        enum { IDX = 1 };
    };

    template<>
    struct ValueTypeIndex<int,
                          int(INPUTFORMAT_INTEGER)>
    {
        enum { IDX = 0 };
    };

    template<>
    struct ValueTypeIndex<unsigned int,
                          int(INPUTFORMAT_OCTAL)>
    {
        enum { IDX = 2 };
    };

    template<>
    struct ValueTypeIndex<unsigned int,
                          int(INPUTFORMAT_UINTEGER)>
    {
        enum { IDX = 3 };
    };

    template<>
    struct ValueTypeIndex<unsigned int,
                          int(INPUTFORMAT_HEXADECIMAL)>
    {
        enum { IDX = 4 };
    };


    template<>
    struct ValueTypeIndex<float,
                          int(INPUTFORMAT_DEFAULT)>
    {
        enum { IDX = 5 };
    };

    template<>
    struct ValueTypeIndex<double,
                          int(INPUTFORMAT_DEFAULT)>
    {
        enum { IDX = 6 };
    };



    /**
     * Attempts to convert the given wide string to the value. This is
     * the common workhorse function for all conversions.
     *
     * The template parameter INPUTFORMAT can be one of the
     * InputFormat enums. Mostly, you want to use INPUTFORMAT_DEFAULT
     * (cf overloaded convert function). If you are dealing with
     * integers and special formatted values (octal, hex, etc ...),
     * you can use the enum values above.
     *
     * NOTE: some of the input enum values imply signed, other imply
     * unsigned value. Trying to use the wrong type of variable for
     * 'to' will result in compilation error.
     *
     * @param to the pointer to the value to put the results, must not
     * be NULL
     *
     * @param wide string, must not be NULL, and it must contain only
     * the string representation for the value, nothing more. Failure
     * to eat all of the string and put it to the value shall result
     * in error.
     *
     * @return NULL if successful, and some string about what sort of
     * value/formatting was attempted on failure.
     */
    template<typename V,
             int      INPUTFORMAT>
    const char * wconvert(V             * to,
                          const wchar_t * fromStr)
    {
        const char
            * rv = NULL;

        int
            valueTypeIndex = ValueTypeIndex<V, INPUTFORMAT>::IDX;

        const ValueType
            & valueType = ValueTypes[valueTypeIndex];

        int
            charsRead = 0;
        int
            result = swscanf(fromStr,
                             valueType.formatStr_,
                             to,
                             &charsRead);

        if (result != 1
            || charsRead != (*wcslen)(fromStr))
            {
                rv = valueType.displayName_;
            }

        return rv;
    }


    template<typename V>
    const char * wconvert(V             * to,
                          const wchar_t * fromStr)
    {
        return wconvert<V, INPUTFORMAT_DEFAULT>(to,
                                                fromStr);
    }


    template<typename INT>
    const char * wconvertDecimal(INT           * to,
                                 const wchar_t * fromStr)
    {
        return wconvert<INT, INPUTFORMAT_DECIMAL>(to,
                                                  fromStr);
    }


    template<typename INT>
    const char * wconvertInteger(INT           * to,
                                 const wchar_t * fromStr)
    {
        return wconvert<INT, INPUTFORMAT_INTEGER>(to,
                                                  fromStr);
    }


    template<typename INT>
    const char * wconvertOctal(INT           * to,
                               const wchar_t * fromStr)
    {
        return wconvert<INT, INPUTFORMAT_OCTAL>(to,
                                                fromStr);
    }


    template<typename INT>
    const char * wconvertUnsignedInteger(INT           * to,
                                         const wchar_t * fromStr)
    {
        return wconvert<INT, INPUTFORMAT_UINTEGER>(to,
                                                   fromStr);
    }

    template<typename INT>
    const char * wconvertHexadecimal(INT           * to,
                                     const wchar_t * fromStr)
    {
        return wconvert<INT, INPUTFORMAT_HEXADECIMAL>(to,
                                                      fromStr);
    }

    
        
}


#endif




