/* geojson, Copyright (c) 2013-2020 Yuriy Yakimenko
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#pragma once

#include <vector>
#include "LatLong.h"
#include "MapObject.h"

class MapObjectEx : public MapObject
{
public:
    int mark;
	MapObjectEx (double lat, double lon) : MapObject (TLatLong (lat,lon)), mark(0) { }
};

class GeoUtils
{
private:
    static bool getConvexHull (std::vector<TLatLongSP> latlongs, std::vector<TLatLongSP> & border_latlongs,
        const int batchCount);

    static bool sameHemisphereUsingIndexedPair (const int indexA, const int indexB,
        const std::vector <MapObject> & points);

    static MapObject smallestCircle (std::vector <MapObjectEx> & inputP, double & outradius);


public:
    static MapObject getEquidistantPoint (const MapObject & a, const MapObject & b, const MapObject & c);

    static bool getConvexHull (std::vector<std::pair<double,double> > points,
                 std::vector<std::pair<double,double> > & output,
                 const double radiusMiles, const int vertCount);

    // creates regular polygon centered at coordinate with COUNT vertices.
    static bool getPointsAroundCoordinate (const TLatLong & coord,
                    const double radiusMiles, const int vertCount,
                    std::vector <std::pair<double,double> > & output);

    static TLatLong mincircle (std::vector<std::pair<double,double> > points, double & outRadius);
};
