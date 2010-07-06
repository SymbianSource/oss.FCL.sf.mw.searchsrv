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


#ifndef LOCALE_H_
#define LOCALE_H_

#include <string>
#include <vector>

namespace Cpix {

	namespace Spi {
	
		extern const wchar_t* SymbianLanguageCodePrefix;

		/**
		 * Returns a vector containing a list language names that is 
		 * ordered by priority. 
		 */
		std::vector<std::wstring> GetLanguageNames();
		
	}
}


#endif /* LOCALE_H_ */
