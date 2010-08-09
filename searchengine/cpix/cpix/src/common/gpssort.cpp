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

#include <stdlib.h>
#include <wchar.h>
#include <math.h>

#include <algorithm>
#include <memory>
#include <vector>

#include "CLucene.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"

#include "cpixtools.h"

#include "cpixhits.h"
#include "cpixsearch.h"
#include "cpixexc.h"
#include "document.h"
#include "iidxdb.h"
#include "initparams.h"
#include "cpixutil.h"
#include "indevicecfg.h"
#include "common/gpssort.h"
#include "common/cloners.h"


namespace
{
    // constant used to indicate an invalid Gps Longitude or Latitude value
    static const float invalidGpsCoord = -360.00;
    // constant used to indicate invalid distance value
    static const float invalidDistance = 999999.99;


    class GpsDistance
    {
    private:
        // not owned
        lucene::document::Document  * doc_;

        // GPS coordinate extracted from clucene document
        float                         gpsLat_;
        float                         gpsLong_;

        // distance from a reference point
        float                         distance_;

        // for shared ptr behaviour
        mutable const GpsDistance   * next_;


    public:

        // TODO private methods
        void cleanup()
        {
            delete doc_;
            doc_ = NULL;
        }


        void enterIntoChain(const GpsDistance * chain,
                            const GpsDistance * p)
        {
            p->next_ = chain->next_;
            chain->next_ = p;
        }


        void exitFromChain()
        {
            if (next_ == this)
                {
                    cleanup();
                }
            else
                {
                    const GpsDistance
                        * next = next_,
                        * prev = next_;
                    
                    for (; prev->next_ != this; prev = prev->next_)
                        ;

                    prev->next_ = next;

                    doc_ = NULL;
                }
        }

        void copyGutsFrom(const GpsDistance & that)
        {
            doc_ = that.doc_;
            gpsLat_ = that.gpsLat_;
            gpsLong_ = that.gpsLong_;
            distance_ = that.distance_;
        }


        GpsDistance(lucene::document::Document * doc,
                    float                        gpsLat,
                    float                        gpsLong)
            : doc_(doc),
              gpsLat_(gpsLat),
              gpsLong_(gpsLong),
              distance_(invalidDistance),
              next_(NULL)
        {
            next_ = this;
        }

        
        GpsDistance(const GpsDistance & that)
            : doc_(NULL),
              gpsLat_(0.0),
              gpsLong_(0.0),
              distance_(invalidDistance),
              next_(NULL)
        {
            enterIntoChain(&that,
                           this);
            copyGutsFrom(that);
        }


        GpsDistance & operator=(const GpsDistance & that)
        {
            if (this != &that)
                {
                    exitFromChain();
                    enterIntoChain(&that,
                                   this);
                    copyGutsFrom(that);
                }

            return *this;
        }


        ~GpsDistance()
        {
            exitFromChain();
        }


        void addField(lucene::document::Field & field)
        {
            if (doc_ != NULL)
                {
                    doc_->add(field);
                }
            else
                {
                    THROW_CPIXEXC(PL_ERROR);
                }
        }


        lucene::document::Document * release()
        {
            lucene::document::Document
                * rv = doc_;

            doc_ = NULL;

            return rv;
        }

        
        float distance() const
        {
            return distance_;
        }
        

        void getDistanceStr(wchar_t  buf[],
                            size_t   bufSize) const
        {
            static const double
                metersInKilometer = 1000.0;

            if (distance_ == invalidDistance)
                {
                    wcsncpy(buf,
                            L"(unknown)",
                            bufSize);
                }
            else
                {
// FIXME: This is temporary fix to do distance formatting in non-leaking way                  
                double d = (distance() / metersInKilometer); 
                snwprintf(buf,
                          bufSize,
                          L"%d.%d",
                          (int)d,
                          (((int)(d*10))%10));


/* FIXME: snwprintf() with floating point formatting '%.1f' leaks  
					double d = distance() / metersInKilometer; 
                     snwprintf(buf,
                              bufSize,
                              L"%.1f",
                              d);*/
                }
        }


