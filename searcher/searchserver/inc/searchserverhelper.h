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

#ifndef CSEARCHSERVERHELPER_H
#define CSEARCHSERVERHELPER_H

#include "cpixerror.h"

class SearchServerHelper
	{
public:

    /**
     * Helper method for cpix error checking.
     * In case of error this method clears error and leaves with given error code
     * @aResult cpix type containing error code
     * @aErrorCode Leaves with this code in case of cpix error
     */
    template <typename T>
    inline static void CheckCpixErrorL(T* aResult, TInt aErrorCode);

    /**
     * Log cpix error to rdebug/file.
     * @param aError Error to log.
     */
    inline static void LogErrorL(cpix_Error& aError);
	};

#include "SearchServerHelper.inl"

#endif // CSEARCHSERVERHELPER_H
