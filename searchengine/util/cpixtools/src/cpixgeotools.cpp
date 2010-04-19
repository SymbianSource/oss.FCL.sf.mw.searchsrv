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
#include "cpixgeotools.h"

#include <assert.h>
#include <math.h>

#include <iostream>
#include <set>


namespace
{
    template<typename CHARACTER>
    std::basic_string<CHARACTER> ToString(const Cpt::QNr & qnr)
    {
        using namespace Cpt;

        uint32_t
            mask = 0x3,
            value = qnr.getPath();
        uint16_t
            level = qnr.getLevel();
        CHARACTER
            pathStr[QNr::MAX_LEVEL + 1];
        pathStr[QNr::MAX_LEVEL] = static_cast<CHARACTER>(0);

        for (int i = 0; i <= level; ++i)
            {
                uint32_t
                    yx = value & mask;
                pathStr[QNr::MAX_LEVEL - 1 - i]
                    = static_cast<CHARACTER>('A' + yx);

                value >>= 2;
            }

        std::basic_string<CHARACTER>
            rv(pathStr + QNr::MAX_LEVEL - 1 - level);

        return rv;
    }
    
}



namespace Cpt
{

    ExifRational::ExifRational()
        : numerator_(0),
          denominator_(1)
    {
        ;
    }


    ExifRational::ExifRational(long numerator,
                               long denominator)
        : numerator_(numerator),
          denominator_(denominator)
    {
        ;
    }


    double ExifRational::getDouble() const
    {
        assert(denominator_ != 0);

        return static_cast<double>(numerator_) 
            / static_cast<double>(denominator_);
    }

    
    std::ostream & operator<<(std::ostream       & os,
                              const ExifRational & r)
    {
        os << r.numerator_ << '/' << r.denominator_;
        return os;
    }




    ExifGpsCoord::ExifGpsCoord()
    {
        ;
    }


    ExifGpsCoord::ExifGpsCoord(const ExifRational & degrees,
                               const ExifRational & minutes,
                               const ExifRational & seconds)
        : degrees_(degrees),
          minutes_(minutes),
          seconds_(seconds)
    {
        ;
    }


    double ExifGpsCoord::getDegrees() const
    {
        double
            rv = degrees_.getDouble()
            + minutes_.getDouble() / 60.0
            + seconds_.getDouble() / 3600.0;

        return rv; 
    }

    std::ostream & operator<<(std::ostream       & os,
                              const ExifGpsCoord & r)
    {
        os 
            << r.degrees_ << " "
            << r.minutes_ << " "
            << r.seconds_ << " ";

        return os;
    }


    ExifGpsLocation::ExifGpsLocation() 
   	{
   	}
   	
    
    ExifGpsLocation::ExifGpsLocation
    	(char gpsLatitudeRef,
    	 const ExifGpsCoord& gpsLatitude,
    	 char gpsLongitudeRef,
    	 const ExifGpsCoord& gpsLongitude) 
	: gpsLatitudeRef_(gpsLatitudeRef),
	  gpsLongitudeRef_(gpsLongitudeRef),
	  gpsLatitude_(gpsLatitude),	  
	  gpsLongitude_(gpsLongitude)
	{
    }

    	
    std::ostream & operator<<(std::ostream   		 &os,
                              const ExifGpsLocation  &gpsLocation)
    {
        os
            << "GPS "
            << gpsLocation.gpsLongitudeRef_ << ' '
            << gpsLocation.gpsLongitude_ << ' '
            << gpsLocation.gpsLatitudeRef_ << ' '
            << gpsLocation.gpsLatitude_;

        return os;
    }

    
    const char QNr::NORTH = 'N';
    const char QNr::SOUTH = 'S';
    const char QNr::EAST  = 'E';
    const char QNr::WEST  = 'W';


