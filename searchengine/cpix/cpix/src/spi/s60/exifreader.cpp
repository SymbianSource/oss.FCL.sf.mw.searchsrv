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

#ifdef __SYMBIAN32__

#include "spi/exif.h"
#include <memory>

// from Symbian
//#include <osextensions/exifread.h>  //Old Include
//#include <osextensions/exiftag.h>   //Old Include
#include <exifread.h> 
#include <exiftag.h> 
#include <f32file.h>
#include <utf.h> // CnvUtfConverter

#include <sstream>
#include <iostream>
#include <algorithm>

/*
 * Exif service provider for S60. Using existing exif library present in mobile phones. 
 *
 * TODO: 
 *    - Read BOM from the document and use it. 
 */

namespace Cpix {

	namespace Spi {
	
		const int MAX_EXIF_SIZE = 0x20000; // 2 * 65536
		
		TExifIfdType ToS60ifd(exif_ifd_t ifd) {
			switch (ifd) {
				case EXIF_IFD_0: return EIfd0;           
				case EXIF_IFD_1: return EIfd1;
				case EXIF_IFD_EXIF: return EIfdExif;
				case EXIF_IFD_GPS: return EIfdGps;
				case EXIF_IFD_INTEROPERABILITY: return EIfdIntOp;
			}
			throw std::invalid_argument( std::string( "Not a valid cpix/spi/exif idf identifier.") ); 
		}
		
		exif_ifd_t FromS60ifd(TExifIfdType ifd) {
			switch (ifd) {
				case EIfd0: return EXIF_IFD_0;           
				case EIfd1: return EXIF_IFD_1;
				case EIfdExif: return EXIF_IFD_EXIF;
				case EIfdGps: return EXIF_IFD_GPS;
				case EIfdIntOp: return EXIF_IFD_INTEROPERABILITY;
			}
			throw std::invalid_argument( std::string( "Not a valid s60 idf identifier.") ); 
		}
		
		/**
		 * Provides interface for accessing Exif entries.
		 */
		class S60ExifEntry : public ExifEntry
		{
		public: 
			S60ExifEntry(TExifIfdType content, const CExifTag& tag, exif_bom_t bom) 
			: 	content_( FromS60ifd( content ) ), 
				tag_(tag),
				bom_(bom) {}
			
			
			virtual ~S60ExifEntry() {}
			
			virtual exif_ifd_t ifd() const {
				return content_;
			}
			
			virtual exif_tag_t tag() const {
				return tag_.TagInfo().iId; 
			}

			virtual exif_bom_t bom() const {
				return bom_;
			}
			
			virtual exif_format_t format() const {
				// Both variables follow the same exif standard 
				return static_cast<exif_format_t>( tag_.TagInfo().iDataType );
			}
			
			virtual int components() const {
				return tag_.TagInfo().iDataCount; 
			}
			
			virtual const unsigned char* data() const {
				return reinterpret_cast<const unsigned char*>( tag_.Data().Ptr() );
			}
			
			virtual int size() const {
				return tag_.Data().Size();
			}

		private: 
			
			exif_ifd_t content_;
			
			const CExifTag& tag_;
			
			exif_bom_t bom_;

		};
		
		/**
		 * A bit complex class for iterating throught all exif entries in the jpeg. 
		 * Complexity raises from the fact, that the iterator has to first fetch present
		 * idf list, and then, when iterating throught idf, it has to recover tag list 
		 * for the idf. Splitting a idf tag iterator (iterating over all tags in specific
		 * idf) could simplify the class. 
		 */
		class S60EntryIterator : public ExifMetadata::poly_iterator 
		{
		public:

			S60EntryIterator(const CExifRead& reader, exif_bom_t bom) 
			: next_(0),
			  idfs_(0),
			  idfCount_(0),
			  idf_(-1),
			  tags_(0), 
			  tagCount_(0),
			  tag_(0),
			  reader_(reader),
			  bom_(bom) {
				idfs_ = reader_.GetIfdTypesL( idfCount_ ); 
			}
						
			virtual ~S60EntryIterator() {
				delete idfs_;
				delete tags_;
			}
			
			virtual std::auto_ptr<ExifEntry> operator++(int) {
				prepareNextTagL(); 
				return std::auto_ptr<ExifEntry>( next_ ); // resets next_ 
			}
			
			virtual operator bool() {
				prepareNextTagL(); 
				return next_.get(); 
			}
			
		private: 
			
