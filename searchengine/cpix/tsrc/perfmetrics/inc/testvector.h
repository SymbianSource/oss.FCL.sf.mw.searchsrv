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

#ifndef PERFMETRICS_TESTVECTOR_H_
#define PERFMETRICS_TESTVECTOR_H_

#include <iosfwd>
#include <list>
#include <string>
#include <strstream>
#include <vector>

struct TestVector {
    std::string            testName;
    int                    create;
    int                    bufferSize;
    int                    numPreindexedItems;
    int                    maxInputSize;
    int                    lumpSize;
    int                    first;
    int                    last;
    std::list<std::string> queryTerms;
    int                    search;
    int                    incrementalSearch; 
    int                    minIncrementalLength;
    int                    suggestionSearch;
    int                    minSuggestionLength;
    int                    takeIdxSnapshot;
    int                    addLasts;
    int                    delReAddLasts;
    int                    updateLasts;

    TestVector();

private:
    friend std::istream & operator>>(std::istream & is,
                                     TestVector   & tv);

    std::istream & read(std::istream & is);

    int readInt(std::istream   & is);
    std::string readStr(std::istream   & is);
    void nextLine(std::istream   & is,
                  std::string    & line);
};

std::istream & operator>>(std::istream & is,
                          TestVector   & tv);



#endif
