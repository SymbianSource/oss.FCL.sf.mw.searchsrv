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


#include "cpixmemtools.h"
#include "itk.h"
#include <iostream>

typedef Cpt::poly_forward_iterator<const char*> poly_zoo_iterator;
typedef Cpt::auto_iterator<const char*, poly_zoo_iterator> zoo_iterator;

const char* ZooAnimals[] = {
	"ape",
	"snake",
	"bird",
	NULL
};

class poly_zoo_iterator_ : public poly_zoo_iterator {
public: 
	poly_zoo_iterator_() : i_(0) {}
	virtual ~poly_zoo_iterator_() {
		std::cout<<"Iterator down."<<std::endl; 
	}
	virtual const char* operator++(int) {
		return ZooAnimals[i_++]; 
	}
	virtual operator bool() {
		return ZooAnimals[i_];
	}
private: 
	int i_;
};

zoo_iterator ZooIterator() {
	return zoo_iterator( new poly_zoo_iterator_() );
}

void TestAutoIterator(Itk::TestMgr * ) {
	zoo_iterator i = ZooIterator(); 
	while (i) {
		std::cout<<"Animal: "<<i++<<std::endl;
	}
}
 
Itk::TesterBase * CreateMemTests()
{
    using namespace Itk;

    SuiteTester
        * geoTests = new SuiteTester("mem");

    geoTests->add("autoiterator",
				  TestAutoIterator,
                  "autoiterator");
    
    return geoTests;
}
