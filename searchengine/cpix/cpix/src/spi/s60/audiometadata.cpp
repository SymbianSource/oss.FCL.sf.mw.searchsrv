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


#include "spi/id3.h"
#include "cpixexc.h"

#include <memory>

// from Symbian
#include <MetaDataFieldContainer.h> 
#include <MetaDataUtility.h>
#include <f32file.h>
#include <utf.h> // CnvUtfConverter

#include <sstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>

namespace Cpix {

	namespace Spi {
	
		struct Id3MetadataConversion {
			id3_frame_t frame_;
			id3_field_t field_;
			TMetaDataFieldId metadataid_;
		};
		
		Id3MetadataConversion Id3MetadataConversions[] = {
			{ ID3_FRAME_TITLE, 			ID3_FIELD_TEXT, 	EMetaDataSongTitle }, 
			{ ID3_FRAME_ARTIST, 		ID3_FIELD_TEXT, 	EMetaDataArtist },
			{ ID3_FRAME_ALBUM, 			ID3_FIELD_TEXT, 	EMetaDataAlbum },
			{ ID3_FRAME_YEAR, 			ID3_FIELD_TEXT, 	EMetaDataYear },
			{ ID3_FRAME_COMMENT, 		ID3_FIELD_TEXT, 	EMetaDataComment },
			{ ID3_FRAME_TRACK, 			ID3_FIELD_TEXT, 	EMetaDataAlbumTrack },
			{ ID3_FRAME_CONTENTTYPE, 	ID3_FIELD_TEXT, 	EMetaDataGenre },
			{ ID3_FRAME_COMPOSER, 		ID3_FIELD_TEXT, 	EMetaDataComposer },
			{ ID3_FRAME_COPYRIGHT, 		ID3_FIELD_TEXT, 	EMetaDataCopyright },
			{ ID3_FRAME_ORIGARTIST, 	ID3_FIELD_TEXT, 	EMetaDataOriginalArtist },
			{ ID3_FRAME_AUDIOFILEURL,	ID3_FIELD_URL, 		EMetaDataUrl },
			{ ID3_FRAME_USERURL, 		ID3_FIELD_URL, 		EMetaDataUserUrl },
			{ ID3_FRAME_PICTURE, 		ID3_FIELD_TEXT, 	EMetaDataJpeg },
			{ ID3_FRAME_TITLE, 			ID3_FIELD_TEXT, 	EMetaDataVendor },
			{ ID3_FRAME_RATING, 		ID3_FIELD_TEXT, 	EMetaDataRating },
			{ ID3_FRAME_UNIQUEFILEID,	ID3_FIELD_TEXT, 	EMetaDataUniqueFileIdentifier },
			{ ID3_FRAME_SONGLENGTH, 	ID3_FIELD_TEXT, 	EMetaDataDuration },
			{ ID3_FRAME_DATE, 			ID3_FIELD_TEXT, 	EMetaDataDate },
			{ ID3_FRAME_UNSYNCEDLYRICS,	ID3_FIELD_TEXT, 	EMetaDataUnsyncLyrics },
			
			{ ID3_FRAME_UNKNOWN, 		ID3_FIELD_UNKNOWN, 	EUnknownMetaDataField } // End of list
		};
	
		TMetaDataFieldId Id3ToMetadata(id3_frame_t frame, id3_field_t field) {
			for (int i = 0; Id3MetadataConversions[i].frame_ != ID3_FRAME_UNKNOWN; i++) {
				Id3MetadataConversion& c = Id3MetadataConversions[i];
				if ( c.frame_ == frame && c.field_ == field ) {
					return c.metadataid_;
				}
			}
			return EUnknownMetaDataField; 
		}
		
		bool MetadataToId3(TMetaDataFieldId metadataid, id3_frame_t& frame, id3_field_t& field) {
			for (int i = 0; Id3MetadataConversions[i].frame_ != ID3_FRAME_UNKNOWN; i++) {
				Id3MetadataConversion& c = Id3MetadataConversions[i];
				if ( c.metadataid_ == metadataid ) {
					frame = c.frame_;
					field = c.field_;
					return true;
				}
			}
			return false; 
		
		}
	
