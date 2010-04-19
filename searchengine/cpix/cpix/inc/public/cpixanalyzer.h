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

#ifndef CPIX_CPIXANALYZER_H
#define CPIX_CPIXANALYZER_H


#include "cpixinit.h"
#include "cpixerror.h"


/**
 * CPix API for analyzers.
 *
 * See cpixsearch.h for notes and info.
 */


#ifdef __cplusplus
extern "C" {
#endif


    /**
     * See ISO 639-1
     * http://www.loc.gov/standards/iso639-2/php/English_list.php
     */
    struct cpix_LangCode_
    {
        const char * code_;
    };
    typedef cpix_LangCode_ cpix_LangCode;

    extern const cpix_LangCode cpix_LANG_EN;
    extern const cpix_LangCode cpix_LANG_FI;
    extern const cpix_LangCode cpix_LANG_HU;
    extern const cpix_LangCode cpix_LANG_RU;

    
    /**
     * Gets the language descriptors for supported languages.
     *
     * @return the NULL-terminated array of (pointers-to) supported
     * language descriptors. DO NOT MODIFY THESE.
     */
    const cpix_LangCode ** cpix_supportedLanguages();

    /************************************************************************
     * Interface cpix_Analyzer
     */
    struct cpix_Analyzer_
    {
        // pointer to native (CLucene) impl
        void       * ptr_;
        
        // Last error, if any, that resulted from executing the
        // last operation on this cpix_XXX object
        // Use macros cpix_Failed, cpix_Succeeded and cpix_ClearError.
        // Do not attempt releasing it.
        cpix_Error * err_;
    };
    typedef cpix_Analyzer_ cpix_Analyzer;
    

    /**
     * Factory method to create a simple analyzer.
     *
     * @return a new analyzer object, ownership is transferred to the
     * caller. NULL if creation failed - no more info is given why.
     */
    cpix_Analyzer * cpix_CreateSimpleAnalyzer(cpix_Result * result);
    
    /**
     * Factory method to create an analyzer that uses the snowball
     * library (libstemmer) to perform stemming.
     *
     * @param result any details on failure are communicated through
     * this
     *
     * @param langCode the language code specifying the stemmer to use
     *
     * @return the new analyzer object, ownership is transferred to
     * the caller. NULL if creation failed - no more info is given
     * why.
     */
    cpix_Analyzer * cpix_CreateSnowballAnalyzer(cpix_Result   * result,
                                                cpix_LangCode   langCode);
    
    /**
     * Factory method for creating a complex analyzer described in the
     * definition string. 
     * 
     * The basic syntax consist of a sequence starting with a tokenizer
     * definitions and followed by the sequence of filter definitions.
     * 
     * tokenizer>filter>filter>filter
     *  
     * In syntax, tokenizers and filters are separated by pipe symbol '>', 
     * which symbolizes the piping of the token stream output of previous item
     * to the token stream input of the following item.
     * 
     * Filters' accept constructors, like the stemmer, stop filter and length 
     * filter: 
     * 
     * stdtokens>length(2, 8)>stop(en)>stop('foo', 'bar')>stem(en)
     * 
     * There is also special syntax for switching filter depending of the 
     * analyzed field. 
     * 
     * switch {
     *   case 'field1':            tokenizer1>filter1; 
     *   case 'field2', 'field3':  tokenizer2>filter2;
     *   default:                  analyzer;
     * }
     * 
     * See documentation for exact list of analyzers, tokenizers and filters
     * that are supported and of their parameters. 
     */
    cpix_Analyzer * cpix_Analyzer_create(cpix_Result   * result,
                                         const wchar_t * definition);


    /**
     * Destroys this analyzer instance. Never fails.
     */
    void cpix_Analyzer_destroy(cpix_Analyzer * thisAnalyzer);
 
#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */



#endif /* CPIX_CPIXANALYZER_H */
