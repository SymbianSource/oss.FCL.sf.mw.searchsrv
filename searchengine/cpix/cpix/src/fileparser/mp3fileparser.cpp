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

#include <wchar.h>
#include <string.h>

#include <fstream>
#include <string>
#include <sstream>

#include "CLucene.h"

#include "indevicecfg.h"

#include "cpixidxdb.h"
#include "cluceneext.h"
#include "cpixexc.h"

#include "cpixtools.h"

#include "spi/id3.h"

#include "document.h"
#include "fileparser/fileparser.h" // removeStandardFields()

namespace
{
    const char EXTENSION[]            = ".mp3";
    const char EXTENSION_UPPER[]      = ".MP3";
    const char PATH_SEPARATOR[] = "/\\";
}

namespace Cpix
{

    bool isMp3File(const char * path)
    {
        size_t
            length = strlen(path);

        const char
            * ext = path + length - strlen(EXTENSION);
        
        bool
            rv = false;

        if ((strcmp(EXTENSION, ext) == 0)
            ||(strcmp(EXTENSION_UPPER, ext) == 0))
            {
            rv = true;
            }
        
        if (rv)
        {
			try {
				if (!Cpix::Spi::ParseId3Metadata(path).get()) {
					rv = false; // empty autopointer returned
				}
			} catch (...) {
				rv = false; // ignore			
			}
        }

        return rv;
    }
    
    using namespace std;
	using namespace Cpix::Spi;
	
    void addDocId3TextField(Document* doc, const wchar_t* field, Id3Metadata& data, id3_frame_t frameId, wostringstream* excerpt)
    {
		auto_ptr<Id3Field> fld = data.field(frameId, ID3_FIELD_TEXT);
		if ( fld.get() )
           {
            std::wstring text = fld->text(); 
			if ( excerpt ) 
				{
				(*excerpt)<<text<<L" "; 
				}
                        
			auto_ptr<Field>
				newField(new Field(field,
								   text.c_str(),
								   cpix_STORE_YES | cpix_INDEX_TOKENIZED));

			doc->add(newField.get());

            newField.release();
		}
    }	
	
    void addDocId3Genre(Document* doc, Id3Metadata& data, wostringstream* excerpt)
    {
        auto_ptr<Id3Field> field = data.field(ID3_FRAME_CONTENTTYPE, ID3_FIELD_TEXT);
        if ( field.get() )
            {
                std::wstring text = field->text(); 
                if ( excerpt ) 
                    {
                        (*excerpt)<<text<<L" "; 
                    }
                
                auto_ptr<Field>
                    newField(new Field( LGENRE_FIELD,
                                        text.c_str(),
                                        cpix_STORE_YES | cpix_INDEX_TOKENIZED));
                doc->add(newField.get());
                newField.release();
            }
    }

    void processMp3File(Cpix::Document 	* doc,
                        const char  	* path)
    {
        // Assign parsed file
		auto_ptr<Id3Metadata> data = ParseId3Metadata(path); 
		
        wostringstream excerpt; 
		
        // remove these fields before creating new values for them.
        removeStandardFields(doc);

        addDocId3TextField(doc, LTITLE_FIELD, 	*data, ID3_FRAME_TITLE, &excerpt);
        addDocId3TextField(doc, LSUBTITLE_FIELD,*data, ID3_FRAME_SUBTITLE, NULL);
        addDocId3TextField(doc, LTRACK_FIELD, 	*data, ID3_FRAME_TRACK, NULL);
        addDocId3TextField(doc, LALBUM_FIELD, 	*data, ID3_FRAME_ALBUM, &excerpt);
        addDocId3TextField(doc, LARTIST_FIELD, 	*data, ID3_FRAME_ARTIST, &excerpt);
        addDocId3TextField(doc, LBAND_FIELD, 	*data, ID3_FRAME_BAND, NULL);	    
	    
        addDocId3Genre( doc, *data, NULL ); 
        //	    addDocId3TextField(doc, LGENRE_FIELD, tag, ID3FID_CONTENTTYPE, NULL, aggregate);
	    
        wstring excerptStr = excerpt.str();
	    
        if (excerptStr.length() == 0) 
	    { // excerpt fallback, use file name instead
            string aExcerpt; 
            aExcerpt.append(path);
	        string::size_type
	            pos = string::npos;
	        do {
	            pos = aExcerpt.find_last_of(PATH_SEPARATOR);
	            if (pos != string::npos)
	                {
                            aExcerpt = aExcerpt.erase(0, pos + 1);
	                }
	        } while (pos != string::npos);
	        excerpt<<aExcerpt.c_str(); 
	        excerptStr = excerpt.str(); 
	    }
        
        doc->setExcerpt(excerptStr.c_str());
        doc->setAppClass(MP3APPCLASS);
        doc->setMimeType(LMP3FILE_MIMETYPE);
            
    }

}
