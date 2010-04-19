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

#ifndef TESTCORPUS_H_
#define TESTCORPUS_H_

#include <string>
#include <vector>
#include <map>

class TestCorpus 
{
public: 
	
	virtual size_t size() = 0;
	virtual std::wstring item(size_t index) = 0; 
	
	virtual ~TestCorpus() {}; 
	
};

class LineTestCorpus : public TestCorpus
{
public: 
	
	LineTestCorpus(const char* path); 
	~LineTestCorpus(); 
	
	virtual size_t size();
	virtual std::wstring item(size_t index); 
	
private: 
	std::vector<std::wstring> items_; 
	
};


class LineTestCorpusRef : public TestCorpus
{
public: 
	
	/** reference counter and pointer */
	typedef std::pair<size_t, LineTestCorpus*> CorpusEntry;
	
	LineTestCorpusRef(const char* path); 
	~LineTestCorpusRef(); 
	
	virtual size_t size();
	virtual std::wstring item(size_t index); 
	
private: 

	std::string path_;
	LineTestCorpus* ref_;
	
private: // static 
	
	static std::map<std::string, CorpusEntry>* instances_;
	
};


#endif /*TESTCORPUS_H_*/
