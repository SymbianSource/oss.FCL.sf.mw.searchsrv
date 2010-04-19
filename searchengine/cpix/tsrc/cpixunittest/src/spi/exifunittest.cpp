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
#include "spi/exif.h"
#include "cpixgeotools.h"
#include "cpixstrtools.h"
#include <iostream>
#include "itk.h"
#include "config.h"

const char* ExifJpegTestCorpusFiles[] = {
	JPG_TEST_CORPUS_PATH "\\08012009047.jpg",
	JPG_TEST_CORPUS_PATH "\\08012009048.jpg",
	JPG_TEST_CORPUS_PATH "\\New_York.jpg",
	0
};

void DumpExifDataFrom(const char* file) {
	using namespace std;
	using namespace Cpix::Spi;

	printf("Dumping exif contents for !%s\n", (file+1));
	std::auto_ptr<ExifMetadata> data;
	try {
		data = ParseExifMetadata( file );
	} catch (std::exception& e) {
		printf("Failed because \"%s\"\n", e.what()); 
		return; 
	}
	
	if (data.get()) 
	{
		printf("Loading data succeeded.\n");
		ExifMetadata::iterator i = data->entries();
		while ( i ) {
			std::auto_ptr<ExifEntry> entry = i++;
			printf("Ifd: %s\n", GetIfdName(entry->ifd()));
			printf("Tag: %d %p\n", (int)entry->tag(), (void*)entry->tag());
			printf("Value: ");

			switch (entry->format()) {
				 case EXIF_FORMAT_BYTE:
					 printf("? (byte)"); 
					 break;
				 case EXIF_FORMAT_ASCII:
					 {
						 Cpt::auto_array<char> buf( new char[entry->size()+1] ); 
						 memcpy(buf.get(), entry->data(), entry->size());
						 buf.get()[entry->size()] = '\0';
						 printf("%s (ascii)", buf.get());
					 }
					 break;
				 case EXIF_FORMAT_SHORT:
					 printf("? (short)"); 
					 break;
				 case EXIF_FORMAT_LONG:
					 printf("? (long)"); 
					 break;
				 case EXIF_FORMAT_RATIONAL:
					 for (int i = 0; i < entry->components(); i++) {
						 printf("%d/%d", entry->rationalAt(i).numerator_, entry->rationalAt(i).denominator_);
						 if (i + 1< entry->components()) printf(" "); 
					 }
					 printf(" (rational)"); 
					 break;
				 case EXIF_FORMAT_SIGNED_BYTE:
					 printf("? (sbyte)"); 
					 break;
				 case EXIF_FORMAT_UNDEFINED:
					 printf("? (undefined)");
					 break;
				 case EXIF_FORMAT_SIGNED_SHORT:
					 printf("? (sshort)"); 
					 break;
				 case EXIF_FORMAT_SIGNED_LONG:
					 printf("? (slong)"); 
					 break;
				 case EXIF_FORMAT_SIGNED_RATIONAL:
					 printf("? (srational)"); 
					 break;
				 case EXIF_FORMAT_FLOAT:
					 printf("? (float)"); 
					 break;
				 case EXIF_FORMAT_DOUBLE:
					 printf("? (double)"); 
					 break;
				 default: 
					 printf("? (?)"); 
					 break; 
			 }
			 printf("\n"); 
		}
	}
	printf("\n"); 
}

void ExractExifDataFrom(const char* file) {
	using namespace std;
	using namespace Cpix::Spi; 
	printf("Extracting gps information from !%s\n", (file+1));
	
	std::auto_ptr<ExifMetadata> data = ParseExifMetadata( file );
	if (data.get()) 
	{
		printf("Loading data succeeded\n"); 
		auto_ptr<ExifEntry> usercomment 	= data->entry(EXIF_IFD_EXIF, EXIF_TAG_USER_COMMENT);
		if (usercomment.get()) {
			printf("Comment: %S\n", usercomment->asUserComment().c_str()); 
		}
		auto_ptr<ExifEntry> xpkeywords 		= data->entry(EXIF_IFD_0, EXIF_TAG_XP_KEYWORDS);
		if (xpkeywords.get()) {
			printf("XP Keywords: %S\n", (const wchar_t*)xpkeywords->data());
		}
		
		auto_ptr<ExifEntry> latitude_ref 	= data->entry(EXIF_IFD_GPS, EXIF_TAG_GPS_LATITUDE_REF);
		auto_ptr<ExifEntry> latitude 		= data->entry(EXIF_IFD_GPS, EXIF_TAG_GPS_LATITUDE);
		auto_ptr<ExifEntry> longitude_ref 	= data->entry(EXIF_IFD_GPS, EXIF_TAG_GPS_LONGITUDE_REF);
		auto_ptr<ExifEntry> longitude 		= data->entry(EXIF_IFD_GPS, EXIF_TAG_GPS_LONGITUDE);

		char lat_ref[2];
		char lon_ref[2];
		Cpt::ExifRational lat[3];
		Cpt::ExifRational lon[3];
		
		strncpy(lat_ref, (const char *)latitude_ref->data(), latitude_ref->size());
		strncpy(lon_ref, (const char *)longitude_ref->data(), longitude_ref->size());
		
		for (int i = 0; i < 3; i++) 
			{
			lat[i] = latitude->rationalAt(i);
			lon[i] = longitude->rationalAt(i);
			}
		
		printf("Latitude\n");
		printf("Reference: %s\n", lat_ref); 
		printf("Degrees: %d/%d\n", lat[0].numerator_, lat[0].denominator_); 
		printf("Minutes: %d/%d\n", lat[1].numerator_, lat[1].denominator_); 
		printf("Seconds: %d/%d\n", lat[2].numerator_, lat[2].denominator_); 

		printf("Longitude: \n"); 
		printf("Reference: %s\n",lon_ref);
		printf("Degrees: %d/%d\n", lon[0].numerator_, lon[0].denominator_); 
		printf("Minutes: %d/%d\n", lon[1].numerator_, lon[1].denominator_); 
		printf("Seconds: %d/%d\n", lon[2].numerator_, lon[2].denominator_); 
	}
	printf("\n");  
}

void ExifExtractionTest(Itk::TestMgr* /*testMgr*/ ) {
	for (int i = 0; ExifJpegTestCorpusFiles[i]; i++) {
		ExractExifDataFrom(ExifJpegTestCorpusFiles[i]);
	}
}

void ExifDumpTest(Itk::TestMgr* /*testMgr*/ ) {
	for (int i = 0; ExifJpegTestCorpusFiles[i]; i++) {
		DumpExifDataFrom(ExifJpegTestCorpusFiles[i]);
	}
}

Itk::TesterBase * CreateExifTests()
{
    using namespace Itk;

    SuiteTester
        * whiteBox = new SuiteTester("exif"); // default context

    whiteBox->add("extract",
                  &ExifExtractionTest,
                  "extract");

    whiteBox->add("dump",
                  &ExifDumpTest,
                  "dump");

    return whiteBox;
}
