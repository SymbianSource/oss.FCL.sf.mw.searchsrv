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

#ifndef STATEMACHINE_INL_
#define STATEMACHINE_INL_

namespace analysis {

	template<class Encoding>
	StateCursor<Encoding>::StateCursor()
	:  blob_(0), 
	   pointer_(0)
	{}
	
	template<class Encoding>
	void StateCursor<Encoding>::reset(byte_t* blob, StateOffset offset)
	{
		blob_ = blob; 
		pointer_ = &blob[offset]; 
	}

	template<class Encoding>
	bool StateCursor<Encoding>::isFinal() 
	{
		return Encoding::getFinal(pointer_); 
	}
		
	template<class Encoding>
	bool StateCursor<Encoding>::next(wchar_t c) 
	{
		byte_t* p = pointer_; 

		// How many transitions we have
		int n = Encoding::readSuccCount(p);

		// Search transition that matches 'c'
		for (int i = 0; i < n; i++) 
		{ 	// linear speed: O(n/2)
			wchar_t c2 = Encoding::readChar(p);
			if ( c2 == c ) {
				// Found transition, state is changed.
				// Read pointer to the next state
				int offset = Encoding::getOffset(p);
				pointer_ = &blob_[offset];
				return true; 
			} else if ( c2 > c ) break;
			// Skip pointer to next state
			p += Encoding::SIZEOF_OFFSET;
		}
		// Maching transition was not found
		return false; 
	}

	template<class Encoding>
	const StateOffset StateMachine<Encoding>::ROOT_STATE_OFFSET = 0;
	
	template<class Encoding>
	StateMachine<Encoding>::StateMachine()
	:	blob_(0)
	{}
	
	template<class Encoding>
	void StateMachine<Encoding>::reset(byte_t* blob) 
	{
		blob_ = blob; 
	}
			
	template<class Encoding>
	void StateMachine<Encoding>::rootState(StateCursor<Encoding>& cursor)
	{
		cursor.reset(blob_, ROOT_STATE_OFFSET); 
	}

}


#endif /* STATEMACHINE_INL_ */
