/*
 * analysisutil.cpp
 *
 *  Created on: Mar 25, 2010
 *      Author: admin
 */

#include "localetestinfos.h"
#include "config.h"

const char* EnglishLocale::LOCALE = "en";
const char* EnglishLocale::FILES[] = {
	STEM_TEST_CORPUS_PATH "\\en\\1.txt",
	STEM_TEST_CORPUS_PATH "\\en\\2.txt",
	STEM_TEST_CORPUS_PATH "\\en\\3.txt",
	STEM_TEST_CORPUS_PATH "\\en\\4.txt",
	0
};
const wchar_t* EnglishLocale::QUERIES[] = {
	L"happy",
	L"happiness",
	L"happening",
	0
};


const char* FrenchLocale::LOCALE = "fr";

const char* FrenchLocale::FILES[] = {
	LOC_TEST_CORPUS_PATH "\\fr\\1.txt",
	LOC_TEST_CORPUS_PATH "\\fr\\2.txt",
	LOC_TEST_CORPUS_PATH "\\fr\\3.txt",
	LOC_TEST_CORPUS_PATH "\\fr\\4.txt",
	0
};

const wchar_t* FrenchLocale::QUERIES[] = {
	L"d'\xe9nergie",
	L"\xe9nergie",
	L"elle",
	0
};

const char* HebrewLocale::LOCALE = "he";

const char* HebrewLocale::FILES[] = {
	LOC_TEST_CORPUS_PATH "\\he\\1.txt",
	LOC_TEST_CORPUS_PATH "\\he\\2.txt",
	LOC_TEST_CORPUS_PATH "\\he\\3.txt",
	LOC_TEST_CORPUS_PATH "\\he\\4.txt",
	0
};

const wchar_t* HebrewLocale::QUERIES[] = {
	L"\x05e9\x05e4\x05e2\x05ea", // L"שפעת",
	L"\x05e4\x05e2\x05ea", // L"פעת",
	L"\x05e9\x05e4\x05e2*", // L"שפע*",
	L"\x05e4\x05e8*", //L"פר*",
	0
};

const char* ThaiLocale::LOCALE = "th";
const char* ThaiLocale::FILES[] = {
	LOC_TEST_CORPUS_PATH "\\th\\1.txt",
	LOC_TEST_CORPUS_PATH "\\th\\2.txt",
	LOC_TEST_CORPUS_PATH "\\th\\3.txt",
	LOC_TEST_CORPUS_PATH "\\th\\4.txt",
	0
};
const wchar_t* ThaiLocale::QUERIES[] = {
	L"\x0E14\x0E32\x0E27\x0E15\x0E01", // a thai word
	L"\x0E21\x0E35", // another thai word
	0
};


const char* ChineseLocale::LOCALE = "ch";
const char* ChineseLocale::FILES[] = {
	LOC_TEST_CORPUS_PATH "\\ch_hk\\1.txt",
	LOC_TEST_CORPUS_PATH "\\ch_hk\\2.txt",
	LOC_TEST_CORPUS_PATH "\\ch_hk\\3.txt",
	LOC_TEST_CORPUS_PATH "\\ch_hk\\4.txt",
	
	LOC_TEST_CORPUS_PATH "\\ch_prc\\1.txt",
	LOC_TEST_CORPUS_PATH "\\ch_prc\\2.txt",
	LOC_TEST_CORPUS_PATH "\\ch_prc\\3.txt",
	LOC_TEST_CORPUS_PATH "\\ch_prc\\4.txt",

	LOC_TEST_CORPUS_PATH "\\ch_simple\\1.txt",
	LOC_TEST_CORPUS_PATH "\\ch_simple\\2.txt",
	LOC_TEST_CORPUS_PATH "\\ch_simple\\3.txt",
	LOC_TEST_CORPUS_PATH "\\ch_simple\\4.txt",

	LOC_TEST_CORPUS_PATH "\\ch_tw\\1.txt",
	LOC_TEST_CORPUS_PATH "\\ch_tw\\2.txt",
	LOC_TEST_CORPUS_PATH "\\ch_tw\\3.txt",
	LOC_TEST_CORPUS_PATH "\\ch_tw\\4.txt",

	0
};
const wchar_t* ChineseLocale::QUERIES[] = {
	L"\x53f0\x6e7e", // a chinese word
	L"\x4e2d\x56fd", // another chinese word
	0
};

const char* KoreanLocale::LOCALE = "ko";
const char* KoreanLocale::FILES[] = {
	LOC_TEST_CORPUS_PATH "\\ko\\1.txt",
	LOC_TEST_CORPUS_PATH "\\ko\\2.txt",
	LOC_TEST_CORPUS_PATH "\\ko\\3.txt",
	LOC_TEST_CORPUS_PATH "\\ko\\4.txt",
	
	0
};
const wchar_t* KoreanLocale::QUERIES[] = {
	L"\xc2a4\xd1a0\xb9ac", // a korean word
	L"\xc778\xbb3c", // another korean word
	0
};

const char* JapaneseLocale::LOCALE = "jp";
const char* JapaneseLocale::FILES[] = {
	LOC_TEST_CORPUS_PATH "\\jp\\1.txt",
	LOC_TEST_CORPUS_PATH "\\jp\\2.txt",
	LOC_TEST_CORPUS_PATH "\\jp\\3.txt",
	LOC_TEST_CORPUS_PATH "\\jp\\4.txt",

	LOC_TEST_CORPUS_PATH "\\jp_old\\1.txt",
	LOC_TEST_CORPUS_PATH "\\jp_old\\2.txt",
	LOC_TEST_CORPUS_PATH "\\jp_old\\3.txt",
	LOC_TEST_CORPUS_PATH "\\jp_old\\4.txt",

	0
};
const wchar_t* JapaneseLocale::QUERIES[] = {
	L"\x56fd\x969b", // a japanese word
	L"\x65e5\x672c", // another japanese word
	0
};
