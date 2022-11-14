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

#include "MapObject.h"
#include "MMath.h"
#include "LatLong.h"

using namespace std;

const double MapObject::EARTH_RADIUS = 3958.76; // Authalic/Volumetric radius in miles
const double MapObject::MILE_2_METERS = 1609.344;

///////////////////////////////////////////////////////////////////////////////////

MapObject::MapObject (const TLatLong & latlong)
{
	double tmp_lat = Math::toRadians(latlong.Latitude());
	double tmp_long = Math::toRadians(latlong.Longitude());

	z = Math::fsin (tmp_lat);
	double temp = Math::fcos (tmp_lat);

	x = Math::fsin (tmp_long) * temp;
	y = Math::fcos (tmp_long) * temp;

}

///////////////////////////////////////////////////////////////////////////////////

MapObject::MapObject (double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

///////////////////////////////////////////////////////////////////////////////////

MapObject::~MapObject(void)
{

}

///////////////////////////////////////////////////////////////////////////////////

double MapObject::GetAngleCos (const MapObject & obj) const
{
	double ret = (x * obj.x) + (y * obj.y) + (z * obj.z);
	if (ret > 1)
		ret = 1;
	if (ret < -1)
		ret = -1;

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////

double MapObject::GetAngle (const MapObject & obj) const
{
	double ret = GetAngleCos (obj);

	ret = Math::arccos (ret);

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////

TLatLong MapObject::GetLatLong (void) const
{
	return TLatLong (x, y, z);
}

///////////////////////////////////////////////////////////////////////////////////

TLatLongSP MapObject::GetLatLongPtr (void) const
{
	return TLatLongSP (new TLatLong (x, y, z));
}

///////////////////////////////////////////////////////////////////////////////////

string MapObject::latLongString (void) const
{
    TLatLong temp (x, y, z);

    return temp.toString();
}

///////////////////////////////////////////////////////////////////////////////////
// returns distance in in_miles.
///////////////////////////////////////////////////////////////////////////////////
double MapObject::GetAirDistance (const MapObject & obj) const
{
	return GetAngle (obj) * EARTH_RADIUS;
}

///////////////////////////////////////////////////////////////////////////////////

MapObject MapObject::crossProduct (const MapObject &a, const MapObject &b)
{
	double x1 = a.y * b.z - a.z * b.y;
	double y1 = a.z * b.x - a.x * b.z;
	double z1 = a.x * b.y - a.y * b.x;

	double n = sqrt (x1*x1 + y1*y1 + z1*z1);

	if (n == 0)
	{
		fprintf (stderr, "%lf %lf %lf\n", a.x, a.y, a.z);
		throw std::runtime_error ("Cannot calculate cross product");
	}

	return MapObject (x1/n, y1/n, z1/n);
}

///////////////////////////////////////////////////////////////////////////////////

double MapObject::distanceToSegment (const MapObject & a, const MapObject & b,
                bool & withinSegment, TLatLongSP & closest)
{
	MapObject G = MapObject::crossProduct (a, b);

	MapObject F = MapObject::crossProduct (*this, G);

	MapObject T = MapObject::crossProduct (G, F);

	double ret = this->GetAirDistance (T);

	TLatLongSP my_closest = T.GetLatLongPtr();

	double angle_ta = T.GetAngleCos (a);
	double angle_tb = T.GetAngleCos (b);
	double angle_ab = a.GetAngleCos (b);

	withinSegment = false;

	if (angle_ab < angle_ta && angle_ab < angle_tb)
	{
		withinSegment = true;
		closest = my_closest;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////

MapObject MapObject::midpoint (const MapObject &a, const MapObject &b)
{
	double x = a.x + b.x;
	double y = a.y + b.y;
	double z = a.z + b.z;

	double n = sqrt (x*x + y*y + z*z);

    MapObject ret (x/n, y/n, z/n);

    return ret;
}

///////////////////////////////////////////////////////////////////////////////////

MapObject MapObject::midpointLL (const TLatLong &a, const TLatLong &b)
{
	return midpoint (MapObject (a), MapObject(b));
}

///////////////////////////////////////////////////////////////////////////////////

MapObject MapObject::equidistantPoint (const MapObject & a, const MapObject & b, const MapObject & c)
{
	if (a == b || b == c || a == c)
		throw std::runtime_error("cannot calculate equidistant point");

    MapObject ab_mid = midpoint (a, b);
	MapObject ab_cross = crossProduct (a, b);

	MapObject plane1 = crossProduct (ab_mid, ab_cross);

	MapObject ac_mid = midpoint (a, c);
    MapObject ac_cross = crossProduct (a, c);

	MapObject plane2 = crossProduct (ac_mid, ac_cross);

	MapObject eq_point = crossProduct (plane1, plane2);

	if (eq_point.GetAngleCos(a) < 0)
		eq_point.invert();

	return eq_point;
}

///////////////////////////////////////////////////////////////////////////////////

void MapObject::transform(double gamma, double theta)
{
	// x is sin of new angle
	// sin(a+b) = sinA*cosB+cosA*sinB
	// sinA = x, cosA=y
	double x1 = x * cos(gamma) + y * sin(gamma);
	// y is cos of new angle
	// cos (a+b) = cosA*cosB - sinA*sinB
	double y1 = y * cos(gamma) - x * sin(gamma);

	// step 2. tilting.
	// x remains the same
	// y and z change

	double z1 = z * cos(theta) + y1 * sin(theta);
	double y2 = y1 * cos(theta) - z * sin(theta);

	this->x = x1;
	this->y = y2;
	this->z = z1;
}

///////////////////////////////////////////////////////////////////////////////////

void MapObject::inverse_transform(double gamma, double theta)
{
	double z1 = z * cos(theta) + y * sin(theta);
	double y1 = y * cos(theta) - z * sin(theta);

	double x1 = x * cos(gamma) + y1 * sin(gamma);
	double y2 = y1 * cos(gamma) - x * sin(gamma);

	this->x = x1;
	this->y = y2;
	this->z = z1;
}

///////////////////////////////////////////////////////////////////////////////////

void MapObject::transformToCenter(const TLatLong & using_latlong)
{
	double gamma = -Math::toRadians(using_latlong.Longitude());
	double theta = -Math::toRadians (using_latlong.Latitude());
	transform (gamma, theta);
}

///////////////////////////////////////////////////////////////////////////////////

void MapObject::inverseTransform (const TLatLong & using_latlong)
{
	double gamma = Math::toRadians(using_latlong.Longitude());
	double theta = Math::toRadians (using_latlong.Latitude());

	inverse_transform (gamma, theta);
}

///////////////////////////////////////////////////////////////////////////////////

void MapObject::invert(void)
{
	this->x = -this->x;
	this->y = -this->y;
	this->z = -this->z;
}

///////////////////////////////////////////////////////////////////////////////////
// create regular polygon with cenrer in pt. radius is Distance from center to all
// resulting vertices of the polygon.
///////////////////////////////////////////////////////////////////////////////////
void MapObject::getNPointsAround (const TLatLong & pt, double radiusMiles, const int vertCount,
                                  std::vector<TLatLongSP> & output)
{
	double alpha = radiusMiles / MapObject::EARTH_RADIUS;
	double y = cos (alpha);

	double coef = sqrt(1 - y * y);

	for (int i=0; i < vertCount; i++)
	{
		double beta = i * 2 * M_PI / vertCount;
		double x = sin (beta);
		double z = cos (beta);

		MapObject temp (x*coef, y, z*coef);

		temp.inverseTransform (pt);
		TLatLongSP temp_ll = temp.GetLatLongPtr ();

		output.push_back (temp_ll);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// pass distance in miles for distance to the chord and N (integer) is number of vertices around center
// then result is used in getNPointsAround (MapObject) to create regular polygon.
// in case of N=3 (triangle) result is about twice as big as input radius;
// in case of N=4 (square) result is about 1.4 times bigger, and so on.
///////////////////////////////////////////////////////////////////////////////////////
double MapObject::getTrueRadius (const double radiusMiles, const int vertCount)
{
	// normal one is (x=0, y=cos(R/EARTH_RADIUS), z=-sin(R/EARTH_RADIUS))
	// normal two is (x=sin(PI*2/N) * cos(R/EARTH_RADIUS), y=cos(R/EARTH_RADIUS) * cos(PI*2/N) , z=-sin(R/EARTH_RADIUS)

	if (vertCount < 3)
	{
		throw std::invalid_argument("Number of vertices is too small.");
	}

    double angle = radiusMiles/MapObject::EARTH_RADIUS;

	MapObject a (0.0, cos( angle ), -sin( angle ));

	MapObject b (sin (M_PI * 2 / vertCount) * cos( angle ),
	             cos (M_PI * 2 / vertCount) * cos( angle ),
	             -sin ( angle ));

	MapObject ab = MapObject::crossProduct (a, b);

	double z = fabs (ab.Z());

	return acos(z) * MapObject::EARTH_RADIUS;

}