        /**
         * Sets the distance_ to the distance between this point and
         * the reference point, in meters, provided both GSP
         * coordinates are sensible, otherwise it leaves it
         * invalidDistance.
         */
        void calculateDistanceFrom(float refPointGpsLat,
                                   float refPointGpsLong)
        {
            if (refPointGpsLat != invalidGpsCoord
                && refPointGpsLong != invalidGpsCoord
                && gpsLat_ != invalidGpsCoord
                && gpsLong_ != invalidGpsCoord)
                {
                    // Earth radius in meters
                    float distance_rad = 0;
                    float pi = 3.14159265;
                    float radius = 6371000;
                    float ref_latrad = refPointGpsLat * pi /180;
                    float ref_longrad = refPointGpsLong * pi /180;
                    float dst_latrad = gpsLat_ * pi /180;
                    float dst_longrad = gpsLong_ * pi /180;

                    float t1, t2, t3, t4, t5, t6, t7, t8;

                    t1 = sin((dst_latrad - ref_latrad)/2);
                    t2 = cos(ref_latrad);
                    t3 = cos(dst_latrad);
                    t4 = sin((dst_longrad - ref_longrad)/2);
                    t5 = t1*t1 + t2*t3*t4*t4;
                    t6 = sqrt(t5); 
                    t7 = sqrt(1-t5); 
                    t8 = atan2(t6, t7);
                    distance_rad = 2*t8;

                    // return distance in meters
                    distance_ = distance_rad * radius;
                }
        }
    };


    bool operator<(const GpsDistance & left,
                   const GpsDistance & right)
    {
        return left.distance() < right.distance();
    }

    
    /**
     * For all GpsDistance structs in the vector, it calculates the
     * distance from a given reference point, sets the
     * GpsDistance.distance_ member as well as defines the
     * LDISTANCE_FIELD on the lucene document pointed by the
     * GpsDistance.doc_ member.
     *
     * @param locations the vector with the GpsDistance structs the
     * calculate the distances for. Precondition: GpsDinstance
     * instances must have their doc_, gpsLat_ and gpsLong_ members
     * set.
     *
     * @param refPointGpsLat GPS latitude of reference point
     *
     * @param refPointGpsLong GPS longitude of reference point
     */
    void calculateDistances(std::vector<GpsDistance> & locations,
                            const float                refPointGpsLat,
                            const float                refPointGpsLong)
    {
        using namespace lucene::document;
        using namespace std;

        for (vector<GpsDistance>::iterator i = locations.begin();
             i != locations.end();
             ++i)
            {
                i->calculateDistanceFrom(refPointGpsLat, 
                                         refPointGpsLong);

                wchar_t
                    dummy[32];

                i->getDistanceStr(dummy,
                                  sizeof(dummy) / sizeof(wchar_t));

                auto_ptr<Field>
                    newField(new Field(LDISTANCE_FIELD,
                                       dummy,
                                       Field::STORE_YES | Field::INDEX_NO));

                i->addField(*newField.get());

                newField.release();
            }
    }


    /**
     * A functor class that watches out for gps lat / long fields
     * during cloning (implementation detail of cloneAndGps, see below).
     */
    class GpsLatLongWatcher
    {
    private:
        float         * gpsLat_;
        float         * gpsLong_;

