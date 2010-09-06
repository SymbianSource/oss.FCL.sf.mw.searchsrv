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

#ifndef __CPIXMAINDEFS_H__
#define __CPIXMAINDEFS_H__


//
//    TODO LIST
//
//    o paths, names, naming styles should be streamlined
//


// ***** MAIN ***** 


/**
 * These two paths are for test tools only, not to be used by
 * harvesters and suchlike.
 */
#define CPIX_TEST_INDEVICE_INDEXDB_PHMEM   "c:\\data\\indexing\\indexdb"
#define CPIX_TEST_INDEVICE_INDEXDB_MMC     "e:\\data\\indexing\\indexdb"

#define LCPIX_INDEVICE_INDEXDB        L"indexing\\indexdb"

#define CPIX_INDEVICE_SERVERIO 	      "c:\\data\\indexing\\serverio"
#define LCPIX_INDEVICE_SERVERIO 	  L"c:\\data\\indexing\\serverio"

#define LROOT_APPCLASS  L"root"

//***** CONTACT *****

#define LCONTACT_QBASEAPPCLASS  L"@0:root contact"
#define LCONTACTAPPCLASS  L"root contact"

//***** CALENDAR *****

#define LCALENDAR_QBASEAPPCLASS  L"@0:root calendar"
#define LCALENDARAPPCLASS  L"root calendar"

//***** BOOKMARKS *****
#define BOOKMARK_QBASEAPPCLASS   "@0:root bookmark"
#define LBOOKMARK_QBASEAPPCLASS  L"@0:root bookmark"
#define BOOKMARKAPPCLASS   "root bookmark"
#define LBOOKMARKAPPCLASS  L"root bookmark"

//******* APPLICATIONS ****
#define APPLICATIONS_QBASEAPPCLASS "@0:root applications"
#define LAPPLICATIONS_QBASEAPPCLASS L"@0:root applications"
#define APPLICATIONS_APPCLASS "root applications"
#define LAPPLICATIONS_APPCLASS L"root applications"

//***** MAPS *****

#define LMAPS_BASEAPPCLASS  L"root maps"

// Distance in KM from a reference point
#define LDISTANCE_FIELD     L"Distance"

//***** NOTES *****

#define NOTES_QBASEAPPCLASS   "@0:root notes"
#define LNOTES_QBASEAPPCLASS  L"@0:root notes"
#define NOTESAPPCLASS   "root notes"
#define LNOTESAPPCLASS  L"root notes"
   //
   // NOTE: Following fields are duplicated in SearchClient's Common.h
   //

   /**
    * The name of the mandatory field that will store the unique ID of
    * a document.
    */
#define LCPIX_DOCUID_FIELD    L"_docuid"

   /**
    * The name of the mandatory field that will store the application
    * class of a document.
    */
#define LCPIX_APPCLASS_FIELD  L"_appclass"
#define LCPIX_UNTOKENIZED_APPCLASS_FIELD L"_appclass_ut"
   /**
    * The name of the optional field that will store the excerpt text
    * for a document.
    */
#define LCPIX_EXCERPT_FIELD   L"_excerpt"

/*
 * The name of the optional field that will store the first line
 * text shown in result of searchUI, used for highlighting
 */

#define LCPIX_HL_EXCERPT_FIELD  L"_hlexcerpt"

   /**
    * The name of the optional field that will store the application
    * ID of the document.
    */
#define LCPIX_MIMETYPE_FIELD     L"_mimetype"

   /**
    * The default field to search for (aggregate of the contents of
    * other fields).
    */
#define LCPIX_DEFAULT_FIELD   L"_aggregate"

   /**
    * The default field to search for prefixes (aggregate of the contents of
    * prefixes of other fields).
    */
#define LCPIX_DEFAULT_PREFIX_FIELD   L"_aggregate_prefix"


   /**
    * Documents, as created by the client may undergo a further
    * the document. Such pre-index processing stage is invoked by
    * processing stage that may remove from, or add multiple fields to
    * putting an extra field with the value for the filter ID
    * identifying the filter to do the processing. (For instance, file
    * parser filter.) This special field is only to kick-start the
    * pre-index processing stage and identify the filter that does the
    * processing, but the field itself will be removed before actual
    * indexing!
    *
    * When defining a filter id, make sure to define the following flags:
    *
    *     cpix_STORE_YES | cpix_INDEX_NO,
    *
    * as clucene is known for misbehaviour if fields are removed and
    * re-added with different flags.
    */
#define LCPIX_FILTERID_FIELD  L"_FilterId"

/**
 * Filter ID for the quadrant processing filter. This filter:
 *
 * (a) Assumes to find GPS coordinates in two fields: LLATITUDE_FIELD
 *     and LLONGITUDE_FIELD. The coordinates should be in a
 *     floating-point, decimal notation, negative values for southern
 *     latitudes and western longitudes. These fields will be left
 *     on the document, unremoved.
 *
 * (b) From the GPS coordinate, it will generate a QNR_FIELD.
 *
 * (c) From that point on, a document with that QNR_FIELD defined is a
 *     subject for quad qry types (geological search).
 *
 */
#define LCPIX_QUADFILTER_FID L"QuadFilter"


   /**
    * Filter ID for the fileparser filter. This filter
    *
    * (a) uses the doc uid field as a filesystem path to get the
    * contents of the file
    *
    * (b) creates one or multiple fields, based on the content of the
    * file.
    *
    * (c) Also creates the excerpt field
    */
