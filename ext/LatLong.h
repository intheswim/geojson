#pragma once

#include <iostream>
#include <cstring>
#include <memory>
#include <vector>

class TLatLong;

typedef std::shared_ptr<TLatLong> TLatLongSP;

class TLatLong
{
public:
    static const int GPS_FORMAT_DMS ;
    static const int GPS_FORMAT_DECIMAL ;

	static const double METERS_2_FEET ;
	static const double METERS_2_MILES ;

	static const int DIR_UNKNOWN;
	static const int DIR_NORTH;
	static const int DIR_SOUTH;
	static const int DIR_EAST;
	static const int DIR_WEST;

	static const int DIR_NORTH_WEST;
	static const int DIR_SOUTH_WEST;
	static const int DIR_NORTH_EAST;
	static const int DIR_SOUTH_EAST;

    static int format ;

private:
    double latitude;
    double longitude;
	std::string toStringDecimal (void) const;

public:

    TLatLong (const TLatLong & ll);
    TLatLong (int lat, int lon);
    TLatLong (double lat, double lon);
	TLatLong (double x, double y, double z);
    ~TLatLong (void);
    bool operator == (const TLatLong &other) const
    {
        return(this->latitude == other.latitude && this->longitude == other.longitude);
    }

    bool isValid () const ;
    double Latitude () const { return latitude; }
    double Longitude () const { return longitude; }
    static std::string distanceString (double dist, bool in_miles);
	std::string toString (void) const ;
	std::string toDecimalSigned (bool short_format) const;
	std::string decimalString (double value, unsigned int precision) const;

    // returns distance in miles:
    static double AirDistance (const TLatLong & pt1, const TLatLong & pt2);
	static int getDirection (const TLatLong * from, const TLatLong * to);
    double distanceToSegment (const TLatLong & a, const TLatLong & b, bool & withinSegment, TLatLongSP & closest_point) const;
    double distanceToSegmentUsingTransform (const TLatLong & a, const TLatLong & b, bool & withinSegment, TLatLongSP & closest_point) const;

    static TLatLongSP crossover (const TLatLong &a, const TLatLong & b, const TLatLong & c, const TLatLong & d, bool & doCross);

    TLatLongSP convertUsingPair (const TLatLong & a, const TLatLong & b);
    static bool sameHemisphereUsingPair (const TLatLong & a, const TLatLong & b, const std::vector <TLatLongSP> & points);
};