    QNr::QNr(char            gpsLatitudeRef,
             ExifGpsCoord    gpsLatitude,
             char            gpsLongitudeRef,
             ExifGpsCoord    gpsLongitude)
        : xPath_(0),
          yPath_(0),
          level_(MAX_LEVEL - 1)
    {
        double
            latitude,
            longitude;

        convertGps(gpsLatitudeRef,
                   gpsLatitude,
                   gpsLongitudeRef,
                   gpsLongitude,
                   &latitude,
                   &longitude);

        init(180.0 + longitude, 90.0 + latitude); 
    }


    QNr::QNr(double latitude,
             double longitude)
        : xPath_(0),
          yPath_(0),
          level_(MAX_LEVEL - 1)
    {
        init(180.0 + longitude, 90.0 + latitude); 
    }



    void QNr::convertGps(char            gpsLatitudeRef,
                         ExifGpsCoord    gpsLatitude,
                         char            gpsLongitudeRef,
                         ExifGpsCoord    gpsLongitude,
                         double        * latitude,
                         double        * longitude)
    {
        assert(gpsLatitudeRef == NORTH || gpsLatitudeRef == SOUTH);
        assert(gpsLongitudeRef == EAST || gpsLongitudeRef == WEST);

        // The origo is the south pole, starting at the date line
        // (imagine the Mercator projection). We have to convert those
        // longitude / latitude coordinates (with origo at
        // intersection of equator and the greenwich meridian) into
        // this other reference system.
        //
        *latitude = gpsLatitude.getDegrees();
        *longitude = gpsLongitude.getDegrees();
        
        if (gpsLatitudeRef == SOUTH)
            {
                *latitude = -*latitude;
            }

        if (gpsLongitudeRef == WEST)
            {
                *longitude = -*longitude;
            }
    }


    void QNr::init(double x,
                   double y)
    {
        // We have 2^MAX_LEVEL number of quads covering the whole
        // range of 360 degrees around the equator. So one quad has
        // 360 / 2^MAX_LEVEL degrees in X direction.
        //
        // We have also 2^MAX_LEVEL number of quads covering the whole
        // range of 180 degrees from pole to pole, so one quad has 180
        // / 2^MAX_LEVEL degrees in Y direction.
        static const double
            unitX = 360.0 / static_cast<double>(1 << MAX_LEVEL),
            unitY = 180.0 / static_cast<double>(1 << MAX_LEVEL);
        
        // The X-path values of quads from the root are actually
        // binary numbers growing from 00..0 to 11..1, and the same
        // goes for Y-path values. So we can obtain the X (and Y) path
        // values by dividing the x (y) coordinates with the width
        // (height) of a quad.
        uint32_t
            xPath = static_cast<uint32_t>(x / unitX),
            yPath = static_cast<uint32_t>(y / unitY);
        
        xPath_ = static_cast<uint16_t>(xPath);
        yPath_ = static_cast<uint16_t>(yPath);
    }




    int16_t QNr::getLevel() const
    {
        return level_;
    }


    QNr QNr::getParentQNr() const
    {
        QNr
            rv(*this);

        rv.widen();

        return rv;
    }


    std::wstring QNr::toWString() const
    {
        return ToString<wchar_t>(*this);
    }



    std::string QNr::toString() const
    {
        return ToString<char>(*this);
    }



