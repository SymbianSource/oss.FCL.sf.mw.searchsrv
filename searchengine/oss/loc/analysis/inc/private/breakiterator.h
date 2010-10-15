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

#ifndef BREAKITERATOR_H_
#define BREAKITERATOR_H_

#include "statemachine.h"

namespace analysis {

	/**
	 * Abstract class for finding word boundaries in text
	 */
	class BreakIterator {
		
	public:
		
		virtual ~BreakIterator(); 
		
		/**
		 * Sets the text
		 */
		virtual void setText(const wchar_t* text) = 0; 
		
		/**
		 * Returns true, if next boundary is exist 
		 */
		virtual bool hasNext() = 0; 
		
		/**
		 * Returns the location of current break in string
		 */
		virtual int current() = 0; 

		/**
		 * Finds next break and returns the new location
		 */
		virtual int next() = 0;
		
	};
	
	/**
	 * State machine and longest matching algorithm based break 
	 * iterator. Used for finding word boundaries. State machine
	 * is typically compiled from dictionary.
	 * 
	 * @tparam Encoding Describes the serialization format of the state machine
	 */
	template<class Encoding>
	class StateMachineBreakIterator : public BreakIterator {

		public:
		
			/**
			 * Constructs the break iterator to use given state machine
			 */
			StateMachineBreakIterator(StateMachine<Encoding>& machine);
			
			~StateMachineBreakIterator(); 
			
		public: // From BreakIterator
			
			virtual void setText(const wchar_t* text);
			
			virtual bool hasNext(); 
			
			virtual int current(); 

			virtual int next();

		private: 
	
			/**
			 * Prepares next
			 */
			void prepareNext(); 
		
		private: 
		
			/** Used state machine. E.g. compiled from dictionary */
			StateMachine<Encoding>& machine_;
			
			/** Pointer to a state. Used for moving within state machine  */
			StateCursor<Encoding> state_; 
			
			/** Compiled text */ 
			const wchar_t* text_; 
			
			/** Cursor in text */
			int cursor_;
			
			/** Current break */
			int current_; 
			
			/** Next break */
			int next_; 
			
	};

}

#include "breakiterator.inl"

#endif /* BREAKITERATOR_H_ */