		class S60Id3Field : public Id3Field
		{
		public: 
			S60Id3Field(id3_frame_t frame, id3_field_t field, std::wstring content)
			:	frame_( frame ), 
				field_( field ), 
				content_( content ) {
				
			}

			virtual ~S60Id3Field() {}

			virtual id3_frame_t frame() {
				return frame_;
			}
			
			virtual id3_field_t field() {
				return field_;
			}
			
			virtual id3_type_t type() {
				return ID3_FIELDTYPE_STRING; // 
			}
			
			virtual std::wstring text() {
				return content_;
			}
		private: 
			id3_frame_t frame_; 
			id3_field_t field_; 
			std::wstring content_; 
		};
		
		class S60Id3EntryIterator : public Id3Metadata::poly_iterator
		{
		public: 
			
			S60Id3EntryIterator(const CMetaDataFieldContainer& container) 
				: container_( container ), i_(0) {
			
			}

			virtual ~S60Id3EntryIterator() {
			}

			virtual std::auto_ptr<Id3Field> operator++(int) {
				TMetaDataFieldId id; 
				TPtrC content = container_.At(i_++, id); 
				
				std::wstring stdcontent;
				for (int i = 0; i < content.Length(); i++) {
					stdcontent += content[i]; 
				}
				id3_frame_t frame; 
				id3_field_t field; 
				MetadataToId3( id, frame, field ); 
				return std::auto_ptr<Id3Field>( new S60Id3Field( frame, field, stdcontent ) );
			} 
			
			virtual operator bool() {
				return i_ < container_.Count(); 
			}
			
		private: 
			
			const CMetaDataFieldContainer& container_; 

			int i_;
			
		};
		
		class S60Id3Metadata : public Id3Metadata
		{
		public:
			
			S60Id3Metadata(const char* file) {
				TRAPD( err, ConstructL(file ) ); 
				if ( err != KErrNone ) {
					std::ostringstream msg; 
					msg<<"S60 Audio Metadata reader left with Symbian error code "<<err;
					throw std::invalid_argument(msg.str()); 
				}
			}
			
			virtual ~S60Id3Metadata() {
			}
			
			virtual std::auto_ptr<Id3Field> field(id3_frame_t content, id3_field_t field) {
				std::auto_ptr<Id3Field> ret; 
				TRAPD( err, ret = FieldL( content, field ) );
				if ( err != KErrNone ) {
                                    THROW_CPIXEXC("S60 Audio Metadata reader left with Symbian error code %d",
                                                  err);
				}
				return ret; 
			}
			
			virtual iterator entries() {
				return iterator( new S60Id3EntryIterator( metadata_->MetaDataFieldsL() ) );
			}
		protected: 

			virtual std::auto_ptr<Id3Field> FieldL(id3_frame_t frame, id3_field_t field) {
				TMetaDataFieldId id = Id3ToMetadata(frame, field); 
				std::auto_ptr<Id3Field> ret; 
				if (id != -1) {
					TPtrC content = metadata_->MetaDataFieldsL().Field( id ); 
					if (content != KNullDesC) {
						std::wstring stdcontent;
						for (int i = 0; i < content.Length(); i++) {
							stdcontent += content[i]; 
						}
						ret.reset(new S60Id3Field(frame, field, stdcontent)); 
					}
				}
				return ret;
			}
			
			void ConstructL(const char* file) {
				HBufC8* filename = StringToDescriptorLC(file);
				HBufC* unicodeFileName = CnvUtfConverter::ConvertToUnicodeFromUtf8L(*filename);
				CleanupStack::PushL(unicodeFileName);
				metadata_.reset(CMetaDataUtility::NewL());
				metadata_->OpenFileL(*unicodeFileName);
				CleanupStack::PopAndDestroy(2, filename);
			}
			
		private: 
			
			std::auto_ptr<CMetaDataUtility> metadata_; 

			HBufC8* StringToDescriptorLC(const char* aString)
				{
				TPtrC8 ptr(reinterpret_cast<const TUint8*>(aString));
				HBufC8* buffer = HBufC8::NewLC(ptr.Length());
				buffer->Des().Copy(ptr);
				return buffer;
				}
		};
		
		std::auto_ptr<Id3Metadata> ParseId3Metadata(const char* file) {
			return std::auto_ptr<Id3Metadata>( new S60Id3Metadata( file ) ); 
		}
		
	}
	
}