    double QNr::getKmWidth() const
    {
        // In the NOSE sources, the method QNr::qnrLength computes the
        // length of the quad side. Except that it is a simplified
        // implementation, and works correctly for small quads around
        // the equator only. As you go closer to the poles, the quads
        // get narrower and narrower (their height remains the
        // same). That means that a search looking for results in a
        // given distance up north (or down south), say, 2 km, will
        // miss hits that are 1 km away if they are in the
        // longitudinal direction. (In Helsinki, the width of the quad
        // is half of its height, in 45 degrees latitude (~Berlin),
        // the ratio between the width and the height is the square
        // root of 2.) That's a lot of possible false negative.

        // This implementation is taking into account that quads have
        // variable width depending on how close to the pole they
        // are. If the shape of the quad is not taken into account by
        // a client using QNr class, it may still produce false
        // positives, but false positives are usually much less of a
        // problem than false negatives. A client can take the height
        // into account by using the getKmHeight().

        // First of all, the circumference c of a certain latitude
        // line l is
        //
        //  c = 2*R*Pi*cos(l)
        //
        // since the radius of the circle of latitude l is r =
        // R*cos(l), and therefore it's circumference is c = 2*r*Pi.
        //
        // The circumference of the Earth (C = 2*R*Pi) at the equator
        // is known, it is C = 40075 km. Therefore the circumference
        // of latitude line l:
        //
        //  c = C * cos(l).
        //

        static const double
            equatorCircumference = 40075.0; // around the equator
        double
            tmp = latitudeInAbsRadians();
        tmp = cos(tmp);
        double
            latitudeCircumference = equatorCircumference 
            * tmp;
        double
            quadWidth = latitudeCircumference 
            / static_cast<double>(1 << (level_ + 1));
        
        return quadWidth;
    }


    double QNr::getKmHeight() const
    {
        static const double
            circumference = 40008.0; // through the poles
        
        double
            quadHeight = circumference / (1 << (level_ + 2));

        return quadHeight;
    }


    void QNr::widen()
    {
        if (level_ > 0)
            {
                // least significant direction bit is on least
                // significant but of the path member variables, so we
                // just roll them out
                xPath_ >>= 1;
                yPath_ >>= 1;
                
                --level_;
            }
    }
    
        
    uint32_t QNr::getPath() const
    {
        uint32_t
            rv = combinePathComponents(xPath_,
                                       yPath_,
                                       level_ + 1);

        return rv;
    }


    double QNr::latitudeInAbsRadians() const
    {
        static const double
            PI = 3.14159265358979;

        // If we are level_ 0, then there are 4 quads only, 2 on both
        // hemisphere, therefore the latitude going through the
        // centerpoint of the quad (stretching from 0 to 90 degrees)
        // is 45 degrees, that is PI/4 in radians
        double
            rv = PI / 4.0;

        if (level_ != 0)
            {
                // The quads in the quadtree have the nice property
                // that coordinates of adjacents quads happens to be
                // adjacents numbers. That is, for instance, assuming
                // level_ = 3, if we take the y-path in the quad tree,
                // the y-coordinates go from 000 (just from under the
                // north pole) to 011 (just above the equator) to 100
                // (just under the equator) to 111 (just above the
                // south pole).
        
                // First we obtain the y-coordinates (they are encoded
                // in the path)
                uint32_t
                    yCoord = yPath_;

                // Now y starts from 00..0 at the north pole and grows
                // to 11..1 (level_ number of bits) to south pole. But
                // what we need is that they start from 0..0 at the
                // equator and grow to 1..1 on either arctic (level_-1
                // number of bits).
                uint32_t
                    halfway = (1 << level_);
                uint32_t
                    yTrueAbsCoord = 0;
                if (yCoord < halfway)
                    {
                        yTrueAbsCoord = halfway - 1 - yCoord;
                    }
                else
                    {
                        yTrueAbsCoord = yCoord ^ halfway;
                    }

                // The latitudes coinciding with the bigger
                // latitudinal side of a quad (closer to the equator)
                // keep increasing in PI / (2^(level_+1)) units.
                //
                // So the absolute value of the radian is
                //
                //   levelUnit * yTrueAbsCoord + levelUnit/2
                //
                // which is the same as
                //
                //   levelUnit/2 * (2*yTrueAbsCoord + 1)
                //
                // which is
                //
                //   PI * (2 * yTrueAbsCoord + 1) / (2^(level_ + 2))

                rv = PI * static_cast<double>((yTrueAbsCoord << 1) + 1);
                rv /= static_cast<double>(1 << (level_ + 2));
            }
        
        return rv;
    }
        


