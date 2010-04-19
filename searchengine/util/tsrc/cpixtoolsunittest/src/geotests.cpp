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
#include <iostream>
#include <set>


#include "itk.h"

#include "cpixgeotools.h"
#include "cpixstrtools.h"



struct GeoData {
    char              gpsLongitudeRef_;
    Cpt::ExifGpsCoord gpsLongitude_;
    char              gpsLatitudeRef_;
    Cpt::ExifGpsCoord gpsLatitude_;
};

GeoData TestData[] = {

    // Quads around origo. On level 15, they should be about of
    // 0.611495 / 0.305238 km size
    {
        Cpt::QNr::EAST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(1,100)),
        Cpt::QNr::NORTH,
        Cpt::ExifGpsCoord(Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(1,100))
    },
    {
        Cpt::QNr::EAST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(1,100)),
        Cpt::QNr::SOUTH,
        Cpt::ExifGpsCoord(Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(1,100))
    },
    {
        Cpt::QNr::WEST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(1,100)),
        Cpt::QNr::NORTH,
        Cpt::ExifGpsCoord(Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(1,100))
    },
    {
        Cpt::QNr::WEST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(1,100)),
        Cpt::QNr::SOUTH,
        Cpt::ExifGpsCoord(Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(1,100))
    },

    // Quads at mid-points of 4 great quads. On level 15, they should
    // be about of 0.4324.. / 0.305238 km size
    // 
    {
        Cpt::QNr::EAST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(90,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1)),
        Cpt::QNr::NORTH,
        Cpt::ExifGpsCoord(Cpt::ExifRational(45,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1))
    },
    {
        Cpt::QNr::EAST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(90,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1)),
        Cpt::QNr::SOUTH,
        Cpt::ExifGpsCoord(Cpt::ExifRational(45,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1))
    },
    {
        Cpt::QNr::WEST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(90,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1)),
        Cpt::QNr::NORTH,
        Cpt::ExifGpsCoord(Cpt::ExifRational(45,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1))
    },
    {
        Cpt::QNr::WEST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(90,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1)),
        Cpt::QNr::SOUTH,
        Cpt::ExifGpsCoord(Cpt::ExifRational(45,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1))
    },


    // Quads at mid-points / 60 degrees of 4 great quads. On level 15,
    // they should be about of 0.3057 / 0.305238 km size
    // 
    {
        Cpt::QNr::EAST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(90,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1)),
        Cpt::QNr::NORTH,
        Cpt::ExifGpsCoord(Cpt::ExifRational(60,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1))
    },
    {
        Cpt::QNr::EAST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(90,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1)),
        Cpt::QNr::SOUTH,
        Cpt::ExifGpsCoord(Cpt::ExifRational(60,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1))
    },
    {
        Cpt::QNr::WEST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(90,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1)),
        Cpt::QNr::NORTH,
        Cpt::ExifGpsCoord(Cpt::ExifRational(60,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1))
    },
    {
        Cpt::QNr::WEST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(90,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1)),
        Cpt::QNr::SOUTH,
        Cpt::ExifGpsCoord(Cpt::ExifRational(60,1),
                          Cpt::ExifRational(0,1),
                          Cpt::ExifRational(0,1))
    },

    // Quads around the poles. They should have a very short width
    // indeed (centimeters).
    {
        Cpt::QNr::EAST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(179,1),
                          Cpt::ExifRational(59,1),
                          Cpt::ExifRational(5999,100)),
        Cpt::QNr::NORTH,
        Cpt::ExifGpsCoord(Cpt::ExifRational(89,1),
                          Cpt::ExifRational(59,1),
                          Cpt::ExifRational(5999,100))
    },
    {
        Cpt::QNr::EAST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(179,1),
                          Cpt::ExifRational(59,1),
                          Cpt::ExifRational(5999,100)),
        Cpt::QNr::SOUTH,
        Cpt::ExifGpsCoord(Cpt::ExifRational(89,1),
                          Cpt::ExifRational(59,1),
                          Cpt::ExifRational(5999,100))
    },
    {
        Cpt::QNr::WEST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(179,1),
                          Cpt::ExifRational(59,1),
                          Cpt::ExifRational(5999,100)),
        Cpt::QNr::NORTH,
        Cpt::ExifGpsCoord(Cpt::ExifRational(89,1),
                          Cpt::ExifRational(59,1),
                          Cpt::ExifRational(5999,100))
    },
    {
        Cpt::QNr::WEST,
        Cpt::ExifGpsCoord(Cpt::ExifRational(179,1),
                          Cpt::ExifRational(59,1),
                          Cpt::ExifRational(5999,100)),
        Cpt::QNr::SOUTH,
        Cpt::ExifGpsCoord(Cpt::ExifRational(89,1),
                          Cpt::ExifRational(59,1),
                          Cpt::ExifRational(5999,100))
    },
};


