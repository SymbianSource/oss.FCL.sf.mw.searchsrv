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

#include "CLucene.h"

#include "indevicecfg.h"

#include "cpixidxdb.h"
#include "cluceneext.h"
#include "cpixexc.h"
#include "document.h"
#include "cpixdoc.h"

#include "cpixtools.h"
#include "fileparser/fileparser.h" // removeStandardFields()

#include <iostream>

#include "spi/exif.h"




namespace
{
    const char EXTENSION[]              = ".jpg";
    const char EXTENSION_UPPERCASE[]    = ".JPG";
    const char EXTENSION_JPEG[]              = ".jpeg";
    const char EXTENSION_JPEG_UPPERCASE[]    = ".JPEG";
    const char PATH_SEPARATORS[] = "/\\";
}

namespace Cpix
{

    bool isJpegFile(const char * path)
    {
        size_t
            length = strlen(path);

        const char
            * ext_jpg = path + length - strlen(EXTENSION);
        const char
            * ext_jpeg = path + length - strlen(EXTENSION_JPEG);

        bool
            rv = false;

        if ((strcmp(EXTENSION, ext_jpg) == 0) || 
            (strcmp(EXTENSION_UPPERCASE, ext_jpg) == 0) || 
            (strcmp(EXTENSION_JPEG, ext_jpeg) == 0) || 
            (strcmp(EXTENSION_JPEG_UPPERCASE, ext_jpeg) == 0))
            {
                rv = true;
            }
        return rv;
    }
    
