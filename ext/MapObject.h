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

#include <iostream>
#include <cstring>
#include <memory>
#include <vector>
#include <stdio.h>
#include <cmath>

#include "LatLong.h"

#define USE_EUCLIDIAN_SMALL_SCALE

class TLatLong;
class MapObject;

typedef std::shared_ptr<MapObject> MapObjectSP;

class MapObject
{
private:
	double x, y, z;

public:
	static const double EARTH_RADIUS ;
  	static const double MILE_2_METERS ;

  	MapObject (double x, double y, double z);
   	MapObject (const TLatLong & latlong);
	MapObject (const MapObject & other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
	}
	~MapObject(void);

	bool operator == (const MapObject &other) const
    {
        return(this->x == other.x && this->y == other.y && this->z == other.z);
    }

    static MapObject crossProduct (const MapObject &a, const MapObject &b);
    void transform (double gamma, double theta);
    void inverse_transform (double gamma, double theta);
	static MapObject midpoint (const MapObject &a, const MapObject &b);
    static MapObject midpointLL (const TLatLong &a, const TLatLong &b);
	static MapObject equidistantPoint (const MapObject & a, const MapObject & b, const MapObject & c);

	TLatLong GetLatLong (void) const;
	TLatLongSP GetLatLongPtr (void) const;
	std::string latLongString (void) const;

	// returns angle in radians
	double GetAngle (const MapObject & obj) const;

	// returns distance in miles:
	double GetAirDistance (const MapObject & obj) const;

	// returns cosine of angle:
	double GetAngleCos (const MapObject & obj) const;

    double distanceToSegment (const MapObject &a, const MapObject &b, bool & withinSegment, TLatLongSP & closest);

    void transformToCenter(const TLatLong & using_latlong);
    void inverseTransform (const TLatLong & using_latlong);
    void invert(); // opposite position on earth

    double X () const { return x; }
    double Y () const { return y; }
    double Z () const { return z; }

    static void getNPointsAround (const TLatLong & pt, double radiusMiles, const int vertCount,
                           std::vector<TLatLongSP> & output);

    static double getTrueRadius (const double radiusMiles, const int vertCount);
};
