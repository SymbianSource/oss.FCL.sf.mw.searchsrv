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
#include "tinyunicode.h"

namespace analysis {

	namespace unicode {

        // See Chapter 16 Hangul in http://unicode.org/reports/tr15/ for reference
        // 
        // Ported from Hangul Java code example
        //
	
        int IsHangulSyllable(int c) {
            return (c >= 0xAC00 && c <= 0xD7AF);
        }
        int IsHangulJamo(int c) {
            return (c >= 0x1100 && c < 0x1200)     // Hangul Jamo
                || (c >= 0x3130 && c <= 0x318F)    // Hangul compatibility Jamo
                || (c >= 0xA960 && c < 0xA97F)     // Hangul Jamo Extended-A
                || (c >= 0xD780 && c <= 0xD7FF)    // Hangul Jamo Extended-B
                || (c >= 0xff00 && c <= 0xffef);   // Hangul halfwidth and fullwidth forms
        }

        int IsHangul(int c) {
            return IsHangulSyllable(c) || IsHangulJamo(c); 
        }
		int IsCjk(int c) {
			return (c >= 0x4E00 && c < 0xa000)  // CJK Unified ideographs block
                 || IsHangul(c)              // Korean alphabet
				 || (c >= 0x3400 && c < 0x4Dc0)     // CJK Unified ideographs extension A
				 || (c >= 0x3040 && c <= 0x309f)    // Hiragana
				 || (c >= 0x20000 && c < 0x30000);  // CJK Unified ideographs extension B, C, D E and so forth
		}
		
		int IsThai(int c) {
			return (c >= 0x0E00) && (c < 0x0F00); // Thai unicode block 
		}
	}
	
}