std::ostream & operator<<(std::ostream   & os,
                          const GeoData  & geoData)
{
    os
        << "GPS "
        << geoData.gpsLongitudeRef_ << ' '
        << geoData.gpsLongitude_ << ' '
        << geoData.gpsLatitudeRef_ << ' '
        << geoData.gpsLatitude_;

    return os;
}


void testPlainGeo(Itk::TestMgr * testMgr)
{
    using namespace std;
    using namespace Itk;
    using namespace Cpt;

    for (int idx = 0; idx < sizeof(TestData) / sizeof(GeoData); ++idx)
        {
            GeoData
                & geoData(TestData[idx]);

            QNr
                qnr(geoData.gpsLatitudeRef_,
                    geoData.gpsLatitude_,
                    geoData.gpsLongitudeRef_,
                    geoData.gpsLongitude_);

            cout << "QNr from " << geoData << endl;

            for (int i = QNr::MAX_LEVEL-1; i >= 0; --i)
                {
                    cout << "level " << i << endl;

                    std::wstring
                        wstr = qnr.toWString();

                    auto_array<char>
                        str(new char[wstr.length() + 1]);
                    wcstombs(str.get(),
                             wstr.c_str(),
                             wstr.length() + 1);

                    cout << "qnr str: " << str.get() << endl;

                    int16_t
                        level = qnr.getLevel();

                    ITK_EXPECT(testMgr,
                               level == i,
                               "Qnr level is %d instead of %d",
                               level,
                               i);

                    double
                        kmWidth = qnr.getKmWidth(),
                        kmHeight = qnr.getKmHeight();

                    printf("qnr km width: %16.8f\n",
                           kmWidth);
                    printf("qnr km height: %16.8f\n\n",
                           kmHeight);

                    qnr = qnr.getParentQNr();
                }
        }
}


void printQNrArea(Itk::TestMgr   * ,
                  const Cpt::QNr & qnr)
{
    using namespace std;
    using namespace Cpt;

    cout << "On level " << qnr.getLevel() << endl;
    cout << "C: " << qnr << endl;

    set<QNr>
        adjacents;
    qnr.getAdjacents(adjacents);

    cout << "Adjacents: " << endl;

    set<QNr>::const_iterator
        it = adjacents.begin(),
        end = adjacents.end();

    for (; it != end; ++it)
        {
            cout << " o " << *it << endl;
        }
}


void testAdjacentQNrs(Itk::TestMgr * testMgr)
{
    using namespace std;
    using namespace Itk;
    using namespace Cpt;

    for (size_t i = 0; i < sizeof(TestData) / sizeof(GeoData); ++i)
        {
            GeoData
                & geoData = TestData[i];

            QNr
                qnr(geoData.gpsLatitudeRef_,
                    geoData.gpsLatitude_,
                    geoData.gpsLongitudeRef_,
                    geoData.gpsLongitude_);

            cout << "=============="
                 << " QNr from " << geoData << endl;

            // level 15
            printQNrArea(testMgr,
                         qnr);

            // level 14
            qnr = qnr.getParentQNr();
            printQNrArea(testMgr,
                         qnr);

            // level 2
            while (qnr.getLevel() != 2)
                {
                    qnr = qnr.getParentQNr();
                }
            printQNrArea(testMgr,
                         qnr);

            // level 1
            qnr = qnr.getParentQNr();
            printQNrArea(testMgr,
                         qnr);

            // level 0
            qnr = qnr.getParentQNr();
            printQNrArea(testMgr,
                         qnr);

            cout << endl;
        }
}

 

Itk::TesterBase * CreateGeoTests()
{
    using namespace Itk;

    SuiteTester
        * geoTests = new SuiteTester("geo");

#define TEST "plain"
    geoTests->add(TEST,
                  testPlainGeo,
                  TEST);
#undef TEST

#define TEST "adjacentQNrs"
    geoTests->add(TEST,
                  testAdjacentQNrs,
                  TEST);
#undef TEST

        
    // ... add more tests to suite
    
    return geoTests;
}


