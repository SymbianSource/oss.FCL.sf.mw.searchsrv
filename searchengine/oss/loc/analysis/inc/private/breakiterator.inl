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
namespace analysis {
	
	template<class Encoding>
	StateMachineBreakIterator<Encoding>::StateMachineBreakIterator(
		StateMachine<Encoding>& machine)
	:	machine_( machine ),
		state_(), 
		current_(-1),
		next_(-1), 
		text_(0)
	{
	}
	
	template<class Encoding>
	StateMachineBreakIterator<Encoding>::~StateMachineBreakIterator() {}
	
	template<class Encoding>
	void StateMachineBreakIterator<Encoding>::setText(const wchar_t* text) 
	{
		// Let's point to the begining of new text
		text_ = text;
		cursor_ = 0; 
		
		// First boundary is in the beginning of buffer 
		current_ = 0; 
		// We haven't searched for next boundary yet
		next_ = -1; 
	}
	
	template<class Encoding>
	bool StateMachineBreakIterator<Encoding>::hasNext()
	{
		prepareNext(); 
		return next_ != -1; 
	}
	
	template<class Encoding>
	int StateMachineBreakIterator<Encoding>::current()
	{
		return current_; 
	}
	
	template<class Encoding>
	int StateMachineBreakIterator<Encoding>::next()
	{
		prepareNext();
		current_ = next_; 
		next_ = -1; 
		return current_;
	}
	
	template<class Encoding>
	void StateMachineBreakIterator<Encoding>::prepareNext()
	{
		// Implements longest matching word algorithm. The used 
		// state machine contains an entire dictionary. Each state
		// transition interprets as incremental search in dictionary. 
		// Each final (or terminal) state, marks location, where the 
		// consumed states form a valid word. We try to find the 
		// longest matching word. 
		// 
		
		// Prepare next_ only, if new next_ hasn't been prepared before 
		if (next_ == -1 && text_ && text_[cursor_]) {
			// Reset state machine
			machine_.rootState(state_);

			// lastBreak points to the end of last recognized word
			int lastBreak = -1;  
			// Continue until EOF
			while (text_[cursor_]) {
				// Feed next character to the state machine 
				// and try to transit the state
				if (!state_.next(text_[cursor_++])) {
					// Check last final state
					if (lastBreak != -1) { 
						// Final state marked a valid word
						// This is word boundary we were lookign
						cursor_ = lastBreak; 
					}
					break;
				} else if (state_.isFinal()) {
					// Found a valid word! Mark the location 
					lastBreak = cursor_; 
					// Still, continue and try to find even a longer word
				}
			}
			next_ = cursor_;
		}
	}

}
