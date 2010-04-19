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


#ifndef EXIF_H_
#define EXIF_H_

#include "cpixmemtools.h"
#include <memory>
#include <exception>
#include <stdexcept>
#include "cpixgeotools.h"

namespace Cpix {

	namespace Spi {
	
		enum { XF_DATETIME_SIZE = 20 };
	
		/**
		 * Byte order
		 */
		enum exif_bom_t {
			EXIF_BOM_BIGENDIAN, 
			EXIF_BOM_LITTLEENDIAN
		};

		enum exif_ifd_t {
			EXIF_IFD_0 = 0,          
			EXIF_IFD_1,              
			EXIF_IFD_EXIF,    
			EXIF_IFD_GPS,
			EXIF_IFD_INTEROPERABILITY
		};
		
		const char* GetIfdName( exif_ifd_t ifd ); 
		
		typedef int exif_tag_t;
		
		enum {
			EXIF_TAG_DATE_TIME_ORIGINAL = 0x9003, 
			EXIF_TAG_USER_COMMENT		= 0x9286,
			EXIF_TAG_XP_TITLE			= 0x9c9b,
			EXIF_TAG_XP_COMMENT			= 0x9c9c,
			EXIF_TAG_XP_AUTHOR			= 0x9c9d,
			EXIF_TAG_XP_KEYWORDS	    = 0x9c9e,
			EXIF_TAG_XP_SUBJECT			= 0x9c9f
		};
		
		enum {
			EXIF_TAG_GPS_LATITUDE_REF   = 0x0001,
			EXIF_TAG_GPS_LATITUDE       = 0x0002,
			EXIF_TAG_GPS_LONGITUDE_REF  = 0x0003,
			EXIF_TAG_GPS_LONGITUDE      = 0x0004
		};
		
		enum exif_format_t {
			EXIF_FORMAT_UNKNOWN       	=  0,
			EXIF_FORMAT_BYTE       		=  1,
			EXIF_FORMAT_ASCII      		=  2,
			EXIF_FORMAT_SHORT      		=  3,
			EXIF_FORMAT_LONG       		=  4,
			EXIF_FORMAT_RATIONAL   		=  5,
			EXIF_FORMAT_SIGNED_BYTE 	=  6,
			EXIF_FORMAT_UNDEFINED  		=  7,
			EXIF_FORMAT_SIGNED_SHORT 	=  8,
			EXIF_FORMAT_SIGNED_LONG     =  9,
			EXIF_FORMAT_SIGNED_RATIONAL = 10,
			EXIF_FORMAT_FLOAT      		= 11,
			EXIF_FORMAT_DOUBLE     		= 12,
		};
		
		uint32_t ParseExifLong(const unsigned char* data, int i, exif_bom_t bom); 
		uint16_t ParseExifShort(const unsigned char* data, int i, exif_bom_t bom); 
		
		class ExifEntry 
		{
		public: 
			virtual ~ExifEntry();
			
			// all content
			virtual exif_ifd_t ifd() const = 0; 
			virtual exif_tag_t tag() const = 0;
			virtual exif_format_t format() const = 0; 
			virtual int components() const = 0; 

			virtual int size() const = 0; 
			virtual const unsigned char* data() const = 0;
			virtual exif_bom_t bom() const = 0; 
			
			/**
			 * Checks if the given assumed format is the same as this entry's format
			 * and if index i is within bounds 
			 */
			inline void check(exif_format_t format, int i) {
				if (this->format() != format) {
					throw std::logic_error( std::string( "Entry is of wrong type.") ); 
				}
				if (i < 0 || i >= components()) {
					throw std::invalid_argument( std::string( "Index out of range." ) ); 
				}
			}
			
			/**
			 * Interpret the entry's data as user comment and returns the comment string. 
			 * Understands user comments in both UCS and ASCII formats.
			 */
			std::wstring asUserComment();
			
			inline uint32_t numeratorAt(int i) {
				check(EXIF_FORMAT_RATIONAL, i); 
				return ParseExifLong(data(), i*2, bom()); // assume intel BOM
			}
			
			inline uint32_t denominatorAt(int i) {
				check(EXIF_FORMAT_RATIONAL, i); 
				return ParseExifLong(data(), i*2 + 1, bom()); // assume intel BOM
			}
			
			inline Cpt::ExifRational rationalAt(int i) {
				check(EXIF_FORMAT_RATIONAL, i); 
				return Cpt::ExifRational(ParseExifLong(data(), i*2, bom()), 
										 ParseExifLong(data(), i*2+1, bom()));
			}

		};
		
		class ExifMetadata 
		{
		public: 
			typedef Cpt::poly_forward_iterator<std::auto_ptr<ExifEntry> > poly_iterator;
			typedef Cpt::auto_iterator<std::auto_ptr<ExifEntry>, poly_iterator > iterator;
			
			virtual ~ExifMetadata();
			
			virtual std::auto_ptr<ExifEntry> entry(exif_ifd_t ifd, exif_tag_t tag) = 0;
			
			/**
			 * The returned iterator provides java-style, C++ forward iteratorish style of iteration.
			 * The actual iterator holds simply ownership to the actual polymorphic iterator and
			 * acts like auto_pointer. The 'real iterator' (that is polymorphic) is always located
			 * in heap. Iterator is used in following way: 
			 * 
			 * ExifMetadata::iterator i = data->entries(); 
			 * while ( i ) {
			 *   auto_ptr<ExifEntry> entry = i++; 
			 *   // Use entry here
			 * }
			 */
			virtual iterator entries() = 0;
		};
		
		/**
		 * Parses exif metadata. Throws on failure. 
		 */
		std::auto_ptr<ExifMetadata> ParseExifMetadata(const char* file); 
		
	} // Spi
	
} // Cpix


#endif /* EXIF_H_ */
