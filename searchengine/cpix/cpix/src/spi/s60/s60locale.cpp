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

#include <e32std.h>
#include <sstream>

#include "spi/locale.h"

namespace Cpix {

	namespace Spi {
	
		struct LangCodeTranslationEntry {
			int symbianCode_;
			const wchar_t* isoCode_; 
		};
		
		LangCodeTranslationEntry LangCodeTranslations[] = {
			{ELangEnglish, L"en"},
			{ELangCanadianEnglish, L"en"},
			{ELangInternationalEnglish, L"en"},
			{ELangSouthAfricanEnglish, L"en"},
			
			{ELangFrench, L"fr"}, 
			{ELangSwissFrench, L"fr"},
			{ELangBelgianFrench, L"fr"},
			{ELangInternationalFrench, L"fr"},
			{ELangCanadianFrench, L"fr"},
			
			{ELangHebrew, L"he"},
			
			{ELangTaiwanChinese, L"ch"},
			{ELangHongKongChinese, L"ch"},
			{ELangPrcChinese, L"ch"},
			{ELangThai, L"th"},
			{ELangJapanese, L"jp"},
			{ELangKorean, L"ko"},
			
			{ELangNone, 0}
		};
		
		std::vector<std::wstring> GetLanguageNames() {
			TLanguage lang = User::Language();
			
			std::vector<std::wstring> ret; 
			std::wostringstream code; 
			code<<SymbianLanguageCodePrefix<<lang;
			ret.push_back(code.str()); 
			
			for (int i = 0; LangCodeTranslations[i].symbianCode_ != ELangNone; i++) {
				if ( LangCodeTranslations[i].symbianCode_ == lang ) {
					ret.push_back( LangCodeTranslations[i].isoCode_ ); 
				}
			}
			
			return ret;
		}
		
	}
}
