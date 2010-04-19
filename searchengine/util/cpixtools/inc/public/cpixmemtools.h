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
/*
 * cpixmemorytools.h
 *
 *  Created on: 23.4.2009
 *      Author: arau
 */

#ifndef CPIXMEMORYTOOLS_H_
#define CPIXMEMORYTOOLS_H_

#include <list>
#include <vector>
#include <memory>

namespace Cpt
{

    /**
     * Stripped down implementation of the std::vector, 
     * which holds ownership to members and releases the owned instances,
     * when destroyed. 
     */
    template<typename T>
    class auto_vector : public std::vector<T*> {
    private: 
    	// pure copy constructor is not supported.
    	// copy method will empty the given vector from all members
		auto_vector(const auto_vector &vector);
        auto_vector& operator=(const auto_vector &vector);

    public: 
    	// transfers all the objects to 
        auto_vector(auto_vector &vector);
  
        auto_vector& operator=(auto_vector &vector);
   
    	auto_vector(); 
    	
    	/**
    	 * Destructor. Destroys all contained members
    	 */
    	~auto_vector(); 
    	
    	/**
    	 * Like push_back, except that it transfers ownership throught auto_ptr
    	 */
    	void donate_back(std::auto_ptr<T> ptr); 
    	
    	/**
    	 * Like pop_back, except that it transfers ownership throught auto_ptr
    	 */
    	std::auto_ptr<T> release_back(); 
    };

    template<typename T>
    class poly_forward_iterator {
    public: 
    	virtual ~poly_forward_iterator() {}
		virtual T operator++(int) = 0; 
		virtual operator bool() = 0;
    };
    
    
    template<typename T, typename I>
	class auto_iterator_ref {
	public: 
		auto_iterator_ref(I* p) : p_(p) {} 
		I* p_;
	};
    
	template<typename T, typename I>
	class auto_iterator {
	public: 
		inline explicit auto_iterator(I* iterator) : iterator_(iterator) {}; 
		inline ~auto_iterator() 			{}
		inline T operator++(int) 			{ return (*iterator_)++; }
		inline operator bool()  			{ return *iterator_; } 
	
		// special conversions enabling returning as return value
		inline auto_iterator(auto_iterator_ref<T, I> ref) throw()
		  : iterator_(ref.p_) {}
		operator auto_iterator_ref<T, I>() throw() {
			return auto_iterator_ref<T, I>(iterator_.release());
	    }
	private: 
		std::auto_ptr<I> iterator_;
	};
	
}

/**************************************************************
 *
 *  IMPLEMENTATION OF TEMPLATES
 *
 */
namespace Cpt
{

	template<typename T>
	auto_vector<T>::auto_vector() {}
	
	template<typename T>
	auto_vector<T>::~auto_vector() 
	{
		for (int i = 0; i < this->size(); i++) delete (*this)[i]; 
	}
	
	template<typename T>
	auto_vector<T>::auto_vector(auto_vector<T> &vector) 
	{
		// without using fororder, the order would be inversed
		operator=(vector); 
	}
	
	
	template<typename T>
	auto_vector<T>& auto_vector<T>::operator=(auto_vector<T> &vector) 
	{
		auto_vector<T> fororder; 
		while (!vector.empty()) fororder.donate_back(vector.release_back()); 
		while (!fororder.empty()) donate_back(fororder.release_back()); 
		return *this;
	}
	
	/**
	 * Grants the ownership of the item for this vector
	 */
	template<typename T>
	void auto_vector<T>::donate_back(std::auto_ptr<T> ptr) 
	{
		this->push_back(ptr.release()); 
	}
		
	/**
	 * Releases the ownership of the item
	 */
	template<typename T>
	std::auto_ptr<T> auto_vector<T>::release_back() 
	{
		T* v = this->back();
		std::auto_ptr<T> ret(v); 
		this->pop_back(); 
		return ret;
	}

}

#endif /* CPIXMEMORYTOOLS_H_ */
