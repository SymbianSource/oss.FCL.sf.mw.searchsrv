/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Jos van den Oever
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2006 Jos van den Oever <jos@vandenoever.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "jstreamsconfig.h"
#include "fileinputstream.h"
#include "cpixstrtools.h"
#include <cerrno>
#include <cstring>
#include <zlib.h>

namespace jstreams
    {

#ifdef __SYMBIAN32__
const int32_t FileInputStream::defaultBufferSize = 5012;
#else 
const int32_t FileInputStream::defaultBufferSize = 1048576;
#endif

    //Keep this many previous recent characters for back reference:
#define oldchar 15

    FileInputStream::FileInputStream(const char *filepath, int32_t buffersize)
        {

        file = 0;
           
        if (wcscmp((wchar_t*) (getExtension(filepath).c_str()),L"txt" )== 0)
            {
            constructTextStream(filepath,buffersize);
            }
        // Add more construction here...

        }

    wstring FileInputStream::getExtension(const char *filepath)
        {
        using namespace std;
        using namespace Cpt;

        auto_array <wchar_t> wPath(filepath);
        wstring wPathStr(wPath.get());

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
                return wExtension;

                }
           

            void FileInputStream::constructTextStream(const char *filepath, int32_t buffersize)
                {
                // try to open the file for reading
                file = fopen(filepath, "rb");
                this->filepath = filepath;
                if (file == 0)
                    {
                    // handle error
                    error = "Could not read file '";
                    error += filepath;
                    error += "': ";
                    error += strerror(errno);
                    status = Error;
                    return;
                    }
                // determine file size. if the stream is not seekable, the size will be -1
                fseek(file, 0, SEEK_END);
                size = ftell(file);
                fseek(file, 0, SEEK_SET);

                // if the file has size 0, make sure that it's really empty
                // this is useful for filesystems like /proc that report files as size 0
                // for files that do contain content
                if (size == 0)
                    {
                    char dummy[1];
                    size_t n = fread(dummy, 1, 1, file);
                    if (n == 1)
                        {
                        size = -1;
                        fseek(file, 0, SEEK_SET);
                        }
                    else
                        {
                        fclose(file);
                        file = 0;
                        return;
                        }
                    }

                // allocate memory in the buffer
                int32_t bufsize = (size <= buffersize) ?size+1 :buffersize;
                mark(bufsize);
                }
      
            FileInputStream::~FileInputStream()
                {
                char tempFile[252];
                char tempFile1[252];
                
                if (file)
                    {
                    if (fclose(file))
                        {
                        // handle error
                        error = "Could not close file '" + filepath + "'.";
                        }
                    }
                const char *temp = filepath.c_str();
                
                if(strstr(temp,"c:\\temp\\pdf"))
                    {
                        strcpy(tempFile,temp);
                        strcat(tempFile,"_lock");
                        strcpy(tempFile1,tempFile);
                        strcat(tempFile1,"_1");
                        
                        if((access(tempFile,F_OK) != 0) && (access(tempFile1,F_OK) != 0))
                            {
                            FILE *fp = fopen(tempFile,"w");
                            fclose(fp);
                            }
                        else if(access(tempFile,F_OK) == 0)
                            {
                                remove(tempFile);
                                FILE *fp = fopen(tempFile1,"w");
                                fclose(fp);
                            }
                        else
                            {
                                remove(temp);
                                remove(tempFile1);
                            }
                    }

                }

            int32_t
            FileInputStream::fillBuffer(char* start, int32_t space)
                {

                wstring extension = getExtension(this->filepath.c_str());
                
               

                if (wcscmp(extension.c_str(),L"txt")== 0)
                    {
                    return fillTextBuffer(start, space);
                    }

                // Call different fill buffer here.. for example RTF can have fillRtfBuffer
                return -1;

                }
            
          

            int32_t
            FileInputStream::fillTextBuffer(char* start, int32_t space)
                {

                if (file == 0  )
                    {
                     return -1;
                    }
                // read into the buffer
                int32_t nwritten = fread(start, 1, space, file);
                // check the file stream status
                if (ferror(file))
                    {
                    error = "Could not read from file '" + filepath + "'.";
                    fclose(file);
                    file = 0;
                    status = Error;
                    return -1;
                    }
                if (feof(file))
                    {
                    fclose(file);
                    file = 0;
                    }
                return nwritten;

                }
           

            }
