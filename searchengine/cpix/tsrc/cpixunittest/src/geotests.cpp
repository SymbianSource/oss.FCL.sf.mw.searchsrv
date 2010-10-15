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
#include <stddef.h>

#include <iostream>
#include <set>
#include <sstream>

#include "cpixtools.h"

#include "itk.h"

#include "cpixidxdb.h"
#include "cpixsearch.h"

#include "config.h"
#include "testutils.h"
#include "setupsentry.h"

#include "std_log_result.h"

#define VATTUNIEMENRANTA_2_GPS L"60.154023,24.887724"



struct GpsLocation
{
    char                    gpsLatitudeRef_;
    Cpt::ExifGpsCoord       gpsLatitude_;
    char                    gpsLongitudeRef_;
    Cpt::ExifGpsCoord       gpsLongitude_;
};


const GpsLocation GPSLOC_DATA[] = {
    {
        Cpt::QNr::NORTH, 
        Cpt::ExifGpsCoord(Cpt::ExifRational(60,1),
                          Cpt::ExifRational(9,1),
                          Cpt::ExifRational(3529,100)),
        Cpt::QNr::EAST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(24,1),
                          Cpt::ExifRational(53,1),
                          Cpt::ExifRational(2508,100)),
    },

    
};


class JpgFileIdxUtil : public FileIdxUtil
{
public:
    virtual ~JpgFileIdxUtil() throw ()
    {
        ;
    }

protected:
    virtual void printHit(cpix_Document * doc,
                          Itk::TestMgr   * testMgr)
    {
        cpix_DocFieldEnum
            * dfe = cpix_Document_fields(doc);
        if (cpix_Succeeded(doc))
            {
                cpix_Field
                    field;

                printf("DOCUMENT:\n");

                while (cpix_DocFieldEnum_hasMore(dfe))
                    {
                        cpix_DocFieldEnum_next(dfe,
                                               &field);
                        const wchar_t
                            * name = cpix_Field_name(&field);

                        ITK_EXPECT(testMgr,
                                   cpix_Succeeded(&field),
                                   "Getting field name failed");

                        printf(" o %S: ",
                               name);

                        bool
                            binary = 
                            static_cast<bool>(cpix_Field_isBinary(&field));

                        /* OBS
                        printf("%S\n",
                               (binary ? 
                                L"(binary)" 
                                : cpix_Field_stringValue(&field)
                                )
                               );
                        */
                        if (binary)
                            {
                                printf("(binary)\n");
                            }
                        else
                            {
                                const wchar_t
                                    * val = cpix_Field_stringValue(&field);
                                
                                ITK_EXPECT(testMgr,
                                           cpix_Succeeded(&field),
                                           "Getting field value failed");

                                bool
                                    docuid = wcscmp(LCPIX_DOCUID_FIELD,
                                                    name) == 0;

                                if (docuid)
                                    {
                                        // docuid is the path that may
                                        // differ in its drive letter
                                        // in emulator and in target
                                        printf("!%S\n",
                                               (wcslen(val) > 0 ? val+1 : val));
                                    }
                                else
                                    {
                                        printf("%S\n",
                                               val);
                                    }
                            }
                    }

                cpix_DocFieldEnum_destroy(dfe);
            }
        else
            {
                ITK_EXPECT(testMgr,
                           false,
                           "Could not get doc enum field");
            }
    }
};




class GeoContext : public Itk::ITestContext, public Cpt::IFileVisitor
{
protected:
    //
    // protected members
    //
    JpgFileIdxUtil       * util_;
    cpix_Analyzer        * analyzer_;
    cpix_QueryParser     * queryParser_;
    cpix_Query           * query_;


    Itk::TestMgr   * testMgr_;

public:

    //
    // From ITestContext
    //
    virtual void setup() throw (Itk::PanicExc)
    {
        SetupSentry
            ss(*this);

        cpix_Result
            result;

        cpix_IdxDb_dbgScrapAll(&result);

        if (cpix_Failed(&result))
            {
                ITK_PANIC("Could not scrap all");
            }

        util_ = new JpgFileIdxUtil;
        util_->init();
        
        analyzer_ = cpix_CreateSimpleAnalyzer(&result);
        if (analyzer_ == NULL)
            {
                ITK_PANIC("Could not create analyzer");
            }
        
        queryParser_ = cpix_QueryParser_create(&result,
                                               QNR_FIELD,
                                               analyzer_);
        if (queryParser_ == NULL)
            {
                ITK_PANIC("Could not create query parser");
            }

        ss.setupComplete();
    }


    virtual void tearDown() throw()
    {
        cleanup();
    }


    virtual ~GeoContext()
    {
        cleanup();
    }


    //
    // from Cpt::IFileVisitor
    //
    virtual bool visitFile(const char * path)
    {
        bool
            goOn = true;

        const char
            * extensionChar = path + strlen(path) - 1;
        while (extensionChar >= path && *extensionChar != '.')
            {
                --extensionChar;
            }

        if (strcmp(".jpg", extensionChar) == 0)
            {
                util_->indexFile(path,
                                 analyzer_,
                                 testMgr_);
            }

        return goOn;
    }
    
    
    virtual DirVisitResult visitDirPre(const char * /*path*/)
    {
        return IFV_CONTINUE;
    }


    virtual bool visitDirPost(const char * /*path*/)
    {
        return true;
    }


    //
    // public operations
    //
    GeoContext()
        : util_(NULL),
          analyzer_(NULL),
          queryParser_(NULL),
          query_(NULL),
          testMgr_(NULL)
    {
        ;
    }


