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
#include <iostream>


#include "testvector.h"

TestVector::TestVector()
    : testName("plain"),
      create(1),
      bufferSize(32*1024), // TODO: bind to constant  
      numPreindexedItems(0),
      maxInputSize(10),
      lumpSize(5),
      first(1),
      last(1),
      search(0),
      incrementalSearch(0),
      minIncrementalLength(3),
      suggestionSearch(0),
      minSuggestionLength(3),
      takeIdxSnapshot(0),
      addLasts(1),
      delReAddLasts(1),
      updateLasts(1)
{
    ;
}


int (TestVector::* IntMembers[]) = {
    &TestVector::create,
    &TestVector::bufferSize, 
    &TestVector::numPreindexedItems,
    &TestVector::maxInputSize,
    &TestVector::lumpSize,
    &TestVector::first,
    &TestVector::last,
    &TestVector::search,
    &TestVector::incrementalSearch,
    &TestVector::minIncrementalLength,
    &TestVector::suggestionSearch,
    &TestVector::minSuggestionLength,
    &TestVector::takeIdxSnapshot,
    &TestVector::addLasts,
    &TestVector::delReAddLasts,
    &TestVector::updateLasts
};


std::istream & TestVector::read(std::istream & is)
{
    if (is)
        {
            testName = readStr(is);
        }
            
    for (size_t i = 0; 
         i < sizeof(IntMembers) / sizeof(int (TestVector::*)) && is;
         ++i)
        {
            this->*IntMembers[i] = readInt(is);
        }

    while (is)
        {
            std::string 
                queryTerm = readStr(is);

            if (is)
                {
                    queryTerms.push_back(queryTerm);
                }
        }

    return is;
}


int TestVector::readInt(std::istream   & is)
{
    std::string
        line;

    nextLine(is,
             line);

    int
        rv = 0;

    if (is)
        {
            std::strstream
                dummy(const_cast<char*>(line.c_str()),
                      line.length());
            dummy >> rv;
        }

    return rv;
}


std::string TestVector::readStr(std::istream   & is)
{
    std::string
        line;

    nextLine(is,
             line);
    
    if (line.length() > 0)
        {
            if (line[line.length()-1] == '\r')
                {
                    line = line.substr(0,
                                       line.length() - 1);
                }
        }

    return line;
}


void TestVector::nextLine(std::istream   & is,
                          std::string    & line)
{
    while (getline(is, line))
        {
            if (line.length() > 0
                && line[0] != '#'
                && line[0] != '\r')
                {
                    break;
                }
            else
                {
                    line = "";
                }
        }
}



std::istream & operator>>(std::istream & is,
                          TestVector   & tv)
{
    return tv.read(is);
}
