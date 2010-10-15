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

#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

#include <stdint.h>

namespace analysis {

	typedef uint8_t byte_t; 

	/**
	 * Points to the beginning of the state in the memory
	 */
	typedef byte_t* StatePointer;

	/**
	 * Points to the begining of the state as calculated
	 * from the beginning of the state machine blob. Can be
	 * turned into a StatePointer by combining it with the 
	 * state machine blob pointer. 
	 */
	typedef unsigned int StateOffset;

	/**
	 * Can be used for navigating throught the state machine
	 * staets. In a sense 'a pointer' to some state. Can be used 
	 * to read information of the state or to jump to another location
	 * Must be reset before usage.
	 * 
	 * @tparam Encoding describes encoding of the state machine binary
	 */
	template <class Encoding>
	class StateCursor {
		
	public:
		/** Constructs the cursor. */
		StateCursor();
		
		/** 
		 * Resets the state cursor to point to a new location. After
		 * first reset other methods can be used.
		 * @param blob State machine binary. Ownership NOT transferred 
		 * @param offset Points to a state within the state machine binary 
		 */
		void reset(byte_t* blob, StateOffset offset);
		
		/** 
		 * Describes, if this state is a terminal state.
		 * @note Must not be used before reset() is once called  
		 */
		bool isFinal();
		
		/** 
		 * Moves to next state based on the input. Returns false, 
		 * if no transition for input was found.
		 * @note Must not be used before reset() is once called  
		 * @param c An input character. 
		 * @returns True, if state did change, false otherwise  
		 */
		bool next(wchar_t c);
		
	private: 
		/** State machine binary. Does NOT hold ownership */
		byte_t* blob_; 
		
		/** Raw memory pointer to the state. State is within state machine binary */
		StatePointer pointer_; 
	};

	/**
	 * Represents the state machine. Mainly wraps the state machine binary.
	 * Can be used to reset StateCursors
	 *   
	 * @tparam Encoding describes the encoding of the state machine binary
	 */
	template<class Encoding>
	class StateMachine {
		public: 
		
			static const StateOffset ROOT_STATE_OFFSET;
			
		public:
		
			/** 
			 * Constructs state. State machine is not functional, before
			 * reset is called
			 */
			StateMachine();
			
			/**
			 * Loads the state machine with the state machine binary.
			 * @param blob The state machine binary. Ownership is NOT transferred.
			 */
			void reset(byte_t* blob); 
			
			/**
			 * Resets the cursor to point out to this state machine's
			 * root state.
			 * @param cursor this cursor is set to point to the state machine's root state. 
			 * @note Must not be called before StateMachine is properly reset
			 */
			void rootState(StateCursor<Encoding>& cursor);
			
		private: 
			
			/** State machine binary. Does NOT hold ownership */
			byte_t* blob_;
	};
}

#include "statemachine.inl"

#endif /* STATEMACHINE_H_ */
