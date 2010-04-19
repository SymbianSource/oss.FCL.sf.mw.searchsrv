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

#include "document.h"
#include "cpixdoc.h"
#include "fileparser/fileparser.h" // removeStandardFields()

#include "CLucene.h"

#include "indevicecfg.h"

#include "cpixidxdb.h"
#include "cluceneext.h"

namespace
{
    const char EXTENSION[]       = ".txt";
    const char EXTENSION_UPPER[] = ".TXT";

    const char DEFAULT_ENCODING[] = "UTF-8";
    
    /**
     * Returns 1 on success, 0 on eof. 
     */
    int clgetline(lucene::util::Reader& reader, std::wstring& line) 
    	{
        line = L""; 
        
        // read line 
        while (true) 
        	{
				int c = reader.read(); 
				switch (c) {
					case -1: // EOF
						return line.length() > 0; 
					case '\n': // line break
					case '\r': // line break
						return 1;
					default:
						line += static_cast<wchar_t>(c);
						if  (line.length() > 500)
							return 1;
				}
        	}
    	}

    void getExcerptOfFile(wchar_t       * dst,
                          const char    * path,
                          size_t          maxWords,
                          size_t          bufSize)
    {
        using namespace std;
        using namespace lucene::util;
                
        // Lucene reader can do UTF-8 magic, so let's use it
		FileReader file( path, DEFAULT_ENCODING ); 
		
		if ( file.reader->getStatus() == jstreams::Ok ) 
			{
				cpix_EPIState
					epiState;
				cpix_init_EPIState(&epiState);
		
				wstring
					line;
		
				while (bufSize > 0 && maxWords > 0 && clgetline(file, line))
					{
						dst = cpix_getExcerptOfWText(dst,
													 line.c_str(),
													 &maxWords,
													 &bufSize,
													 &epiState);
					}
			}
    }

}


namespace Cpix
{

    bool isTextFile(const char * path)
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

        return rv;
    }

    
    void processTextFile(Cpix::Document * doc,
                         const char     * path)
    {
        using namespace lucene::util;
        using namespace Cpix;

        // remove these fields before creating new values for them.
        removeStandardFields(doc);

        std::auto_ptr<Field>
            newField(new Field(CONTENTS_FIELD,
                               new FileReaderProxy(path,
                                                   DEFAULT_ENCODING),
                               cpix_STORE_NO | cpix_INDEX_TOKENIZED));
        doc->add(newField.get());
        newField.release();
        
        wchar_t
            excerpt[512];
        getExcerptOfFile(excerpt,
                         path,
                         10, // max words
                         sizeof(excerpt) / sizeof(wchar_t));
        
        doc->setExcerpt(excerpt);
        doc->setAppClass(TEXTAPPCLASS);
        doc->setMimeType(LTEXTFILE_MIMETYPE);
        // Always perform generic file processing
        GenericFileProcessor(doc,path);

        
        // TODO should we index dates (last modified)?
    }

}
