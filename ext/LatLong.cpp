
#include <cmath>
#include <cstdio>
#include "LatLong.h"
#include "MapObject.h"
#include "MMath.h"

using namespace std;

const int TLatLong::GPS_FORMAT_DMS = 0;
const int TLatLong::GPS_FORMAT_DECIMAL = 1;

const double TLatLong::METERS_2_FEET = 3.2808399;
const double TLatLong::METERS_2_MILES = 0.000621371192;

const int TLatLong::DIR_UNKNOWN = -1;

const int TLatLong::DIR_NORTH = 0;
const int TLatLong::DIR_NORTH_EAST = 1;
const int TLatLong::DIR_EAST = 2;
const int TLatLong::DIR_SOUTH_EAST = 3;
const int TLatLong::DIR_SOUTH = 4;
const int TLatLong::DIR_SOUTH_WEST = 5;
const int TLatLong::DIR_WEST = 6;
const int TLatLong::DIR_NORTH_WEST = 7;

int TLatLong::format = GPS_FORMAT_DMS;

//////////////////////////////////////////////////////////////////////////////////////////

TLatLong::TLatLong (const TLatLong & ll)
{
	latitude = ll.latitude;
	longitude = ll.longitude;
}

//////////////////////////////////////////////////////////////////////////////////////////

TLatLong::TLatLong (int lat, int lon)
{
	latitude = 0.000001 * lat;
	longitude = 0.000001 * lon;
}

//////////////////////////////////////////////////////////////////////////////////////////

TLatLong::TLatLong (double lat, double lon)
{
	latitude = lat;
	longitude = lon;
}

//////////////////////////////////////////////////////////////////////////////////////////