    QNr::QNr(uint16_t xPath,
             uint16_t yPath,
             uint16_t level)
        : xPath_(xPath),
          yPath_(yPath),
          level_(level)
    {
        if (level_ < (MAX_LEVEL - 1))
            {
                // making sure that no bits are set beyond what is
                // warranted by the level_
                uint16_t
                    mask = (1 << (level_ + 1)) - 1;
                xPath_ &= mask;
                yPath_ &= mask;
            }
    }



    void QNr::getAdjacents(std::set<QNr> & target) const
    {
        /**
         * The value of this idx is from 0..8 without 4 (4 is
         * skipped). For an index value, idx % 3 will mean the
         * x-coordinate (0, 1, 2), and idx / 3 will mean the
         * y-coordinate (0, 1, 2). The center quad is (1,1) = 4, which
         * is why it is skipped.
         */
        int
            curIdx = 0;

        // maximum value for yPath
        uint32_t
            maxYPath = (1 << (level_ + 1)) - 1;

        for (; curIdx < 9; ++curIdx)
            {
                uint16_t
                    xDirection = (curIdx % 3) - 1,
                    yDirection = (curIdx / 3) - 1;

                // Along x-direction, adjacents of a center quad
                // positioned on the boundary (next to date line
                // longitude) are nicely taken care of the mechanics
                // of integer addition (over and underflowing carry
                // bits). In normal (most) cases, x and y direction
                // adjacents are to be determined the same way.
                uint16_t
                    xPathAdj = xPath_ + xDirection,
                    yPathAdj = yPath_ + yDirection;

                // Along y-direction, adjacents of a center quad
                // positioned on the boundary (around either of the
                // poles) are exceptional. If you are standing on a
                // quad right next to the north pole, and you go north
                // to the next quad, your latitude will be exactly the
                // same and your longitude will be the "opposite".

                // if it would "flip" over to the other pole ...
                if ((yDirection == 0xffff && 0 == yPath_)
                    || (yDirection == 1 && maxYPath == yPath_))
                    {
                        yPathAdj = yPath_;
                        
                        // to switch to the "opposite" quad
                        // (longitudinally), we just flip the most
                        // significant bit on the x path component
                        xPathAdj ^= (1 << level_);
                    }

                target.insert(QNr(xPathAdj,
                                  yPathAdj,
                                  level_));
            }
    }


    uint32_t QNr::combinePathComponents(uint32_t pathX,
                                        uint32_t pathY,
                                        uint16_t numOfBits)
    {
        uint32_t
            path = 0;

        uint32_t
            mask = (1 << (numOfBits - 1));

        for (int i = 0; i < numOfBits; ++i)
            {
                path <<= 2;
                if (pathX & mask)
                    {
                        path |= 0x1;
                    }

                if (pathY & mask)
                    {
                        path |= 0x2;
                    }

                mask >>= 1;
            }

        return path;
    }



    bool operator==(const QNr & left,
                    const QNr & right)
    {
        return left.xPath_ == right.xPath_
            && left.yPath_ == right.yPath_
            && left.level_ == right.level_;
    }


    bool operator!=(const QNr & left,
                    const QNr & right)
    {
        return left.xPath_ != right.xPath_
            || left.yPath_ != right.yPath_
            || left.level_ != right.level_;
    }


    bool operator<(const QNr & left,
                   const QNr & right)
    {
        // lexicographical ordering imposed over 
        //    (level_, xPath_, yPath_) triple
        bool
            rv = left.level_ < right.level_
                  || (left.level_ == right.level_
                      && (left.xPath_ < right.xPath_
                          || (left.xPath_ == right.xPath_
                              && left.yPath_ < right.yPath_)));

        return rv;
    }


    std::ostream & operator<<(std::ostream  & os,
                              const QNr     & qnr)
    {
        using namespace std;
        using namespace Cpt;
        
        os << "QNr(x:" << hex << qnr.xPath_ << ", y:" << hex << qnr.yPath_
           << ", l:" << dec << qnr.level_
           << ", s:" << qnr.toString()
           << ")";
        
        return os;
    }


}
