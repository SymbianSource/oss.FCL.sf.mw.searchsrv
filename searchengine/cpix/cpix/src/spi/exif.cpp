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
 * exif.h
 *
 *  Created on: May 7, 2009
 *      Author: admin
 */

#include "spi/exif.h"
#include <stdexcept>
#include <string>

namespace Cpix {

	namespace Spi {

		const char* GetIfdName( exif_ifd_t ifd ) {
			switch ( ifd ) {
				case EXIF_IFD_0: return "0";           
				case EXIF_IFD_1: return "1";           
				case EXIF_IFD_EXIF: return "exif";           
				case EXIF_IFD_GPS: return "gps";           
				case EXIF_IFD_INTEROPERABILITY: return "interoperability";           
				default: return "invalid";
			}
		}

		ExifEntry::~ExifEntry()
		{
		}
		
		std::wstring ExifEntry::asUserComment() {
			if (this->tag() != EXIF_TAG_USER_COMMENT) {
				throw std::logic_error( std::string( "Tag should be user comment.") ); 
			}
			if (this->format() != EXIF_FORMAT_UNDEFINED) {
				throw std::logic_error( std::string( "Format should be undefined.") ); 
			}
			int size = this->size(); 
			const unsigned char* data = this->data(); 
			if (size < 8) {
				throw std::logic_error( std::string( "User comment field must be 8 bytes or longer." ) ); 
			}
			if (!memcmp(data, "UNICODE\0", 8)) {
				exif_bom_t bom = this->bom(); 
				std::wstring ret; 
				for (int i = 8; i + 1 < size; i+=2) {
					wchar_t c = (wchar_t)(ParseExifShort(&data[i], 0, bom));
					if (!c) break;	
					ret += c; 
				}
				return ret; 
			} else if (!memcmp(data, "ASCII\0\0\0", 8)) {
				std::wstring ret; 
				for (int i = 8; i < size && data[i]; i++) {
					ret += data[i]; 
				}
				return ret; 
			} else {
				throw std::logic_error( std::string( "User comment is not in ASCII or unicode." ) ); 
			}
		}

		
		ExifMetadata::~ExifMetadata()
		{
		}

		uint32_t ParseExifLong(const unsigned char* data, int i, exif_bom_t bom)
		{
			data += i*4; 
			uint32_t ret; 
			switch (bom) 
			{
				case EXIF_BOM_LITTLEENDIAN: 
					ret =  data[0];
					ret += data[1]<<8;
					ret += data[2]<<16; 
					ret += data[3]<<24; 
					break;
				case EXIF_BOM_BIGENDIAN: 
					ret =  data[3];
					ret += data[2]<<8;
					ret += data[1]<<16; 
					ret += data[0]<<24; 
					break;
				default: 
					throw new std::invalid_argument(std::string("Bad exif byte order marker")); 
			}
			return ret;
		}

		uint16_t ParseExifShort(const unsigned char* data, int i, exif_bom_t bom)
		{
			data += i*2; 
			uint32_t ret = 0; 
			switch (bom) 
			{
				case EXIF_BOM_LITTLEENDIAN: 
					ret =  data[0];
					ret += data[1]<<8;
					break;
				case EXIF_BOM_BIGENDIAN: 
					ret += data[1]<<0; 
					ret += data[0]<<8; 
					break;
				default: 
					throw new std::invalid_argument(std::string("Bad exif byte order marker")); 
			}
			return ret;
		}


	}
}
