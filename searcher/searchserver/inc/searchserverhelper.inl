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

#include "SearchServerHelper.h"

template <typename T>
void SearchServerHelper::CheckCpixErrorL(T* aResult, TInt aErrorCode)
    {
    if ( aResult && cpix_Failed( aResult ) )
        {
        SearchServerHelper::LogErrorL( *(aResult->err_) );
        cpix_ClearError(aResult);
        User::Leave(aErrorCode);
        }
    }

void SearchServerHelper::LogErrorL(cpix_Error& aError)
    {
#ifdef CPIX_LOGGING_ENABLED
    const TInt KErrorReportMaxLength = 512;
    TBuf<KErrorReportMaxLength> error_report;
    cpix_Error_report(&aError,
                      (wchar_t*)error_report.Ptr(),
                      KErrorReportMaxLength);
    error_report.SetLength(wcslen((wchar_t*)error_report.Ptr()));
    CPIXLOGSTRING2("CPix error message: %S", &error_report);
#endif // CPIX_LOGGING_ENABLED
    }
