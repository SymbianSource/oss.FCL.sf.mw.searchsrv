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

#include "cpixstrtools.h"

#include "ifieldfilter.h"
#include "fileparser/fileparser.h"
#include "cpixexc.h"

#include "indevicecfg.h"
#include "document.h"
#include "cpixdoc.h"
#include "common/cpixlog.h"

namespace
{

    /*************************************************************
     *
     * These are the signatures for the "interface methods" of the
     * content-specific file parsers.
     *
     */

    /**
     * This "content-specific file parser method signature" is capable
     * telling if a given path matches the file type a particular file
     * parser is can parse.
     */
    typedef bool (*FileTypeMatcher)(const char *);

    /**
     * Once a path has been matched by a particular file parser, this
     * processor will be invoked. The parser must create file type
     * specific fields on the given document based on the file at the
     * given path.
     *
     *  (a) Any common fields (like extension, file base name, etc)
     *      are added by the general file parser, so file-type
     *      specific file parsers need not bother.
     *
     *  (b) All file type specific file parsers MUST define the
     *      appclass, mimetype and excerpt fields. Function 
     *      removeStandardFields() must be called to remove the original
     *      values defined by cpixdox.cpp:initializeDoc.
     *
     *  (c) These flags MUST be used for the three mandatory fields:
     *
     *      appclass  : Field::STORE_YES | Field::INDEX_TOKENIZED
     *
     *      mime type : Field::STORE_YES | Field::INDEX_UNTOKENIZED
     * 
     *      exceprt   : Field::STORE_YES | Field::INDEX_NO
     */
    typedef void (*FileTypeProcessor)(Cpix::Document *,
                                      const char                 *);

}


namespace Cpix
{

    /************************************************************
     *
     * These are the file type specific parser "interface methods",
     * that is functions from different "modules".
     *
     */


    // from textfileparser.cpp
    bool isTextFile(const char * path);
    void processTextFile(Cpix::Document * doc,
                         const char     * path);

    // from jpegfileparser.cpp
    bool isJpegFile(const char * path);
    void processJpegFile(Cpix::Document * doc,
                         const char     * path);

    // from mp3fileparser.cpp
    bool isMp3File(const char * path);
    void processMp3File(Cpix::Document * doc,
                        const char     * path);
    
    // from PDFfileparser.cpp
    bool isPdfFile(const char * path);
    void processPdfFile(Cpix::Document * doc,
                        const char     * path);

    // from unrecognizedfileparser.cpp
    bool isUnrecognizedFile(const char * path);
    void processUnrecognizedFile(Cpix::Document * doc,
                                 const char     * path);
}



namespace Cpix
{

    void GenericFileProcessor(Cpix::Document 		 * doc,
                              const char                 * path)
    {
        using namespace std;
        using namespace Cpix;
        using namespace Cpt;

        /* OBS
        size_t
            bufSize = strlen(path) + 1;
        auto_array<wchar_t>
            wPath(new wchar_t[bufSize]);
        wPath.get()[bufSize - 1] = 0;
        mbstowcs(wPath.get(),
                 path,
                 bufSize);
        */
        auto_array<wchar_t>
            wPath(path);

        wstring
            wPathStr(wPath.get());

        size_t
            lastSep = wPathStr.find_last_of(L"/\\");
        
        // if path got here, then it is a file, therefore the last
        // character of path cannot possibly be a directory separator
        wstring
            wFullName(lastSep == string::npos 
                     ? wPathStr 
                     : wPathStr.substr(lastSep + 1));

        size_t
            lastDot = wFullName.find_last_of(L'.');

        wstring
            wBaseName(wFullName.substr(0, lastDot)),
            wExtension((lastDot == string::npos 
                        || lastDot == wFullName.length() - 1) 
                       ? wstring(L"")
                       : wFullName.substr(lastDot + 1));
        
        auto_ptr<Field>
            newField(new Field(LFULLNAME_FIELD,
                               wFullName.c_str(),
                               cpix_STORE_YES
                             | cpix_INDEX_TOKENIZED
                             | cpix_AGGREGATE_YES
                             | cpix_FREE_TEXT));

        doc->add(newField.get());
        newField.release();


        newField.reset(new Field(LBASENAME_FIELD,
                                 wBaseName.c_str(),
                                 cpix_STORE_NO 
                               | cpix_INDEX_TOKENIZED 
                               | cpix_AGGREGATE_YES
                               | cpix_FREE_TEXT));
        doc->add(newField.get());
        newField.release();

        newField.reset(new Field(LEXTENSION_FIELD,
                                 wExtension.c_str(),
                                 cpix_STORE_YES 
                               | cpix_INDEX_TOKENIZED
                               | cpix_AGGREGATE_YES
                               | cpix_FREE_TEXT));
        doc->add(newField.get());
        newField.release();
    }

}



