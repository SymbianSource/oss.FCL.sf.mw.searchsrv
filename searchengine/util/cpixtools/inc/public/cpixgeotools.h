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
#ifndef CPIXTOOLS_CPIXGEOTOOLS_H_
#define CPIXTOOLS_CPIXGEOTOOLS_H_

#include <stdint.h>
#include <wchar.h>

#include <iosfwd>
#include <set>
#include <string>

namespace Cpt
{
    typedef uint32_t PackedQNr;


    /**
     * Corresponds to the EXIF type "RATIONAL", cf. EXIF 2.2 standard,
     * chapter "4.6.2 IFD Structure". POV.
     */
    struct ExifRational
    {
        long numerator_;
        long denominator_;

        ExifRational();
        ExifRational(long numerator,
                     long denominator);

        double getDouble() const;
    };

    
    std::ostream & operator<<(std::ostream       & os,
                              const ExifRational & r);


    /**
     * Corresponds to how EXIF represents GPS coordinates: 3 RATIONALs
     * for degrees, minutes and seconds, cf. EXIF 2.2 standard,
     * chapter "4.6.6 GPS Attribute Information". POV.
     */
    struct ExifGpsCoord
    {
        ExifRational   degrees_;
        ExifRational   minutes_;
        ExifRational   seconds_;

        ExifGpsCoord();
        ExifGpsCoord(const ExifRational & degrees,
                     const ExifRational & minutes,
                     const ExifRational & seconds);


        double getDegrees() const;
    };


    std::ostream & operator<<(std::ostream       & os,
                              const ExifGpsCoord & r);

    
    struct ExifGpsLocation
    {
    	char         gpsLatitudeRef_;
    	char         gpsLongitudeRef_;
    	ExifGpsCoord gpsLatitude_;
    	ExifGpsCoord gpsLongitude_;

    	ExifGpsLocation();
    	ExifGpsLocation(char gpsLatitudeRef,
                        const ExifGpsCoord& gpsLatitude,
                        char gpsLongitudeRef,
                        const ExifGpsCoord& gpsLongitude);
    };
    
    std::ostream & operator<<(std::ostream   	     &os,
                              const ExifGpsLocation  &gpsLocation);
    
    /**
     * This class corresponds to Java class
     * common/src/com/nokia/nose/geo/QNR.java. It represents a quad
     * region in a quad tree. Probably stands for "Quad NumbeR". The
     * grid cells in the Mercator projection are organized into quad
     * trees. This organization provices a way to obtain the
     * identifier of a parent quad region as well as quickly compute
     * adjacents quad regions. GPS-to-quad and vice versa conversion
     * is also supported.
     */
    class QNr
    {
    private:
        //
        // private members
        //

        /**
         * These two member variables correspond to the int array
         * "_dir" in the original QNr Java implementation. The most
         * significant direction bits (x and y direction on level 0)
         * are stored on the most significant bits of xPath_ and
         * yPath_. So direction bit on level i (i = 0, ..., level_)
         * are stored on the bits (level_-i) of xPath_ and yPath_.
         *
         * The root of the quadtree represents the whole area
         * covered. Obviously, it does not need to be identified, so
         * it does not have any direction bits in the path.
         *
         * Each quad (including the root one) has four subquads:
         *
         *  ^  +---------+---------+
         *  |  |  (0,1)  |  (1,1)  |
         *  |  |  C      |  D      |
         *     |         |         |
         *  Y  +---------+---------+
         *     |  (0,0)  |  (1,0)  |
         *  |  |  A      |  B      |
         *  |  |         |         |
         *  |  +---------+---------+
         *
         *     --------- X -------->
         *
         */
        uint16_t       xPath_;
        uint16_t       yPath_;

        /**
         * The level of the quad tree this quad is at. Minimum is
         * zero: quads right under the root (all area). Level_ = 0
         * means that in path_, the 0th and 1st bit (mask 0x3) mean
         * anything and the whole are is subdivided into 4
         * quads. Level_ = 1 means that in that path bits in positions
         * of 0...3 mean anything (mask 0xf), and the whole area is
         * subdivided into 16 sub-subquads.
         */
        uint16_t       level_;

    public:
        //
        // public operations
        //
        /**
         * The number of levels in the quad tree, which is the same as
         * the number of (xi,yi) pairs making up a path_. Level
         * identifiers run from 0 to MAX_LEVEL-1.
         */
        enum { MAX_LEVEL = 16 };
        

        
        /**
         * Valid characters for constructor
         */
        static const char NORTH;
        static const char SOUTH;
        static const char EAST;
        static const char WEST;


        /**
         * Constructs a QNr from GPS data, cf. EXIF 2.2 standard,
         * chapter "4.6.6 GPS Attribute Information".
         *
         * @param gpsLatitudeRef indicates whether latitude is north
         * or south latitude. Possible values are 'N' and 'S'.
         *
         * @param gpsLatitude GPS latitude: degrees, minutes, seconds.
         *
         * @param gpsLongitudeRef indicates whether longitude is east
         * or west. Possible values are 'E', 'W'.
         *
         * @param gpsLongitude GPS longitude: degrees, minutes,
         * seconds.
         */
        QNr(char            gpsLatitudeRef,
            ExifGpsCoord    gpsLatitude,
            char            gpsLongitudeRef,
            ExifGpsCoord    gpsLongitude);


