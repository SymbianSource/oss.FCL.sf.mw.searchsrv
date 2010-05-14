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

#include <zlib.h>
#include <wchar.h>

#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <iostream>

#include <fcntl.h>


#include "document.h"
#include "cpixdoc.h"
#include "fileparser/fileparser.h" // removeStandardFields()

#include "CLucene.h"

#include "indevicecfg.h"

#include "cpixidxdb.h"
#include "cluceneext.h"
#include "cpixstrtools.h"

namespace
{
    /**
     * Returns 1 on success, 0 on eof. 
     */
    int getPDFExcerpt(const char* filePath,std::wstring& line) 
        {
        line = L"";
        int wordCount = 0;
        
        // read line 
        FILE *fp = fopen(filePath,"rb");
        while (true) 
            {
                int c = fgetc(fp); 
                switch (c) {
                    case -1: // EOF
                        fclose(fp);
                        return line.length() > 0; 
                    case '\n': // line break
                    case '\r': // line break
                        fclose(fp);
                        return 1;
                    default:
                        line += static_cast<wchar_t>(c);
                        if ( c == ' ')
                            wordCount ++;
                        
                        if  ((line.length() > MAX_EXCERPT_LENGTH) ||  wordCount == 10 )
                            {
                                fclose(fp);
                                return 1;
                            }
                }
            }
        }

    }

using namespace std;
using namespace Cpt;

