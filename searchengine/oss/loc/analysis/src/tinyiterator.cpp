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

#include "tinyiterator.h"

namespace analysis {
	namespace tiny {
		namespace cl {
		
			const char* TooOldIndexException::what() const {
				return "Index is referring to a location, which is no more present in the buffer."; 
			}

		}
	}
}
 