    bool parseExifData(const char           * exifFile,
                       Cpt::ExifGpsLocation * gpsLocation,
                       char                   dateTimeOrig[Cpix::Spi::XF_DATETIME_SIZE],
                       std::wstring& 		  comment) 
    {
        using namespace std; 
        using namespace Cpix::Spi; 
        using namespace Cpt;

        try
            {
                auto_ptr<ExifMetadata> metadata = ParseExifMetadata( exifFile );
            
                if ( metadata.get() ) {
                    std::auto_ptr<ExifEntry> entry;
                    entry = metadata->entry(EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_ORIGINAL);
                    if (entry.get()) {
                        strncpy(dateTimeOrig,
                                reinterpret_cast<const char*>(entry->data()),
                                XF_DATETIME_SIZE);
                    }
                    entry = metadata->entry(EXIF_IFD_EXIF, EXIF_TAG_USER_COMMENT);
                    if (entry.get()) {
                        comment = entry->asUserComment();
                    }
        
                    auto_ptr<ExifEntry> latitude_ref = 
                        metadata->entry(EXIF_IFD_GPS, EXIF_TAG_GPS_LATITUDE_REF);
                
                    auto_ptr<ExifEntry> latitude = 
                        metadata->entry(EXIF_IFD_GPS, EXIF_TAG_GPS_LATITUDE);
                
                    auto_ptr<ExifEntry> longitude_ref = 
                        metadata->entry(EXIF_IFD_GPS, EXIF_TAG_GPS_LONGITUDE_REF);
                
                    auto_ptr<ExifEntry> longitude = 
                        metadata->entry(EXIF_IFD_GPS, EXIF_TAG_GPS_LONGITUDE);
                        
                    if (latitude_ref.get() 
                        && latitude.get()
                        && longitude_ref.get()
                        && longitude.get()) {
                        char 
                            lat_ref,
                            lon_ref;
                        Cpt::ExifRational lat[3], lon[3];
                
                        lat_ref = latitude_ref->data()[0];
                        lon_ref = longitude_ref->data()[0];
                                
                        for (int i = 0; i < 3; i++) {
                            lat[i] = latitude->rationalAt(i);
                            lon[i] = longitude->rationalAt(i);
                        }
    
                        *gpsLocation =
                            ExifGpsLocation(lat_ref, 
                                            ExifGpsCoord(lat[0], 
                                                         lat[1], 
                                                         lat[2]), 
                                            lon_ref, 
                                            ExifGpsCoord(lon[0], 
                                                         lon[1], 
                                                         lon[2]));
                    
                        return true; 
                    }
                }
            }
        catch (...)
            {
                return false;
            }

        return false; 
    }

        
    void processJpegFile(Cpix::Document * doc,
                         const char     * path)
    {
        using namespace std;
        using namespace lucene::util;
        using namespace Cpix; 
        using namespace Cpix::Spi;

        // First: define baseappclass, excerpt and mimetype fields.
        //

        // remove these fields before creating new values for them.
        removeStandardFields(doc);

        // defining excerpt: basename and datetime of original
        string
            excerptFieldValue(path);
        string::size_type
            pos = string::npos;
        do {
            pos = excerptFieldValue.find_last_of(PATH_SEPARATORS);
            if (pos != string::npos)
                {
                    excerptFieldValue = excerptFieldValue.erase(0,
                                                                pos + 1);
                }
        } while (pos != string::npos);

        /* OBS
           size_t
           bufSize = excerptFieldValue.length() + 1;
           Cpt::auto_array<wchar_t>
           wExcerptFieldValue(new wchar_t[bufSize]);
           mbstowcs(wExcerptFieldValue.get(),
           excerptFieldValue.c_str(),
           bufSize);
        */
        Cpt::auto_array<wchar_t>
            wExcerptFieldValue(excerptFieldValue.c_str());

        doc->setExcerpt( wExcerptFieldValue.get() );
        doc->setMimeType( LJPGFILE_MIMETYPE );  
        doc->setAppClass( JPGAPPCLASS );  


        // Second: Parse Exif metadata
        //
        Cpt::ExifGpsLocation
            location;
        char
            dateTimeOrig[XF_DATETIME_SIZE] = "";
        wstring comment; 
        

        if ( !parseExifData( path,
                             &location,
                             dateTimeOrig,
                             comment ) ) {
            return;
        }
        
        // Defining gps latitude and longitude on the document as well
        // as invokding the quad-filter to produce the qnr field in a
        // next step of processing
        double
            gpsLat,
            gpsLong;
        Cpt::QNr::convertGps(location.gpsLatitudeRef_,
                             location.gpsLatitude_,
                             location.gpsLongitudeRef_,
                             location.gpsLongitude_,
                             &gpsLat,
                             &gpsLong);

        size_t bufsize = 32;
        Cpt::auto_array<wchar_t>
            tmp(new wchar_t[bufsize]);
        
        /* TODO: Restore me, when float formatting no more leaks
           snwprintf(tmp.get(),
           bufsize,
           L"%.10f",
           gpsLat);
        */
        Cpt::wsnprintdouble(tmp.get(),
                            bufsize,
                            gpsLat,
                            10);
         
        auto_ptr<Field> newField; 

        newField.reset(new Field(LLATITUDE_FIELD,
                                 tmp.get(),
                                 cpix_STORE_YES | cpix_INDEX_NO));
        doc->add(newField.get());
        newField.release();

        /* TODO: Restore me, when float formatting no more leaks
           snwprintf(tmp.get(),
           bufsize,
           L"%.10f",
           gpsLong);
        */
        Cpt::wsnprintdouble(tmp.get(),
                            bufsize,
                            gpsLong,
                            10);

        newField.reset(new Field(LLONGITUDE_FIELD,
                                 tmp.get(),
                                 cpix_STORE_YES | cpix_INDEX_NO));
        doc->add(newField.get());
        newField.release();

        if (!comment.empty()) {
            newField.reset(new Field(USERCOMMENT_FIELD,
                                     comment.c_str(),
                                     cpix_STORE_YES | cpix_INDEX_TOKENIZED));
            doc->add(newField.get());
            newField.release();
        }
        
        // defining the filter for the quadrant filter
        newField.reset(new Field(LCPIX_FILTERID_FIELD,
                                 LCPIX_QUADFILTER_FID,
                                 cpix_STORE_YES | cpix_INDEX_NO));
        doc->add(newField.get());
        newField.release();

        // defining DATETIMEORIG_FIELD
        //
        
        if (strlen(dateTimeOrig)) 
            {
                /* OBS
                bufSize = strlen(dateTimeOrig) + 1;
                Cpt::auto_array<wchar_t>
                    wDateTimeOrig(new wchar_t[bufSize]);
                mbstowcs(wDateTimeOrig.get(),
                         dateTimeOrig,
                         bufSize);
                */

                Cpt::auto_array<wchar_t>
                    wDateTimeOrig(dateTimeOrig);

                newField.reset(new Field(DATETIMEORIG_FIELD,
                                         wDateTimeOrig.get(),
                                         cpix_STORE_YES | cpix_INDEX_UNTOKENIZED));
                doc->add(newField.get());
                newField.release();
            }
    }

}