			/**
			 * Prepares next tag for use. If next tag is already fetched, does nothing
			 */
			void prepareNextTagL() {
				if ( !next_.get() ) {
					while ( tag_ == tagCount_ && idf_ < idfCount_) { // skip empty idf_s
						nextIdfL(); 
					}
					if ( tags_ ) { 
						next_.reset( new S60ExifEntry( idfs_[idf_], 
													   *reader_.GetTagL( idfs_[idf_], tags_[tag_++] ),
													   bom_ ) ); 
					}
				}
			}

			/**
			 * Gets next idf for use. Does always something. Should be called only when 
			 * idf_ < idfCount_ is true. 
			 */
			void nextIdfL() {
				tag_ = 0; 
				if ( ++idf_ < idfCount_ ) {
					delete tags_; tags_ = 0; 
					tags_ = reader_.GetTagIdsL( idfs_[idf_], tagCount_ ); 
				} else {
					delete tags_;
					tags_ = 0; 
				}
			}

		private: 
			
			std::auto_ptr<ExifEntry> next_;
			
			const TExifIfdType* idfs_;
			
			// rename idf -> ifd
			int idfCount_; // size of previous array
			
			int idf_; // points to currently used idf in idfs array
			
			const TUint16 *tags_;
			
			int tagCount_; // size of previous array
			
			int tag_; // point to current tag in the tags array

			const CExifRead& reader_;

			exif_bom_t bom_;
			
		};
		
		/**
		 * Provides access to exif metadata in jpeg file. 
		 */
		class S60ExifMetadata : public ExifMetadata 
		{
		public: 
			S60ExifMetadata(const char* file) {
				TRAPD(err, ConstructL(file)); 
			
				if ( err != KErrNone ) {
					std::ostringstream out; 
					out<<"S60 exif left with Symbian error code "<<err<<" when processing "<<file;
					std::string str = out.str(); 
//					std::cout<<str<<std::endl<<std::flush; 
					throw std::invalid_argument(str);
				}
				
				// TODO: Figure out how to get bom
				bom_ = EXIF_BOM_LITTLEENDIAN;
			}
			
			virtual ~S60ExifMetadata() {}
			
			virtual std::auto_ptr<ExifEntry> entry(exif_ifd_t ifd, exif_tag_t tag) 
			{
				TUint16 tg = static_cast<TUint16>( tag );
				TExifIfdType s60ifd = ToS60ifd( ifd );
				
				std::auto_ptr<ExifEntry> ret;
				if ( reader_->TagExists( tg, 
										 s60ifd ) )
				{
					ret.reset( new S60ExifEntry( s60ifd, 
												 *reader_->GetTagL( s60ifd, tg ),
												 bom_ ) );
				}
				return ret;  
			}
			virtual iterator entries() {
				return iterator(new S60EntryIterator(*reader_, bom_)); 
			}
			
		private: 
			
			void ConstructL(const char* filename) {
				RFile file;
				
				RFs fs;
				User::LeaveIfError( fs.Connect() ); 
				CleanupClosePushL( fs ); 
				
				HBufC8* wfilename = StringToDescriptorLC(filename);
				HBufC* unicodeFileName = CnvUtfConverter::ConvertToUnicodeFromUtf8L(*wfilename);
				CleanupStack::PushL(unicodeFileName);
				User::LeaveIfError( file.Open( fs, *unicodeFileName, EFileRead ) );
				CleanupClosePushL( file );
	
				TInt size = 0;
				file.Size(size);
				size = std::min(size, MAX_EXIF_SIZE); 

				data_.reset( HBufC8::NewL( size ) ); 
				TPtr8 bufferDes( data_->Des() ); 
				User::LeaveIfError( file.Read( bufferDes ) );
	
				CleanupStack::PopAndDestroy( 4 ); // unicodeFileName, wfilename, file, fs
				
				reader_.reset( CExifRead::NewL( data_->Des(), CExifRead::ENoTagChecking | CExifRead::ENoJpeg ) );
			}
			

			HBufC8* StringToDescriptorLC(const char* aString)
				{
				TPtrC8 ptr(reinterpret_cast<const TUint8*>(aString));
				HBufC8* buffer = HBufC8::NewLC(ptr.Length());
				buffer->Des().Copy(ptr);
				return buffer;
				}

			std::auto_ptr<CExifRead> reader_;
			std::auto_ptr<HBufC8> data_;
			exif_bom_t bom_; 
		};
		
		std::auto_ptr<ExifMetadata> ParseExifMetadata(const char* file)
			{
			return std::auto_ptr<ExifMetadata>( new S60ExifMetadata(file) ); 
			}
	}
}

#endif
