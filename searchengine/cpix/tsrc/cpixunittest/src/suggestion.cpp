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

#include <algorithm>
#include <vector>

#include "cpixsearch.h"

#include "suggestion.h"
#include "config.h"
#include "testutils.h"



Suggestion::Suggestion(cpix_Document * doc,
                       Itk::TestMgr  * testMgr)
{
    const wchar_t
        * value = cpix_Document_getFieldValue(doc,
                                              LTERM_TEXT_FIELD);
    if (cpix_Failed(doc))
        {
            ITK_EXPECT(testMgr,
                       false,
                       "Could not get suggestion term");
            term_ = L"(error)";
        }
    else
        {
            term_ = value;
        }
    
    value = cpix_Document_getFieldValue(doc,
                                        LTERM_DOCFREQ_FIELD);
    if (cpix_Failed(doc))
        {
            ITK_EXPECT(testMgr,
                       false,
                       "Could not get suggestion doc frequency");
            frequency_ = L"0";
        }
    else
        {
            frequency_ = value;
        }
}


const std::wstring & Suggestion::term() const
{
    return term_;
}


const std::wstring & Suggestion::frequency() const
{
    return frequency_;
}


bool operator<(const Suggestion & left,
               const Suggestion & right)
{
    //
    // We want to have the suggestions ((term, frequency) pairs)
    // ordered by:
    //
    //  o first by frequency, in descending order
    //  o then by term (lexicographical ascending)
    //

    bool
        rv = false;

    const std::wstring
        lFreq(left.frequency()),
        rFreq(right.frequency());

    if (lFreq.length() > rFreq.length())
        {
            rv = true;
        }
    else if (lFreq.length() == rFreq.length())
        {
            if (lFreq > rFreq)
                {
                    // same length but lexicographically less =
                    // numerically less
                    rv = true;
                }
            else if (lFreq == rFreq)
                {
                    // same frequencies, now we order according to
                    // terms (alphabetic lexical ordering)
                    rv = left.term() < right.term();
                }
        }

    return rv;
}


void Suggestion::printSuggestions(cpix_Hits    * hits,
                                  Itk::TestMgr * testMgr)
{
    bool
        goOn = true;
    std::vector<Suggestion>
        suggestions;
    int32_t
        hitCount = cpix_Hits_length(hits);

    suggestions.reserve(hitCount);

    for (int32_t i = 0; i < hitCount; ++i)
        {
            cpix_Document
                **doc;
            ALLOC_DOC(doc, 1);
            cpix_Hits_doc(hits,
                          i,
                          doc,
                          1);
            if (doc[0]->ptr_ != NULL) {
            if (cpix_Failed(hits))
                {
                    ITK_EXPECT(testMgr,
                            false,
                            "Failed to get doc %d",
                            i);
                    cpix_ClearError(hits);
                    goOn = false;
                    break;
                }

            suggestions.push_back(Suggestion(doc[0],
                    testMgr));
            }
            FREE_DOC(doc, 1);
        }

    if (goOn)
        {
            std::sort(suggestions.begin(),
                      suggestions.end());
                
            std::vector<Suggestion>::const_iterator
                i = suggestions.begin(),
                end = suggestions.end();

            for (; i != end; ++i)
                {
/*					wprintf(i->term().c_str()); 
					printf(" "); 
					wprintf(i->frequency().c_str()); 
					printf("\n"); */
                    fprintf(stdout,
                            "%S %S\n",
                            i->term().c_str(),
                            i->frequency().c_str());
                }
        }
}

