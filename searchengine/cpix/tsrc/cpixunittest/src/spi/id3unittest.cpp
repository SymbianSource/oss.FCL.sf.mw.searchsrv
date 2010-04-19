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
/*
 * exifunittest.cpp
 *
 *  Created on: 8.5.2009
 *      Author: arau
 */
#include "spi/id3.h"
#include "cpixstrtools.h"
#include "itk.h"

#include <iostream>

#include "config.h"


void DumpId3DataFrom(const char* file) 
	{
	using namespace std;
	using namespace Cpix::Spi;

	printf("Dumping id3 information from !%s\n", (file+1)); 
	auto_ptr<Id3Metadata> metadata( ParseId3Metadata( file ) );  
	
	Id3Metadata::iterator i = metadata->entries(); 
	while (i) {
		auto_ptr<Id3Field> field = i++; 
		
		// Note: these are local ids, as 4 letter id3 ids are not accessible 
		printf("Frame Id: %d\n", field->frame()); 
		printf("Field Id: %d\n", field->field()); 
		
		if (field->type() == ID3_FIELDTYPE_STRING ) {
			printf("Value:%S\n", field->text().c_str());
		} else {
			printf("Value: ?\n");
		}
	}

}

void ExtractId3DataFrom(const char* file) 
{
	using namespace std;
	using namespace Cpix::Spi;
	
	printf("Extracting id3 information from !%s\n", (file+1));  
	auto_ptr<Id3Metadata> metadata( ParseId3Metadata( file ) );  
	
	printf("Title: %S\n", metadata->field( ID3_FRAME_TITLE, ID3_FIELD_TEXT )->text().c_str());  
	printf("Album: %S\n", metadata->field( ID3_FRAME_ALBUM, ID3_FIELD_TEXT )->text().c_str());  
}

void DumpingId3DataTest(Itk::TestMgr* /*testMgr*/) 
{
	for (int i = 0; Mp3TestCorpus[i]; i++) {
		DumpId3DataFrom(Mp3TestCorpus[i]);
	}
}

void ExtractingId3DataTest(Itk::TestMgr* /*testMgr*/) 
{
	for (int i = 0; Mp3TestCorpus[i]; i++) {
		ExtractId3DataFrom(Mp3TestCorpus[i]);
	}
}

Itk::TesterBase * CreateId3Tests()
{
    using namespace Itk;

    SuiteTester
        * id3 = new SuiteTester("id3");
    
    id3->add("extract",
             &ExtractingId3DataTest,
             "extract");

    id3->add("dump",
             &DumpingId3DataTest,
             "dump");

    return id3;
}