    //
    // test methods
    // 

    void testHarvestJpg(Itk::TestMgr * testMgr)
    {
        testMgr_ = testMgr;
        Cpt::traverse(JPG_TEST_CORPUS_PATH,
                      this);
        util_->flush();
       tSearchJpg(Itk::TestMgr * testMgr)
    {
        testMgr_ = testMgr;

        cpix_Hits
            * hits = search(GPSLOC_DATA,
                            sizeof(GPSLOC_DATA) / sizeof(GpsLocation),
                            NULL,
                            4.0, 
                            10);

        cpix_Hits_destroy(hits);
       
    }

private:
          delete util_;
        util_ = NULL;

        cpix_Analyzer_destroy(analyzer_);
        analyzer_ = NULL;

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;

        cpix_Query_destroy(query_);
        query_ = NULL;
    }



    /**
     * This function is the sample code for any client performing
     * location based search on anything that has QNR_FIELD defined.
     *
     * TODO RANKING of hits based on actual distance is not
     * implemented yet
     *
     * @param gpsLocs gps locations around which to search - MUST NOT
     * be NULL
     *
     * @param gpsLocsCount the number of gps locations given by
     * gpsLocs
     *
     * @param additionalTerm any additional term to include in the
     * query ("AND" query operation), if necessary, may be NULL
     *
     * @param radiusKm the radius, in kilometers, within which all
     * hits should be found (no false negatives are allowed). NOTE
     * that we may get false positives (hits that are beyond the
     * radius)
     *
     * @param maxHits the number of hits requested (within the given
     * radius), if possible
     */
    cpix_Hits * search(const GpsLocation   gpsLocs[],
                       size_t              gpsLocsCount,
                       const wchar_t     * additionalTerm,
                       double              radiusKm,
                       size_t              maxHits)
    {
		wchar_t qryStr[512];
		
		int loc = 
			swprintf( qryStr,
					  sizeof(qryStr),
					  L"$quad<");
		
		loc += 
			Cpt::wsnprintdouble( qryStr + loc,
								 sizeof(qryStr)-loc,
								 radiusKm, 
								 4 );  

		
		loc += 
			swprintf( qryStr+loc,
					  sizeof(qryStr)-loc,
					  L",%d,%S",
					  maxHits, 
					  VATTUNIEMENRANTA_2_GPS); 
    
        for (size_t i = 0; i < gpsLocsCount; ++i)
            {
                double
                    latitude,
                    longitude;

                Cpt::QNr::convertGps(gpsLocs[i].gpsLatitudeRef_,
                                     gpsLocs[i].gpsLatitude_,
                                     gpsLocs[i].gpsLongitudeRef_,
                                     gpsLocs[i].gpsLongitude_,
                                     &latitude,
                                     &longitude);
// FIXME: Get rid of Cpt::wsnprintdouble, when printf("%lf", ....) starts to work
                qryStr[loc++]= ',';
                loc +=
					Cpt::wsnprintdouble( qryStr + loc,
										 sizeof(qryStr)-loc,
										 latitude, 
										 10 );  

                qryStr[loc++]= ',';
                loc +=
					Cpt::wsnprintdouble( qryStr + loc,
										 sizeof(qryStr)-loc,
										 longitude, 
										 10 );  

/* FIXME: This DOES NOT work. 
                loc += 
					swprintf( qryStr + loc, 
							  sizeof(qryStr)-loc,
							  L",%lf,%lf", 
							  latitude,
							  longitude );
			    */ 
            }

        loc += 
			swprintf(qryStr + loc, sizeof(qryStr)-loc, L">");

        if (additionalTerm != NULL)
            {
				loc += 
					swprintf(qryStr + loc, 
							 sizeof(qryStr)-loc,
							 L"(%S)", 
							 additionalTerm);
            }
        
        cpix_Query_destroy(query_);
        query_ = NULL;

        query_ = cpix_QueryParser_parse(queryParser_,
                                        qryStr);
        ITK_ASSERT(testMgr_,
                   cpix_Succeeded(queryParser_),
                   "Failed to parse: %S\n",
                   qryStr);
        if(!cpix_Succeeded(queryParser_))
            {
                       
        cpix_Hits
            * hits = cpix_IdxDb_search(util_->idxDb(),
                                       query_);
        ITK_ASSERT(testMgr_,
                   cpix_Succeeded(util_->idxDb()),
                   "Failed to search (geo)");
        if(!cpix_Succeeded(util_->idxDb()))
            {
                
            }
        
        int32_t
            hitCount = cpix_Hits_length(hits);
        
        if (hitCount > 0)
            {
                util_->printHits(hits,
                                 testMgr_);
            }
        else
            {
                printf("  Nothing is found\n");
            }

        return hits;
    }


};




Itk::TesterBase * CreateGeoTests()
{
    using namespace Itk;

    GeoContext
        * geoContext = new GeoContext;
    ContextTester
        * contextTester = new ContextTester("geo",
                                            geoContext);

#define TEST "harvestJpg"
    contextTester->add(TEST,
                       geoContext,
                       &GeoContext::testHarvestJpg,
                       TEST);
#undef TEST

    // TODO perhaps a dump-all test case too?

#define TEST "searchJpg"
    contextTester->add(TEST,
                       geoContext,
                       &GeoContext::testSearchJpg,
                       TEST);
#undef TEST

    // TODO add more tests to suite
        
    return contextTester;
}