TLatLong::TLatLong (double x, double y, double z)
{
	latitude = Math::toDegrees(asin(z));

	double temp = hypot (x, y);

	if (temp==0.0) // poles?
	{
		longitude = 0.0;
	}
	else
	{
		longitude = Math::toDegrees (atan2 (x, y));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////

TLatLong::~TLatLong()
{
}

//////////////////////////////////////////////////////////////////////////////////////////

bool TLatLong::isValid () const
{
	return (Math::abs (latitude) > 0.0001 && Math::abs (longitude) > 0.0001);
}

//////////////////////////////////////////////////////////////////////////////////////////

string TLatLong::distanceString (double dist, bool in_miles)
{
	char buffer[20];

	if (in_miles == false)
		dist *= (MapObject::MILE_2_METERS * 0.001);

	if (dist < 0.1)
		sprintf (buffer, "%.2f", dist);
	else
		sprintf (buffer, "%.1f", dist);

	string s (buffer);

	return s;
}

//////////////////////////////////////////////////////////////////////////////////////////

string TLatLong::toString (void) const
{
	return toStringDecimal ();
}

//////////////////////////////////////////////////////////////////////////////////////////

string TLatLong::toDecimalSigned (bool short_format) const
{
	string s1 = decimalString (latitude, short_format ? 5 : 6);
	string s2 = decimalString(longitude, short_format ? 5 : 6);

	string s = s1 + "," + s2;

	return s;
}

//////////////////////////////////////////////////////////////////////////////////////////

string TLatLong::decimalString (double value, unsigned int precision) const
{
	char buffer [20];

	sprintf (buffer, "%f", value);

	char *ptr = strchr (buffer, '.');

	if (ptr != NULL && (strlen (ptr + 1) > precision))
	{
		*(ptr + precision + 1) = 0;
	}
	string s (buffer);

	return s;
}

//////////////////////////////////////////////////////////////////////////////////////////

string TLatLong::toStringDecimal (void) const
{
	string s1 = decimalString (fabs(latitude), 6);
	s1 += (latitude < 0) ? "S" : "N";

	string s2 = decimalString(fabs(longitude), 6);
	s2 += (longitude < 0) ? "W" : "E";

	string s = s1 + "," + s2;

	return s;
}

//////////////////////////////////////////////////////////////////////////////////////////

double TLatLong::AirDistance (const TLatLong & pt1, const TLatLong & pt2)
{
	MapObject o1  (pt1);
	MapObject o2  (pt2);

	double ret = o1.GetAirDistance(o2);

	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////

int TLatLong::getDirection (const TLatLong * from, const TLatLong * to)
{
	if (from->isValid() == false || to->isValid() == false)
	{
		return DIR_UNKNOWN;
	}

	if (fabs (to->Latitude() - from->Latitude())   < 0.00001 &&
	    fabs (to->Longitude() - from->Longitude()) < 0.00001)
	{
		return DIR_UNKNOWN;
	}

	double lat2 = to->Latitude() * Math::PI / 180.0;
	double lat1 = from->Latitude() * Math::PI / 180.0;

	double lon1 = from->Longitude() * Math::PI / 180.0;
	double lon2 = to->Longitude() * Math::PI / 180.0;


	double f1 = atan2 (sin (lon2 - lon1) * cos(lat2),
	                   cos (lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(lon2-lon1));

	double r = fmod (Math::PI * 2 + f1, Math::PI * 2);

	r = r * 180 / Math::PI;

	const double RANGE = 30; // should be 22.5 to be precise

	if (r <= RANGE || r >= 360-RANGE) return DIR_NORTH;

	if (r > RANGE && r <= 90-RANGE) return DIR_NORTH_EAST;

	if (r > 90-RANGE && r <= 90+RANGE) return DIR_EAST;

	if (r > 90+RANGE && r <= 180-RANGE) return DIR_SOUTH_EAST;

	if (r > 180-RANGE && r <= 180+RANGE) return DIR_SOUTH;

	if (r > 180+RANGE && r <= 270-RANGE) return DIR_SOUTH_WEST;

	if (r > 270-RANGE && r <= 270+RANGE) return DIR_WEST;

	if (r > 270+RANGE && r < 360-RANGE) return DIR_NORTH_WEST;

	return DIR_UNKNOWN;
}

//////////////////////////////////////////////////////////////////////////////////////////

double TLatLong::distanceToSegment (const TLatLong & a, const TLatLong & b,
                                    bool & withinSegment, TLatLongSP & closest_point) const
{
	MapObject mo (*this);

	MapObject ma (a);
	MapObject mb (b);

	TLatLongSP closest;

	double ret = mo.distanceToSegment (ma, mb, withinSegment, closest);

	if (closest)
	{
		closest_point = closest;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////

double TLatLong::distanceToSegmentUsingTransform (const TLatLong & a, const TLatLong & b,
                                                  bool & withinSegment, TLatLongSP & closest_point) const
{
	MapObject mo  = MapObject::midpoint (a, b);

	TLatLong ll = mo.GetLatLong();

	MapObject ma (a);
	MapObject mb (b);

	ma.transformToCenter (ll);
	mb.transformToCenter (ll);

	MapObject mc (*this);

	mc.transformToCenter (ll);

	TLatLong lla = ma.GetLatLong();
	TLatLong llb = mb.GetLatLong();

	double x1 = ma.X();
	double z1 = ma.Z();

	// mormalize to get sin(-A) and cos(-A)
	double r = hypot (x1, z1);
	x1 = x1 / r;
	z1 = z1 / r;

	// we rotate both points lla and llb by angle (-A)

	double cx = mc.X() * x1 + mc.Z() * z1;

	// sin(a+b) = sinA*cosB+cosA*sinB

	double cz = mc.Z() * x1 - mc.X() * z1;

	double cy = mc.Y();

	double ret = fabs(asin(cz)) * MapObject::EARTH_RADIUS;

	MapObject mc2 (cx, cy, cz);

	MapObject ma2 (hypot(ma.X(),ma.Z()), ma.Y(), 0);

	TLatLong ta = ma2.GetLatLong();

	TLatLong tc = mc2.GetLatLong();

	if (fabs(tc.Longitude()) < fabs(ta.Longitude()))
	{
		withinSegment = true;
	}

	// now coordinate of the closest point:

	double n2 = hypot(cx, cy);
	MapObject mc3 (cx/n2, cy/n2, 0);

	// rotate it by angle A:

	// cos (a+b) = cosA*cosB - sinA*sinB

	double cx2 = mc3.X() * x1 - mc3.Z() * z1;

	// sin(a+b) = sinA*cosB+cosA*sinB

	double cz2 = mc3.Z() * x1 + mc3.X() * z1;

	double cy2 = mc3.Y();

	MapObject mo3 (cx2, cy2, cz2);

	mo3.inverseTransform (ll);

	closest_point = mo3.GetLatLongPtr();

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////
// Do segments ab and bc cross? If so return point of crossing
///////////////////////////////////////////////////////////////////////////////////
TLatLongSP TLatLong::crossover (const TLatLong &a, const TLatLong & b,
					const TLatLong & c, const TLatLong & d, bool & doCross)
{
	MapObject ma (a);
	MapObject mb (b);

	MapObject ab  = MapObject::crossProduct (ma, mb);

	MapObject mc (c);
	MapObject md (d);

	MapObject cd = MapObject::crossProduct (mc, md);

	MapObject pt = MapObject::crossProduct (ab, cd);

	double abAngle = ma.GetAngle (mb);
	double bcAngle = mc.GetAngle (md);

	double ap = pt.GetAngle (ma);
	double bp = pt.GetAngle (mb);

	double cp = pt.GetAngle (mc);
	double dp = pt.GetAngle (md);

	doCross = false;

	if ((ap < abAngle && bp < abAngle) && (cp < bcAngle && dp < bcAngle))
	{
		doCross = true;
	}
	else
	{
		pt.invert();

		ap = pt.GetAngle (ma);
		bp = pt.GetAngle (mb);

		cp = pt.GetAngle (mc);
		dp = pt.GetAngle (md);

		if ((ap < abAngle && bp < abAngle) && (cp < bcAngle && dp < bcAngle))
		{
			doCross = true;
		}
	}

	TLatLongSP ret = pt.GetLatLongPtr();

	return (ret);
}

///////////////////////////////////////////////////////////////////////////////////
// function detects if segment between a and b is the "edge" of a polygon where
// vertices are points.
// 1. we put both midpoint of a and b on "equator".
// then we apply the same transformation to points vector.
// Then we use scalar vector product to determine if they are on the same side of
// line from a to b.
// If not it returns false.
//
// Note: there's more efficient method just below using cross product.
// There's also a faster function in GeoUtils.cpp which does not require
// initialization of the same MapObjects repeatedly.
///////////////////////////////////////////////////////////////////////////////////
// #define ORIGINAL_METHOD

#ifdef ORIGINAL_METHOD
bool TLatLong::sameHemisphereUsingPair (const TLatLong & a, const TLatLong & b, const vector <TLatLongSP> & points)
{
	if (a == b)
	{
		return false;
	}

	MapObject mo  = MapObject::midpoint (a, b);

	TLatLong ll = mo.GetLatLong();

	MapObject ma (a);
	MapObject mb (b);

	ma.transformToCenter (ll);
	mb.transformToCenter (ll);

	int first_sign = 0;

	for (auto & temp : points)
	{
		if (temp.get() == &a || temp.get() == &b)
		{
			continue;
		}

		MapObject mpt (*temp.get());

		mpt.transformToCenter (ll);

		double x1 = ma.X();
		double z1 = ma.Z();

		// mormalize to get sin(-A) and cos(-A)
		/* double r = hypot (x1, z1);
		x1 = x1 / r;
		z1 = z1 / r;  */

		double value = mpt.Z() * x1 - mpt.X() * z1;

		int sign= 0;

		if (fabs(value) == 0)
		{
			continue;
		}

		if (value < 0)
		{
			sign = 1;
		}
		else if (value > 0)
		{
			sign = -1;
		}

		if (first_sign==0)
		{
			first_sign = sign;
		}
		else
		{
			if (first_sign != sign)
			{
				return false;
			}
		}

	}

	return true;

}
#else // ORIGINAL_METHOD
bool TLatLong::sameHemisphereUsingPair (const TLatLong & a, const TLatLong & b, const vector <TLatLongSP> & points)
{
	if (a == b)
	{
		return false;
	}

	MapObject ma (a);
	MapObject mb (b);

	MapObject ab  = MapObject::crossProduct (ma, mb);

	int first_sign = 0;

	for (auto & temp : points)
	{
		if (temp.get() == &a || temp.get() == &b)
		{
			continue;
		}

		MapObject mpt (*temp.get());

		double value = mpt.X() * ab.X() + mpt.Y() * ab.Y() + mpt.Z() * ab.Z();

		int sign= 0;

		if (fabs(value) == 0)
		{
			continue;
		}

		if (value < 0)
		{
			sign = 1;
		}
		else if (value > 0)
		{
			sign = -1;
		}

		if (first_sign==0)
		{
			first_sign = sign;
		}
		else
		{
			if (first_sign != sign)
			{
				return false;
			}
		}
	}

	return true;
}

#endif

///////////////////////////////////////////////////////////////////////////////////
//  This function applies transformation in which
// two coordinates a and b imagined to be on the equator, with midpoint of (a and b)
// being at lat=lon=0.
///////////////////////////////////////////////////////////////////////////////////
TLatLongSP TLatLong::convertUsingPair (const TLatLong & a, const TLatLong & b)
{
	MapObject mo = MapObject::midpoint (a, b);

	TLatLongSP llsp = mo.GetLatLongPtr();

	MapObject ma (a);
	MapObject mb (b);

	ma.transformToCenter (*llsp);
	mb.transformToCenter (*llsp);

	// at this point midpoint has coordinates (0,0)
	// while ma and mb have coordinates (lat, lon) and (-lat,-lon);

	MapObject mpt (*this);

	mpt.transformToCenter (*llsp);

	// next we find transformation in which both ma and mb will be on equator
	// and apply this transformation to "this" ( mpt ).

	double x1 = ma.X();
	double z1 = ma.Z();

	// mormalize to get sin(-A) and cos(-A)
	double r = hypot (x1, z1);
	x1 = x1 / r;
	z1 = z1 / r;

	double px = mpt.X() * x1 + mpt.Z() * z1;

	double pz = mpt.Z() * x1 - mpt.X() * z1;

	double py = mpt.Y();

	// this is for debugging only:

	/* double norm = px*px + py*py + pz*pz;

	printf ("norm: %lf, %lf %lf %lf\n", norm, px, py, pz);  */

	MapObject mc2 (px, py, pz);

	TLatLongSP ret = mc2.GetLatLongPtr();

	return (ret);
}