namespace
{

    struct FileProcessorDef
    {
        FileTypeMatcher   matcher_;
        FileTypeProcessor processor_;
    };

    
    const FileProcessorDef fileProcessorDefs[] = {
        {
            Cpix::isTextFile,
            Cpix::processTextFile
        },
/* 
 * Since all the media files are getting harvested from MDS, there is no need for Jpeg and MP3 parser in cpix.
 * Source code of Jpeg and MP3 parser are note removed because incase future if we revert to 
 * CPIX parsers, we can use this code.
*/        
        
#if 0  
        {
            Cpix::isJpegFile,
            Cpix::processJpegFile
        },

        {
            Cpix::isMp3File,
            Cpix::processMp3File
        },
#endif
		{
            Cpix::isPdfFile,
            Cpix::processPdfFile
        },
 
        // TODO
        // enter more file processor definitions here

        
        // This last file parser accepts ALL types, and only defines a
        // generic appclass (FILEAPPCLASS). So any specific type
        // parser must be added before this "catch-all" clause.
        {
            Cpix::isUnrecognizedFile,
            Cpix::processUnrecognizedFile
        },

        // end of list
        { NULL,
          NULL
        }
    };


    FileTypeProcessor GetFileTypeProcessor(const char * path)
    {
        FileTypeProcessor
            rv = NULL;

        const FileProcessorDef
            * fpd = fileProcessorDefs;
        
        while (fpd->matcher_ != NULL)
            {
                if ( (*fpd->matcher_)(path) )
                    {
                        rv = fpd->processor_;
                        break;
                    }

                ++ fpd;
            }

        return rv;
    }


}


namespace Cpix
{

    void removeStandardFields(Cpix::Document * doc)
    {
        doc->removeField(LCPIX_EXCERPT_FIELD);
        doc->removeField(LCPIX_APPCLASS_FIELD);
        doc->removeField(LCPIX_MIMETYPE_FIELD);
    }
    
    /**
    * check whether the given file is parsable. Since the 
    * media files are getting harvested from MDS server, the JPEG and
    * MP3 are there in blocked list.
    */
    
    bool isFileAllowedToParse(const wchar_t  * wPathName)
        {
           bool rv = false;
           const wchar_t *parsedList[] = {
				   L"pdf",
				   L"txt"
		   };
           wstring
                       wPathStr(wPathName);
       
           if(!wPathStr.empty())
               {
                   size_t
                       lastSep = wPathStr.find_last_of(L"/\\");
               
                   // if path got here, then it is a file, therefore the last
                   // character of path cannot possibly be a directory separator
                   wstring
                       wFullName(lastSep == string::npos 
                                ? wPathStr 
                                : wPathStr.substr(lastSep + 1));

                   size_t
                       lastDot = wFullName.find_last_of(L'.');

                   wstring
                       wExtension((lastDot == string::npos 
                                   || lastDot == wFullName.length() - 1) 
                                  ? wstring(L"")
                                  : wFullName.substr(lastDot + 1));

				   if (!wExtension.empty()) {
					   for (int i=0; i < (sizeof(parsedList)/sizeof(char*)); i++) {
						   if (!wcscmp ((wchar_t*)wExtension.c_str(), parsedList[i])) {
							   rv = true;
							   break;
						   }
					   }
               		}
               }    
        	   return rv;
        }

    bool FileParser::process(Cpix::Document * doc)
    {
        bool
            rv = false;

        const wchar_t
            * docUid = doc->get(LCPIX_DOCUID_FIELD);

        if (docUid == NULL)
            {
                logMsg(CPIX_LL_WARNING,
                       "Corrupt document - no doc uid field");
            }
        else
            {
                if(isFileAllowedToParse(docUid))
                    {
                          Cpt::auto_array<char>  path(docUid);
        
                            try
                                {
            
                                    FileTypeProcessor
                                        ftp = GetFileTypeProcessor(path.get());
            
                                    if (ftp == NULL)
                                        {
                                            logMsg(CPIX_LL_WARNING,
                                                   "File %s: unrecognized type, not parsing file type specifics.\n",
                                                   path.get());
                                        }
                                    else
                                        {
                                            (*ftp)(doc,
                                                   path.get());
                                        }
                                    rv = true; // return true so that doc is indexed
                                }
                            catch (std::exception & exc)
                                {
                                    logMsg(CPIX_LL_ERROR,
                                           "File %s: parser error: %s\n",
                                           path.get(),
                                           exc.what());
                                }
                            catch (...)
                                {
                                    logMsg(CPIX_LL_ERROR,
                                           "File %s: unknown parse error\n",
                                           path.get());
                                }
                      }
             }
                      

        return rv;
    }
    

    FileParser::~FileParser()
    {
        ;
    }


}