        /**
         * Constructs a QNr from GPS coordinates.
         *
         * @param latitude is the latitude, 0.0 is the greenwhich
         * line, positive coordinates to the eastern hemisphere,
         * negatives to the western one.
         *
         * @param longitude is the longitude, 0.0 is the equator,
         * positive coordinates to the northern hemisphere, negatives
         * to the southern one.
         */
        QNr(double latitude,
            double longitude);


        /**
         * Converts a GPS coordinate specified by the first 4
         * arguments into another format of the same GPS coordinate in
         * the last two arguments.
         *
         * If you give the first 4 arguments to this function and then
         * feed the resulting two doubles into the second QNr
         * constructor, you will get the same result as if you have
         * used the first QNr function with that 4 argumemnts.
         */
        static void convertGps(char            gpsLatitudeRef,
                               ExifGpsCoord    gpsLatitude,
                               char            gpsLongitudeRef,
                               ExifGpsCoord    gpsLongitude,
                               double        * latitude,
                               double        * longitude);

        /**
         * @returns the level of the quad tree this quad is
         * at.
         */
        int16_t getLevel() const;


        /**
         * @returns the quad that has the same path but has one level
         * less, which effectively means the parent quad of this
         * quad. If the level of this quad is 0 (already in the root
         * quad), then it returns a value equivalent to this.
         */
        QNr getParentQNr() const;

        
        /**
         * @return wstring representation of this instance. There is a
         * 1-to-1 mapping between quads (on any level) and their
         * string representations.
         */
        std::wstring toWString() const;


        /**
         * @return string representation of this instance. There is a
         * 1-to-1 mapping between quads (on any level) and their
         * string representations.
         */
        std::string toString() const;


        /**
         * @returns the approximate width of this quad in kilometers.
         */
        double getKmWidth() const;


        /**
         * @returns the approximate height of this quad in kilometers.
         */
        double getKmHeight() const;


        /**
         * Gets the adjacents of this QNr.
         *
         *
         * +----+----+----+
         * |   0|   1|   2|
         * |    |    |    |
         * +----+====+----+
         * |   3#   4#   5|
         * |    #    #    |
         * +----+====+----+
         * |   6|   7|   8|
         * |    |    |    |
         * +----+----+----+
         *
         * where quad #4 is the one around wich we need the
         * adjacents. Most of the time the number of adjacents is 8,
         * but in extreme cases (on level 0 and 1, around the poles)
         * it can be less. The center quad is also added.
         *
         * @param target the set to put the adjacents of this quad to
         * - it is NOT cleared before inserting new quads!
         */
        void getAdjacents(std::set<QNr> & target) const;


        /**
         * @returns the path of this QNr, it's specially encoding the
         * direction of the MAX_LEVEL-level quad tree. Mostly for
         * debugging / testing purposes.
         */
        uint32_t getPath() const;

    private:
        //
        // private methods
        //
        /**
         * Widens the quad to one level up. It's new value is
         * essentially what is the parent quad of the old value. If on
         * top level (0), no further widening is done.
         */
        void widen();

        
        /**
         * @returns the absolute value of the latitude in
         * radians. Zero means the equator, PI/2 means the arctic
         * (either of them). By latitude we mean that latitude that
         * goes through the center point of this quad (not the one
         * that coincides with its northen or souther borders).
         */
        double latitudeInAbsRadians() const;


        /**
         * Direct construction.
         */
        QNr(uint16_t xPath,
            uint16_t yPath,
            uint16_t level);


        /**
         * Initializes this instance. X and y are in the reference
         * system where the origo is on the south-pole / dateline
         * corner of the mercator projection.  That is, the greenwhich
         * / equator intersection is at (180,90) coordinates, in the
         * dead center of the whole grid (and not in the origo).
         */
        void init(double x, double y);


        /**
         * Combines to path components into one, the exact same format
         * as path_ member, up to maxLevel bits.
         * 
         * @param numOfBits
         */
        static uint32_t QNr::combinePathComponents(uint32_t pathX,
                                                   uint32_t pathY,
                                                   uint16_t numOfBits);


        friend bool operator==(const QNr & left,
                               const QNr & right);
        friend bool operator!=(const QNr & left,
                               const QNr & right);
        friend bool operator<(const QNr & left,
                              const QNr & right);

        friend std::ostream & operator<<(std::ostream  & os,
                                         const QNr     & qnr);
    };



    /**
     * The comparision operators have natural semantics: both path and
     * the level are the same.
     */
    bool operator==(const QNr & left,
                    const QNr & right);

    bool operator!=(const QNr & left,
                    const QNr & right);


    /**
     * This operator is only defined to be able to put QNr-s in
     * std::sets. The ordering is consistent, but it does not
     * correspond to any natural notion of ordering one could care to
     * dream of.
     */
    bool operator<(const QNr & left,
                   const QNr & right);


    /**
     * Streaming operator, mostly for testing purposes.
     */
    std::ostream & operator<<(std::ostream  & os,
                              const QNr     & qnr);

}

#endif
