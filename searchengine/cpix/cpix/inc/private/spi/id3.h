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


#ifndef ID3_H_
#define ID3_H_

#include "cpixmemtools.h"

namespace Cpix {

	namespace Spi {
	
		enum id3_frame_t {
			ID3_FRAME_UNKNOWN = 0, 
			ID3_FRAME_UNIQUEFILEID,				// UFID
			ID3_FRAME_AUDIOCRYTO,
			ID3_FRAME_PICTURE,
			ID3_FRAME_AUDIOSEEKPOINT,
			ID3_FRAME_COMMENT,					// COMM
			ID3_FRAME_INVOLVEDPEOPLE,
			ID3_FRAME_LINK,						// LINK
			ID3_FRAME_OWNERSHIP,
			ID3_FRAME_RATING,					// POPM or popularimeter 
			ID3_FRAME_ALBUM,
			ID3_FRAME_COMPOSER,
			ID3_FRAME_CONTENTTYPE,				// TCON, Content type.
			ID3_FRAME_COPYRIGHT,
			ID3_FRAME_DATE,
			ID3_FRAME_TAGGINGTIME,
			ID3_FRAME_INVOLVEDPEOPLE2,
			ID3_FRAME_FILETYPE,
			ID3_FRAME_TIME,						// TIME
			ID3_FRAME_CONTENTGROUP,				// TIT1
			ID3_FRAME_TITLE,					// TIT2
			ID3_FRAME_SUBTITLE,					// TIT3
			ID3_FRAME_INITIALKEY,
			ID3_FRAME_LANGUAGE,
			ID3_FRAME_SONGLENGTH,
			ID3_FRAME_MUSICIANCREDITLIST,
			ID3_FRAME_MEDIATYPE,
			ID3_FRAME_MOOD,
			ID3_FRAME_ORIGALBUM,
			ID3_FRAME_ORIGFILENAME,
			ID3_FRAME_ORIGLYRICIST,
			ID3_FRAME_ORIGARTIST,
			ID3_FRAME_ORIGYEAR,
			ID3_FRAME_FILEOWNER,
			ID3_FRAME_ARTIST,
			ID3_FRAME_BAND,
			ID3_FRAME_CONDUCTOR,
			ID3_FRAME_PUBLISHER,
			ID3_FRAME_TRACK,
			ID3_FRAME_RECORDINGDATES,
			ID3_FRAME_SIZE,
			ID3_FRAME_USERTEXT,
			ID3_FRAME_YEAR,						// TYER
			ID3_FRAME_UNSYNCEDLYRICS,			// USLT
			ID3_FRAME_AUDIOFILEURL, 			// WOAF
			ID3_FRAME_USERURL, 					// WXXX
		};
		
		enum id3_field_t {
			ID3_FIELD_UNKNOWN = 0, 
			ID3_FIELD_TEXT,
			ID3_FIELD_URL
		};
		enum id3_type_t {
			ID3_FIELDTYPE_NONE = -1, 
			ID3_FIELDTYPE_INTEGER = 0, 
			ID3_FIELDTYPE_BINARY, 
			ID3_FIELDTYPE_STRING, 
			ID3_FIELDTYPE_NUMTYPES 
		};
		
		class Id3Field 
		{
		public: 
			virtual ~Id3Field();
			
			virtual id3_frame_t frame() = 0; 
			virtual id3_field_t field() = 0; 
			virtual id3_type_t type() = 0; 
			virtual std::wstring text() = 0;
		};
		
		class Id3Metadata 
		{
		public: 
			typedef Cpt::poly_forward_iterator<std::auto_ptr<Id3Field> > poly_iterator;
			typedef Cpt::auto_iterator<std::auto_ptr<Id3Field>, poly_iterator> iterator;
			virtual ~Id3Metadata();
			
			virtual std::auto_ptr<Id3Field> field(id3_frame_t frame, id3_field_t field) = 0;
			virtual iterator entries() = 0;   
		};
		
		std::auto_ptr<Id3Metadata> ParseId3Metadata(const char* file);
		
	}
	
}


#endif /* ID3_H_ */
