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


#ifndef LOCALIZATION_H_
#define LOCALIZATION_H_

#include <string>
#include <vector>
#include "cpixsynctools.h"

namespace Cpix {

	/**
	 * Class that is used for storing and retrieving used locale
	 */
	class Localization {
		
		public:
		
			/**
			 * Returns a list of language names.
			 * 
			 * MT safe
			 */
			std::vector<std::wstring> getLanguageNames();

			/**
			 * Sets the used locale, if locale is set to be "auto", 
			 * underlying mechanism will consult environment for
			 * maintaining correct locale. 
			 * 
			 * MT safe
			 */
			void setLocale(const char* locale);

			/**
			 * Sets the used locale, if locale is set to be "auto", 
			 * underlying mechanism will consult environment for
			 * maintaining correct locale.
			 * 
			 * MT safe
			 */
			void setLocale(const wchar_t* locale);
			
		public: // static API 
			
			/**
			 * Accessor for the localization singleton instance
			 * 
			 * NOTE: Should be called during init. Otherwise, if two threads try
			 * to access localization instance at the same time, memory
			 * leak may result. In this case two singleton instances may 
			 * be constructed. 
			 */
			static Localization& instance();
			
			/**
			 * Shutsdown
			 */
			static void shutdown(); 
			
		private: 

			Localization();

			static Localization* theInstance_; 
			
		private:
			
			Cpt::Mutex mutex_;
			
			bool auto_; 

			std::vector<std::wstring> languageNames_;
			
	};
	
}
#endif /* LOCALIZATION_H_ */
