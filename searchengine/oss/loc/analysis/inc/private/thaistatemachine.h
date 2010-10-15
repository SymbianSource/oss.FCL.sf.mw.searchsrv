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

#ifndef THAISTATEMACHINE_H_
#define THAISTATEMACHINE_H_

#include "breakiterator.h"

namespace analysis {

	class ThaiSmEncoding 
	{
		public: 
			static const byte_t FINAL_BIT = 0x80; 
			static const byte_t LENGTH_BITS = 0x7f;
			
			static const byte_t DOT_CODE = 0;
			static const wchar_t THAI_CHART_OFFSET = 0x0E00;
		
			static inline bool getFinal(StatePointer pointer) 
			{
				return ((*pointer) & FINAL_BIT) > 0; 
			}
			static inline int readSuccCount(StatePointer& pointer)
			{
				return (*(pointer++)) & LENGTH_BITS;
			}
			static const int SIZEOF_CHAR = 1;
			static inline wchar_t readChar(byte_t*& pointer)
			{
				byte_t c = (*(pointer++));
				return c == DOT_CODE ? '.' : c + THAI_CHART_OFFSET;
			}
			static const int SIZEOF_OFFSET = 3;
			static inline StateOffset getOffset(byte_t* pointer)
			{
				// big endian
				byte_t high = *(pointer+0);
				byte_t mid = *(pointer+1);
				byte_t low = *(pointer+2);
				return static_cast<StateOffset>((high<<16) + (mid<<8) + low); 
			}
			
	};
	
	class ThaiAnalysisInfraNotInitialized : std::exception 
		{
		public:
			const char* what() const throw();
		};

	class StateMachineFileNotFound : std::exception 
		{
		public:
			const char* what() const throw();
		};
	
	class StateMachineLoadingFailed : std::exception
		{
		public:
			const char* what() const throw();
		};


	class ThaiAnalysisInfra 
	{
		public: // public static API 
		
			static void init(const char* dataFile); 
			
			static ThaiAnalysisInfra* theInstance();
			
			static void shutdown();
			
		public: // public non-static API 
			
			std::auto_ptr<BreakIterator> createBreakIterator();
			
		private: // construction
		
			ThaiAnalysisInfra(const char* dataFile);
			
			~ThaiAnalysisInfra(); 

		private: 
			
			static ThaiAnalysisInfra* theInstance_;
			
			Cpt::auto_array<byte_t> blob_;
			
			StateMachine<ThaiSmEncoding> stateMachine_;
			
			std::string dataFile_; 
	};
	
}
	
#endif 
