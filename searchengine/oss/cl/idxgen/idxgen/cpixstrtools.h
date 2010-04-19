/*------------------------------------------------------------------------------
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#ifndef CPIXTOOLS_CPIXSTRTOOLS_H_
#define CPIXTOOLS_CPIXSTRTOOLS_H_

#include <stdint.h>

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
                     char                   * delimiters,
                     std::list<std::string> & target);




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
         * @returns item i of the stored array
         */
        T& operator[](int i);

        /**
         * @returns item i of the stored array
         */
        const T& operator[](int i) const;


        /**
         * Default constructor
         */
        auto_array();


        /**
         * Constructor taking ownership of given array pointer by p.
         */
        auto_array(T * p);


        /**
         * Destructor
         */
        ~auto_array();
    };




    /**
     * @return the used dynamic memory in bytes.
     */
    // OBS uint32_t getUsedDynamicMemory();

}




/**************************************************************
 *
 *  IMPLEMENTATION OF TEMPLATES
 *
 */
namespace Cpt
{

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
    T & auto_array<T>::operator[](int i)
    {
        return p_[i];
    }

    template<typename T>
    const T & auto_array<T>::operator[](int i) const
    {
        return p_[i];
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


    template<typename T>
    auto_array<T>::~auto_array()
    {
        reset(NULL);
    }


}


#endif