namespace Cpix
    {
    
    //Keep this many previous recent characters for back reference:
#define oldchar 15
    
    ssize_t FindStringInBuffer (char* buffer, char* search, size_t buffersize)
                    {
                    char* buffer0 = buffer;

                    size_t len = strlen(search);
                    bool fnd = false;
                    while (!fnd)
                        {
                        fnd = true;
                        for (size_t i=0; i<len; i++)
                            {
                            if (buffer[i]!=search[i])
                                {
                                fnd = false;
                                break;
                                }
                            }
                        if (fnd) return buffer - buffer0;
                        buffer = buffer + 1;
                        if (buffer - buffer0 + len > buffersize) return -1;
                        }
                    return -1;
                    }
    
    
         //Check if a certain 2 character token just came along (e.g. BT):
         bool seen2(const char* search, char* recent)
             {
             if ( recent[oldchar-3]==search[0]
                     && recent[oldchar-2]==search[1]
                     && (recent[oldchar-1]==' ' || recent[oldchar-1]==0x0d || recent[oldchar-1]==0x0a)
                     && (recent[oldchar-4]==' ' || recent[oldchar-4]==0x0d || recent[oldchar-4]==0x0a)
             )
                 {
                 return true;
                 }
             return false;
             }

         //Convert a recent set of characters into a number if there is one.
         //Otherwise return -1:
         float ExtractNumber(const char* search, int lastcharoffset)
             {
             int i = lastcharoffset;
             while (i>0 && search[i]==' ') i--;
             while (i>0 && (isdigit(search[i]) || search[i]=='.')) i--;
             float flt=-1.0;
             char buffer[oldchar+5];
             memset(buffer,0,sizeof(buffer));
             strncpy(buffer, search+i+1, lastcharoffset-i);
             if (buffer[0] && sscanf(buffer, "%f", &flt))
                 {
                 return flt;
                 }
             return -1.0;
             }
         
         int getTempFileName(const char *path, char *tempFileName)
            {
                int len = strlen(path);
                int retVal = 0;
                char fileName[128];
               
                memset(tempFileName,0,254);
                
                strcpy(tempFileName,"c:\\temp\\pdf\\");
                                
                for(int i=0; i<len; i++)
                    {
                        if(isalnum(path[i]))
                            {
                              fileName[retVal] = path[i];
                              retVal ++;
                            }
                    }
                fileName[retVal] = '\0';
                strcat(tempFileName,fileName);
                return retVal;
            }


         //This method processes an uncompressed Adobe (text) object and extracts text.
         int ProcessOutput( FILE *fileI, char *outBuf)
             {
             const int BufLen = 1024;
             //Are we currently inside a text object?se
             bool intextobject = false;
             char output[BufLen];

             //Is the next character literal (e.g. \\ to get a \ character or \( to get ( ):
             bool nextliteral = false;

             //() Bracket nesting level. Text appears inside ()
             int rbdepth = 0;

             //Keep previous chars to get extract numbers etc.:
             char oc[oldchar];
             int j=0;
             int len;




             if(!fileI)
                 return -1;
             
             len = fread(output, 1, BufLen,fileI);
             if(feof(fileI))
                 return -1;

             for (j=0; j<oldchar; j++) oc[j]=' ';
             for (size_t i=0; i<len; i++)
                 {
                 char c = output[i];
                 if (intextobject)
                     {
                     if (rbdepth==0 && seen2("TD", oc))
                         {
                         //Positioning.
                         //See if a new line has to start or just a tab:
                         float num = ExtractNumber(oc,oldchar-5);
                         if (num>1.0)
                             {
                             strcat ( outBuf," ");

                             }
                         if (num<1.0)
                             {
                             //fputc('\t', fileO);
                             strcat(outBuf," ");

                             }
                         }
                     if (rbdepth==0 && seen2("ET", oc))
                         {
                         //End of a text object, also go to a new line.
                         strcat (outBuf," ");

                         //fputc(0x0d, fileO);

                         //fputc(0x0a, fileO);
                         }
                     else if (c=='(' && rbdepth==0 && !nextliteral)
                         {
                         //Start outputting text!
                         rbdepth=1;
                         //See if a space or tab (>1000) is called for by looking
                         //at the number in front of (
                         int num = ExtractNumber(oc,oldchar-1);
                         if (num>0)
                             {
                             if (num>1000.0)
                                 {
                                 //fputc('\t', fileO);
                                 strcat(outBuf," ");

                                 }
                             else if (num>100.0)
                                 {
                                 //fputc(' ', fileO);
                                 strcat(outBuf," ");

                                 }
                             }
                         }
                     else if (c==')' && rbdepth==1 && !nextliteral)
                         {
                         //Stop outputting text
                         rbdepth=0;
                         }
                     else if (rbdepth==1)
                         {
                         //Just a normal text character:
                         if (c=='\\' && !nextliteral)
                             {
                             //Only print out next character no matter what. Do not interpret.
                             nextliteral = true;
                             }
                         else
                             {
                             nextliteral = false;
                             if ( ((c>=' ') && (c<='~')) || ((c>=128) && (c<255)) )
                                 {
                                 if(isascii(c))
                                 {
                                 char temp[2];
                                 temp[0] = c;
                                 temp[1] = '\0';
                                 strcat(outBuf,temp);
                                 }

                                 }
                             }
                         }
                     }
                 //Store the recent characters for when we have to go back for a number:
                 for (j=0; j<oldchar-1; j++) oc[j]=oc[j+1];
                 oc[oldchar-1]=c;
                 if (!intextobject)
                     {
                     if (seen2("BT", oc))
                         {
                         //Start of a text object:
                         intextobject = true;
                         }
                     }
                 }
             if(outBuf)
                 return strlen(outBuf);
             else
                 return 0;
             }


    /* Decompress from file source to file dest until stream ends or EOF.
     inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
     allocated for processing, Z_DATA_ERROR if the deflate data is
     invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
     the version of the library linked do not match, or Z_ERRNO if there
     is an error reading or writing the files. */
    int inf(FILE *source, FILE *dest)
        {
        int ret;
        unsigned have;
        z_stream strm;
        unsigned char in[CHUNK];
        unsigned char out[CHUNK];

        /* allocate inflate state */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = 0;
        strm.next_in = Z_NULL;
        ret = inflateInit(&strm);
        if (ret != Z_OK)
        return ret;

        /* decompress until deflate stream ends or end of file */
        do
            {
            strm.avail_in = fread(in, 1, CHUNK, source);
            if (ferror(source))
                {
                (void) inflateEnd(&strm);
                return Z_ERRNO;
                }
            if (strm.avail_in == 0)
            break;
            strm.next_in = in;

            /* run inflate() on input until output buffer not full */
            do
                {
                strm.avail_out = CHUNK;
                strm.next_out = out;
                ret = inflate(&strm, Z_FINISH); //Z_NO_FLUSH);
                assert(ret != Z_STREAM_ERROR); /* state not clobbered */
                switch (ret)
                    {
                    case Z_NEED_DICT:
                    ret = Z_DATA_ERROR; /* and fall through */
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                    (void) inflateEnd(&strm);
                    return ret;
                    }
                have = CHUNK - strm.avail_out;
                if (fwrite(out, 1, have, dest) != have || ferror(dest))
                    {
                    (void) inflateEnd(&strm);
                    return Z_ERRNO;
                    }
                }
            while (strm.avail_out == 0);

            /* done when inflate() says it's done */
            }
        while (ret != Z_STREAM_END);

        /* clean up and return */
        (void) inflateEnd(&strm);
        return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
        }

    /* report a zlib or i/o error */
    void zerr(int ret)
        {
        fputs("zpipe: ", stderr);
        switch (ret)
            {
            case Z_ERRNO:
            if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
            if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
            break;
            case Z_STREAM_ERROR:
            fputs("invalid compression level\n", stderr);
            break;
            case Z_DATA_ERROR:
            fputs("invalid or incomplete deflate data\n", stderr);
            break;
            case Z_MEM_ERROR:
            fputs("out of memory\n", stderr);
            break;
            case Z_VERSION_ERROR:
            fputs("zlib version mismatch!\n", stderr);
            }
        }
    
    //Find a string in a buffer:
    int createCompressedStream(FILE *fileI, const char *path)
        {
        const int BufLen = 1024;
        char tempFile[254];
        //Read 1024 chars into memory (!):
        char* buffer = (char *)malloc(sizeof( char ) *BufLen);
        bool hasStreamData = true;
        bool hasStreamStarted = false;


        char* writePointer;
        int bytesToWrite = 0;
        FILE* pdfReaderI;

        if(feof(fileI))
            {

            free (buffer);
            return -1;
            }

        getTempFileName(path,tempFile);
        strcat(tempFile,"_compressedbin.data");

        pdfReaderI = fopen(tempFile,"w+b");

        if(fileI && pdfReaderI )
            {
            while(hasStreamData)
                {
                memset(buffer,0, BufLen);

                /*
                 * !!!!!!!!!!!!!!!!!!!!!!!!!!!   CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                 * Chances are there half of the word "stream" may get read to the buffer.
                 * if it happens, that particular two stream wont get index.
                 * Didnt implement it as of now. Because the logic requires lot of file pointer movement
				 * and character comparison.0
                 */

              
                size_t actualRead = fread(buffer, 1, BufLen,fileI);

                if(feof(fileI))
                    {
                    fclose(pdfReaderI);

                    free (buffer);
                    return -1;
                    }

                int streamStart = FindStringInBuffer (buffer, "stream", actualRead);
                int streamEnd = FindStringInBuffer (buffer, "endstream", actualRead);

                if(streamEnd> 0 && streamStart> 0 ) // To aviod finding the stream in endstream

                    {
                    if ((streamStart - streamEnd) == 3)
                        streamStart = -1;
                    }

                if ((streamStart> 0) && (hasStreamStarted == false ))
                    {
                    if((streamStart + 7)> actualRead)
                        {
                        fseek ( fileI , -(actualRead - streamStart-2) , SEEK_CUR );

                        hasStreamStarted = false;
                        hasStreamData = false;
                        continue;
                        }
                    //Skip to beginning of the data stream:
                    streamStart += 6;
                    if (buffer[streamStart]==0x0d && buffer[streamStart+1]==0x0a) streamStart+=2;
                    else if (buffer[streamStart]==0x0a) streamStart++;

                    hasStreamStarted = true;
                    writePointer = buffer + streamStart;

                    if (streamEnd> 0)
                        {
                        if (buffer[streamEnd-2]==0x0d && buffer[streamEnd-1]==0x0a) streamEnd-=2;
                        else if (buffer[streamEnd-1]==0x0a) streamEnd--;
                        bytesToWrite = streamEnd - streamStart+1;

                        fseek (fileI , -(actualRead - streamEnd - 9) , SEEK_CUR );

                        int i = ftell(fileI);
                        hasStreamStarted = false;
                        hasStreamData = false;

                        }
                    else
                    bytesToWrite = actualRead-streamStart;
                    
                    if(bytesToWrite >  0)
                        fwrite(writePointer, 1,bytesToWrite, pdfReaderI);

                    }
                else if (hasStreamStarted)
                    {
                    if (streamEnd> 0)
                        {
                        if (buffer[streamEnd-2]==0x0d && buffer[streamEnd-1]==0x0a) streamEnd-=2;
                        else if (buffer[streamEnd-1]==0x0a) streamEnd--;
                        bytesToWrite = streamEnd;
                        hasStreamStarted = false;
                        hasStreamData = false;

                        fseek (fileI , -(actualRead - streamEnd - 9) , SEEK_CUR );

                        }
                    else
                    bytesToWrite = actualRead;
                    
                    if(bytesToWrite >  0)
                        fwrite(buffer, 1,bytesToWrite, pdfReaderI);

                    }

                }
            }
        if (pdfReaderI)
            fclose(pdfReaderI); // coverty 121614

        free (buffer);
        return 1;
        }

    int extractAStream(FILE *inFile, const char *path)
        {
        char tempFile[254];
        int retf = createCompressedStream(inFile, path);

        if (retf == -1 )
            {
            return retf;
            }

        FILE* UncompressedFile;
        FILE* CompressedFile;
        
        getTempFileName(path,tempFile);
        strcat(tempFile,"_uncompressedbin.data");
        UncompressedFile = fopen(tempFile,"w+b");
        
        getTempFileName(path,tempFile);
        strcat(tempFile,"_compressedbin.data");
        CompressedFile = fopen(tempFile,"r+b");
        


        if(CompressedFile && UncompressedFile )
            int ret = inf(CompressedFile,UncompressedFile);
        else
            retf = -1;
        
        if (UncompressedFile) // coverty 
            fclose(UncompressedFile);
        if  (CompressedFile)
            fclose(CompressedFile); // coverty
        remove(tempFile);
        return retf;

        }
  

    
    int32_t convertPDFToText(const char *path)
                 {
                 int retVal =0, ret = 0;
                 FILE *fileO, *unCompressedFp;
                 int32_t nwritten = 0;
                 FILE *file;
                 char *outBuf;
                 char tempFile[254];
                 
                 file = fopen(path,"rb");
                
                 outBuf = (char *) malloc (sizeof(char)*1024);
                 _mkdir("c:\\temp");
                 _mkdir("c:\\temp\\pdf");

                // memset(start,0,space);
                 if (file == 0)
                     {

                         free(outBuf);
                         return -1;
                     }
                 
                 getTempFileName(path,tempFile);
                 strcat(tempFile,".txt");
                 
                 fileO = fopen(tempFile,"w");
                 
                 getTempFileName(path,tempFile);
                 strcat(tempFile,"_uncompressedbin.data");
                 
                 while (ret != -1)
                 {
                         ret = extractAStream(file,path);
                         unCompressedFp = fopen(tempFile,"r+b");
                         
                         if(!unCompressedFp && !fileO)
                             ret = -1;
                         
                         if(ret == -1)
                             {
                             fclose(file);
                             free(outBuf);
                             file = 0;
                             if (unCompressedFp)
                                 fclose( unCompressedFp );
                             if(fileO)
                                 fclose( fileO );
                             remove(tempFile);
                             getTempFileName(path,tempFile);
                             strcat(tempFile,"_compressedbin.data");
                             remove(tempFile);
                             return -1;
                             }
     
                         while(retVal != -1)
                             {
                             memset(outBuf,0,1024);
                             retVal = ProcessOutput(unCompressedFp,outBuf);
                             if(retVal> 0)
                                 {
                                 fwrite(outBuf, 1,retVal, fileO);
                                 fflush(fileO);
                                 }
                             }
                         retVal = 0;
                         if(unCompressedFp)
                             fclose(unCompressedFp);
                   }
                 fclose(fileO);
                 remove(tempFile);
                 getTempFileName(path,tempFile);
                 strcat(tempFile,"_compressedbin.data");
                 remove(tempFile);
                 free(outBuf);
                 return nwritten;
                 }



    bool isPdfFile(const char * path)
        {
        size_t length = strlen(path);
        const char PDF_EXTENSION[] = ".pdf";
        const char PDF_EXTENSION_UPPER[] = ".PDF";

        const char * ext = path + length - strlen(PDF_EXTENSION);

        bool rv = false;

        if ((strcmp(PDF_EXTENSION, ext) == 0) || (strcmp(PDF_EXTENSION_UPPER,
                ext) == 0))
            {
            rv = true;
            }

        return rv;
        }
    
   
    void processPdfFile(Cpix::Document * doc, const char * path)
        {
        using namespace lucene::util;
        using namespace Cpix;

        const char DEFAULT_ENCODING[] = "UTF-8";
        char tempFile[254];
        FILE *fp;
		//wchar_t excerpt [MAX_EXCERPT_LENGTH];
        wstring excerpt;
        convertPDFToText(path);

        // remove these fields before creating new values for them.
        removeStandardFields(doc);
        
        getTempFileName(path,tempFile);

        strcat(tempFile,".txt");
        
        fp = fopen(tempFile,"r");
        // determine file size. if the stream is not seekable, the size will be -1
        fseek(fp, 0, SEEK_END);
        long long size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        fclose(fp);
        
        if(size > 0)
            {
                std::auto_ptr<Field> newField(new Field(CONTENTS_FIELD,
                new FileReaderProxy(tempFile, DEFAULT_ENCODING), cpix_STORE_NO
                        | cpix_INDEX_TOKENIZED));
        
                doc->add(newField.get());
                newField.release();
                getPDFExcerpt(tempFile,excerpt);
                doc->setExcerpt(excerpt.c_str());
            }
        else
           {
               //For empty file setting the path as excerpt 
               doc->setExcerpt(doc->get(LCPIX_DOCUID_FIELD));
               
               getTempFileName(path,tempFile);
               strcat(tempFile,".txt");
               remove(tempFile);
           
           }

        doc->setAppClass(CONTENTAPPCLASS);
        doc->setMimeType(LPDFFILE_MIMETYPE);
        GenericFileProcessor(doc,path);
        }

    }