    public:
        GpsLatLongWatcher(float * gpsLat,
                          float * gpsLong)
            : gpsLat_(gpsLat),
              gpsLong_(gpsLong)
        {
            *gpsLat_ = invalidGpsCoord;
            *gpsLong_ = invalidGpsCoord;
        }

        
        void operator()(const wchar_t * name,
                        const wchar_t * value,
                        int             /* config */)
        {
            using namespace Cpt;

            const char
                * result = NULL;

            if (wcscmp(name, LLATITUDE_FIELD) == 0)
                {
                    result = wconvert(gpsLat_,
                                      value);
                    if (result != NULL
                        || *gpsLat_ < -90.0
                        || *gpsLat_ > 90.0)
                        {
                            *gpsLat_ = invalidGpsCoord;
                        }
                            
                }
            else if (wcscmp(name, LLONGITUDE_FIELD) == 0)
                {
                    result = wconvert(gpsLong_,
                                      value);
                    if (result != NULL
                        || *gpsLong_ < -180.0
                        || *gpsLong_ > 180.0)
                        {
                            *gpsLong_ = invalidGpsCoord;
                        }
                }
        }
    };

    
    /**
     * Clones a clucene document and at the same time retrieves its
     * LLATITUDE_FIELD and LLONGITUDE_FIELD values as floats. If the
     * fields don't exist, or cannot be converted or are invalid
     * coordinates, then a warning is omitted on stdout.
     *
     * @param doc the document to clone
     *
     * @param gpsLat put the GPS latitude here. If the corresponding
     * field did not exist, or its conversion to float failed, or it
     * was out of its sensible domain, it is set to invalidGpsCoord.
     * 
     * @param gpsLong put the GPS longitude here. If the corresponding
     * field did not exist, or its conversion to float failed, or it
     * was out of its sensible domain, it is set to invalidGpsCoord.
     */
    std::auto_ptr<lucene::document::Document>
    cloneAndGetGps(lucene::document::Document * doc,
                   float                      * gpsLat,
                   float                      * gpsLong)
    {
        return Cpix::CloneWatchingFields(doc,
                                         GpsLatLongWatcher(gpsLat,
                                                           gpsLong));
    }


    /**
     * Iterates through all hits in the document, and creates a close
     * lucene document for each as well as creates a corresponding
     * GpsDistance structure and puts it into the locations vector.
     */
    void extractLocations(cpix_Hits                * unsortedHits,
                          std::vector<GpsDistance> & locations)
    {
        using namespace Cpix;
        using namespace lucene::document;
        using namespace std;
            
        int32_t
            count = cpix_Hits_length(unsortedHits);
        locations.clear();
        locations.reserve(count);

        for (int32_t i = 0; i < count; ++i)
            {
                cpix_Document* cpixDoc = new cpix_Document;

                cpix_Hits_doc(unsortedHits,
                              i, 
                              &cpixDoc,
                              1);
                
                float
                    gpsLat,
                    gpsLong;

                auto_ptr<lucene::document::Document>
                    clone(cloneAndGetGps(&Cast2Native<cpix_Document>(cpixDoc)->native(),
                                         &gpsLat,
                                         &gpsLong));
                GpsDistance
                    gdp(clone.get(),
                        gpsLat,
                        gpsLong);

                // GpsDistance instance pointed by gdp took ownership
                // of document pointed by cloce
                clone.release();

                locations.push_back(gdp);
                
                delete cpixDoc;
            }
    }
}



namespace Cpix
{
    cpix_Hits * gpsSort(float              refPointGpsLat,
                        float              refPointGpsLong,
                        cpix_Hits        * unsortedHits,
                        cpix_Result      * result)
    {
        using namespace std;

        vector<GpsDistance>
            locations;

        extractLocations(unsortedHits,
                         locations);

        calculateDistances(locations,
                           refPointGpsLat,
                           refPointGpsLong);
        
        stable_sort(locations.begin(),
                    locations.end());

        auto_ptr<HitDocumentList>
            sortedHits(new HitDocumentList());

        vector<GpsDistance>::iterator
            i = locations.begin(),
            end = locations.end();

        for (; i != end; ++i)
            {
                sortedHits->add(i->release());
            }

        cpix_Hits
            * rv = NULL;

        CreateWrapper(sortedHits.get(),
                      result,
                      rv);

        // regardless of whether the previous call succeeded or
        // failed, the sortedHits auto_ptr loses ownership
        sortedHits.release();

        return rv;
    }
}
