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

#include "testcorpus.h"

#include "cpixfstools.h"
#include <sstream>

using namespace std;

LineTestCorpus::LineTestCorpus(const char* path)
: items_() 
{
	FILE* ifs; 
	ifs = fopen(path, "r"); 
    if (ifs)
    {
		Cpt::FileSentry ifsSentry( ifs ); 
        string
            line;

        while (Cpt::fgetline(ifs, line))
        {	
	    	wostringstream wline; 
	    	for (int i = 0; i < line.length(); i++) {
	    		wline<<((wchar_t)line[i]);
	    	}
        	items_.push_back(wline.str()); 
        }
    }
}

LineTestCorpus::~LineTestCorpus() 
	{
	}

size_t LineTestCorpus::size()
	{
	return items_.size(); 
	}


std::wstring LineTestCorpus::item(size_t index)
	{
	return items_[index];
	}

std::map<std::string, LineTestCorpusRef::CorpusEntry>* LineTestCorpusRef::instances_ = NULL;

LineTestCorpusRef::LineTestCorpusRef(const char* path)
{
	if ( instances_ == NULL ) {
		instances_ = new std::map<std::string, LineTestCorpusRef::CorpusEntry>(); 
	}

	this->path_ = path; 
	if (instances_->count(path_) == 0) 
	{
		(*instances_)[path_].first = 0;
		(*instances_)[path_].second = new LineTestCorpus(path);
	}
	(*instances_)[path_].first++; 
	
	ref_ = (*instances_)[path_].second; 
}
	
LineTestCorpusRef::~LineTestCorpusRef()
{
	ref_ = NULL; 
	(*instances_)[path_].first--;
	if ((*instances_)[path_].first == 0)
	{
		delete (*instances_)[path_].second;
		instances_->erase(path_); 
	}

	if ( instances_->size() == 0 ) {
		delete instances_;
		instances_ = NULL; 
	}
}
	
size_t LineTestCorpusRef::size()
	{
	return ref_->size(); 
	}

std::wstring LineTestCorpusRef::item(size_t index)
	{
	return ref_->item( index ); 
	}
	
