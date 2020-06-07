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

#include "LatLong.h"
#include "GeoUtils.h"

#include <algorithm>

using namespace std;

bool GeoUtils::sameHemisphereUsingIndexedPair (const int indexA, const int indexB,
								const std::vector <MapObject> & points)
{
	const MapObject & ma = points[indexA];
	const MapObject & mb = points[indexB];

	if (ma == mb)
	{
		return false;
	}

	MapObject ab  = MapObject::crossProduct (ma, mb);

	int first_sign = 0;

	int index = -1;

	for (auto & pt : points)
	{
		index++;

		if (index == indexA || index == indexB)
		{
			continue;
		}

		double value = pt.X() * ab.X() + pt.Y() * ab.Y() + pt.Z() * ab.Z();

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

//////////////////////////////////////////////////////////////////////////////////////////
// returns point which is in the middle of three input Coordinates.
/////////////////////////////////////////////////////////////////////////////////////////

MapObject GeoUtils::getEquidistantPoint (const MapObject & a, const MapObject & b, const MapObject & c)
{
    MapObject obj = MapObject::equidistantPoint (a, b, c);
    return obj;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Jarvis march algorithm with some optimization when batchCount > 0
// Description:
// We call this method twice. First to generate hull with input points and batchCount set to -1.
// Then its result is used to generate a new set of points (batchCount points centered around
// each point in the result hull) and then this method is called again, with batchcount set
// to positive value.
/////////////////////////////////////////////////////////////////////////////////////////

bool GeoUtils::getConvexHull (vector<TLatLongSP> latlongs, vector<TLatLongSP> & border_latlongs, const int batchCount)
{
	if (latlongs.size() < 2)
	{
		return false;
	}

	long cnt = latlongs.size();

	std::vector<int> border_points;

	std::vector<MapObject> objects;

	for (auto & ll : latlongs)
	{
		objects.emplace_back (*ll.get());
	}

    // find start point.

	int start_index = -1;
	int cur_node = -1, prev_node = -1;

	for (int i=0; i < cnt && (start_index < 0); i++)
    {
		for (int j=i + 1; j < cnt; j++)
		{
			if (batchCount > 0)
			{
				int batch1 = i / batchCount;
				int batch2 = j / batchCount;

				bool firstLast = (batch1==0) && (batch2==(cnt/batchCount - 1));

				if (batch2 - batch1 > 1 && !firstLast)
				{
					continue;
				}

				if (batch2 == batch1)
				{
					if ((j - i > 1) && (j-i != batchCount-1))
					{
						continue;
					}
				}
			}

			bool pair_on_edge = sameHemisphereUsingIndexedPair(i, j, objects);

			if (pair_on_edge)
			{
				start_index = prev_node = i;
				cur_node = j;
				border_points.push_back(start_index);
				border_points.push_back(cur_node);
				break;
			}
		}
	}

	while (cur_node != start_index)
	{
		// why not start with zero? in batch mode our indexes always "grow" so it makes sense to immediately
		// start looking at indexes larger than current node.

		for (int index = cur_node + 1 ; index != cur_node; index++)
		{
			if (index == cnt) { index = 0; }

			if (index == cur_node || index == prev_node) continue;

			if (batchCount > 0)
			{
				int j = cur_node;
				int i = index;

				if (i > j) std::swap(i, j);

				int batch1 = i / batchCount;
				int batch2 = j / batchCount;

				bool firstLast = (batch1==0) && (batch2==(cnt/batchCount - 1));

				if (batch2 - batch1 > 1 && !firstLast)
				{
					continue;
				}

				if (batch2 == batch1)
				{
					if ((j - i > 1) && (j-i != batchCount-1))
					{
						continue;
					}
				}
			}

			bool pair_on_edge = sameHemisphereUsingIndexedPair(cur_node, index, objects);

			if (pair_on_edge)
			{
				border_points.push_back(index);
				prev_node = cur_node;
				cur_node = index;
				break;
			}
		}
	}

	cnt = border_points.size();

	int first = border_points.front();
	int last = border_points.back();

	if (first == last) // must be true when pair_cnt > 1
	{
		border_points.erase(border_points.begin());
	}
	else // if (pair_cnt > 1)
	{
		fprintf (stderr, "not circular\n");
		return false;
	}

	for (auto & index : border_points)
	{
		border_latlongs.push_back(latlongs[index]);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// pairs order: <longitude,latitude>
//
// Creates convex hill with around input "points", which is at least radiusMiles away from the
// closest point in the provided list of points.
//
// To achieve this, we first create convex hull with provided points. Let's say there is 100
// input points. Then around each point
// in the resulting convex hull (let's say there are 30 of them) we create vertCount points
// at radiusMiles distance from them (actually a bit more, so that
// connected line is not closer than radiusMiles to each point). For xample if vertCount is 10,
// in our example we now have 30x10 = 300 points. Then we cann convexHull function
// again with this new set of 300 points as input, and 10 as "batchCount".
// We use batch count for optimization, since in the resulting hull only points around adjacent
// "centers" will form the hull.
/////////////////////////////////////////////////////////////////////////////////////////
bool GeoUtils::getConvexHull (vector<std::pair<double,double> > points,
                 vector<std::pair<double,double> > & output,
				 const double radiusMiles, const int vertCount)
{
	std::vector<TLatLongSP> latlongs;

	for (auto & pt: points)
	{
		auto llsp = std::make_shared<TLatLong>(pt.second, pt.first);

		latlongs.push_back(llsp);
	}

	std::vector<TLatLongSP> border_latlongs;
	std::vector<TLatLongSP> temp_output;

	double trueR = MapObject::getTrueRadius(radiusMiles, vertCount);

	if (getConvexHull(latlongs, border_latlongs, -1))
	{
		for (auto & llsp : border_latlongs)
		{
			MapObject::getNPointsAround (*llsp.get(), trueR, vertCount, temp_output);
		}

		std::vector<TLatLongSP> outline_latlongs;

		if (getConvexHull (temp_output, outline_latlongs, vertCount))
		{
			long cnt = outline_latlongs.size();

			output.reserve(cnt);

			for (auto & llsp : outline_latlongs)
			{
				output.push_back(make_pair(llsp->Longitude(), llsp->Latitude()));
			}
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// output pairs have order <longitude,latitude>
// COUNT is number of vertices in triangle/square/pentagon/hexagon etc.
/////////////////////////////////////////////////////////////////////////////////////////
bool GeoUtils::getPointsAroundCoordinate (const TLatLong & coord,
					const double radiusMiles,
					const int vertCount, vector <pair<double,double> > & output)
{
	std::vector<TLatLongSP> temp_output;

	double trueR = MapObject::getTrueRadius(radiusMiles, vertCount);

	MapObject::getNPointsAround (coord, trueR, vertCount, temp_output);

	for (auto & ll : temp_output)
	{
		output.push_back (make_pair (ll->Longitude(), ll->Latitude() ));
	}

	return true;
}

struct Circle
{
    MapObject center;
    double radius;
    Circle (double lat, double lon, const double r) : center(TLatLong(lat,lon))
    {
        radius = r;
    }
	Circle (MapObject obj, const double r) : center (obj)
	{
		radius = r;
	}
};

Circle smallestEnclosing (const MapObject &a, const MapObject &b, const MapObject &c)
{
	double abCos = a.GetAngleCos (b);
	double bcCos = b.GetAngleCos (c);
	double acCos = a.GetAngleCos (c);

	// min value corresponds to maximum distance.

	int minIndex = -1; // 0 - a, 1 - b, 2 - c.

	minIndex = (abCos < bcCos) ? 2 : 0;
	if (acCos < min(abCos, bcCos)) minIndex = 1;

	if (minIndex == 0)
	{
		MapObject m = MapObject::midpoint (b, c);
		if (m.GetAngleCos(a) > m.GetAngleCos(b)) // distance MA smaller than MB.
		{
			double dist = m.GetAirDistance (b);
			return Circle (m, dist);
		}
	}
	else if (minIndex == 1)
	{
		MapObject m = MapObject::midpoint (a, c);
		if (m.GetAngleCos(b) > m.GetAngleCos(a)) // distance MB smaller than MA.
		{
			double dist = m.GetAirDistance (a);
			return Circle (m, dist);
		}
	}

	else if (minIndex == 2)
	{
		MapObject m = MapObject::midpoint (a, b);
		if (m.GetAngleCos(c) > m.GetAngleCos(a)) // distance MC smaller than MA.
		{
			double dist = m.GetAirDistance (a);
			return Circle (m, dist);
		}
	}

	MapObject m = GeoUtils::getEquidistantPoint (a, b, c);
	double dist = m.GetAirDistance (a);
	return Circle (m, dist);
}

Circle trivial (const vector <MapObjectEx> & input)
{
	const MapObject &cast = (input[0]);
	const MapObject &cast2 = (input[1]);
	const MapObject &cast3 = (input[2]);

    return smallestEnclosing(cast, cast2, cast3);
}

static bool containsPoint (const Circle & c, const MapObject &pt)
{
    bool ret = (c.radius > 0 && (pt.GetAirDistance(c.center) <= c.radius + 1e-15)) ;

    return ret;
}

static int unmarkedIndex (vector <MapObjectEx> & R)
{
    for (int i=0; i < R.size(); i++)
    {
        if (R[i].mark == 0) return i;
    }
    return -1;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Weizl agorithm.
// non recursive version, with MSW fix.
//////////////////////////////////////////////////////////////////////////////////////////
MapObject GeoUtils::smallestCircle (vector <MapObjectEx> & inputP, double & outRadius)
{
	vector <MapObjectEx> inputR;

	if (inputP.size() == 2)
	{
		MapObject m = MapObject::midpoint (inputP[0], inputP[1]);
		outRadius = m.GetAirDistance (inputP[0]);
		return m;
	}

    // 1. pick first three Points and put into inputR (while removing from P).

    for (int i=0; i < 3; i++)
    {
        inputR.push_back(inputP[0]);
        inputP.erase (inputP.begin());
    }

    int index = 0;
    Circle r = trivial(inputR);

	while (index < inputP.size())
	{
		MapObjectEx pt = inputP[index];

		if (containsPoint (r, pt)) { index++; continue; }

		// pt is not in r.

		pt.mark = 1;

		int replace = unmarkedIndex (inputR);
		if (replace >= 0)
		{
			inputP[index] = inputR[replace];
			inputR[replace] = pt;
			index = 0;
			r = trivial(inputR);
		}
		else // all indexes required..
		{
			Circle c1 = smallestEnclosing(inputR[0], inputR[1], pt);
			if (containsPoint (c1, inputR[2]))
			{
				// remove 2.
				MapObjectEx pt2 = inputR[2];
				pt2.mark = 0;
				inputP[index] = pt2;
				inputR[2] = pt;
				index = 0;
				r = c1;
				continue;
			}
			c1 = smallestEnclosing(inputR[0], inputR[2], pt);
			if (containsPoint (c1, inputR[1]))
			{
				// remove 1.
				MapObjectEx pt2 = inputR[1];
				pt2.mark = 0;
				inputP[index] = pt2;
				inputR[1] = pt;
				index = 0;
				r = c1;
				continue;
			}
			c1 = smallestEnclosing(inputR[1], inputR[2], pt);
			if (containsPoint (c1, inputR[0]))
			{
				// remove 0.
				MapObjectEx pt2 = inputR[0];
				pt2.mark = 0;
				inputP[index] = pt2;
				inputR[0] = pt;
				index = 0;
				r = c1;
				continue;
			}
		}
	}

	outRadius = r.radius;

	return r.center;
}

TLatLong GeoUtils::mincircle (std::vector<std::pair<double,double> > points, double & outRadius)
{
	vector <MapObjectEx> inputP;
	for (auto & pair : points)
	{
		inputP.push_back (MapObjectEx (pair.second, pair.first));
	}

	MapObject mo = smallestCircle (inputP, outRadius);

	return mo.GetLatLong ();
}