#define LCPIX_FILEPARSER_FID   L"FileParser"



   

   /**
    * Term textual content. Used in documents forming term list that
    * is returned by suggest method.
    */
#define LTERM_TEXT_FIELD       L"Term"

   /**
    * Term's document frequence. Used in documents forming term list
    * that is returned by suggest method.
    */
#define LTERM_DOCFREQ_FIELD   L"DocFreq"
 
   

   /**
    * This is the directory CPix will use to store its internal data,
    * like cpixreg.txt as well as indexes with automatic paths
    * (automatic = harvester / owner app does not care where it
    * is).
 */
#define DEFAULT_CPIX_DIR "c:\\Data\\"
#define DEFAULT_CLUCENE_LOCK_DIR "c:\\system\\temp"
#define DEFAULT_RESOURCE_DIR "z:\\resource\\cpix"




#define LMESSAGING_MIMETYPE 		L"application/messaging"
#define LCONTACT_MIMETYPE			L"application/contact" //or "text/x-vcard", is not proved to be correct
#define LCALENDAR_MIMETYPE 			L"application/v-calendar" //or "text/x-vcalendar", is not proved to be correct
#define LTEXTFILE_MIMETYPE 			L"text/plain"
#define LPDFFILE_MIMETYPE          L"pdf/plain"
#define LJPGFILE_MIMETYPE 			L"image/jpg"
#define LMP3FILE_MIMETYPE 			L"audio/mp3"
#define BOOKMARK_MIMETYPE            "application/bookmark"
#define LBOOKMARK_MIMETYPE           L"application/bookmark"
#define APPLICATION_MIMETYPE    "application/application"
#define LAPPLICATION_MIMETYPE    L"application/application"
#define NOTES_MIMETYPE               "application/notes"
#define LNOTES_MIMETYPE               L"application/notes"
#define FILE_MIMETYPE                 "type/file"
#define LFILE_MIMETYPE                L"type/file"
#define FOLDER_MIMETYPE               "type/folder"
#define LFOLDER_MIMETYPE              L"type/folder"

/* 
 * Analyzer definition related constants  
 *********************************************** 
 */

#define CPIX_PIPE 	  				L">"
#define CPIX_SWITCH 	  			L"switch"
#define CPIX_LOCALE_SWITCH  		L"locale_switch"
#define CPIX_CONFIG_SWITCH  		L"config_switch"
#define CPIX_CASE 	 				L"case"
#define CPIX_DEFAULT  				L"default"

// These can be given as parameters for the 'natural' analyzer
#define CPIX_ID_INDEXING 			L"indexing"  
#define CPIX_ID_QUERY 				L"query"  
#define CPIX_ID_PREFIX 				L"prefix"  

#define CPIX_ANALYZER_DEFAULT   	L"natural"  

// Default indexing analyzer
//#define CPIX_ANALYZER_DEFAULT_QUERY L"natural(indexing)"

// Default term query analyzer
#define CPIX_ANALYZER_DEFAULT_QUERY L"natural(query)"

// Default prefix analyzer
#define CPIX_ANALYZER_DEFAULT_PREFIX L"natural(prefix)"  

#define CPIX_ANALYZER_STANDARD  	L"standard"
#define CPIX_ANALYZER_PHONENUMBER   L"PhoneNumberAnalyzer"
//#define CPIX_ANALYZER_DEFAULT   	L"standard"  

#define CPIX_TOKENIZER_STANDARD    	L"stdtokens" 
#define CPIX_TOKENIZER_WHITESPACE  	L"whitespace"
#define CPIX_TOKENIZER_LETTER  		L"letter"
#define CPIX_TOKENIZER_KEYWORD  	L"keyword"
#define CPIX_TOKENIZER_CJK  		L"cjk"
#define CPIX_TOKENIZER_NGRAM        L"ngram"
#define CPIX_TOKENIZER_KOREAN       L"korean"
#define CPIX_TOKENIZER_KOREAN_QUERY L"koreanquery"

#define CPIX_FILTER_STANDARD  	  	L"stdfilter"
#define CPIX_FILTER_LOWERCASE  		L"lowercase"
#define CPIX_FILTER_ACCENT  		L"accent"
#define CPIX_FILTER_STOP  		  	L"stop" 
#define CPIX_FILTER_STEM 		  	L"stem"
#define CPIX_FILTER_LENGTH			L"length"
#define CPIX_FILTER_PREFIXES		L"prefixes"
#define CPIX_FILTER_THAI			L"thai"
#define CPIX_FILTER_PREFIX			L"prefix"
#define CPIX_FILTER_ELISION			L"elision"

#define CPIX_WLANG_EN               L"en" // english
#define CPIX_WLANG_FI               L"fi" // finnish
#define CPIX_WLANG_HU               L"hu" // hungarian
#define CPIX_WLANG_RU               L"ru" // russian
#define CPIX_WLANG_AR               L"ar" // arabic
#define CPIX_WLANG_CN               L"cn" // chinese
#define CPIX_WLANG_CS               L"cs" // czech
#define CPIX_WLANG_DE               L"de" // deutch - german
#define CPIX_WLANG_EL               L"el" // greek
#define CPIX_WLANG_FA               L"fa" // farsi  - persian
#define CPIX_WLANG_FR               L"fr" // french
#define CPIX_WLANG_HE               L"he" // hebrew
#define CPIX_WLANG_NL               L"nl" // dutch
#define CPIX_WLANG_BN               L"bn" // bangla - bengali

#define MAX_EXCERPT_LENGTH          125   //maximum exceprt length 

#endif /*__CPIXMAINDEFS_H__*/
