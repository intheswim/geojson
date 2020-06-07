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

#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926535897932
#endif


class Math
{
	public:

	constexpr static double PI = M_PI;

    static int imax (int a, int b)
	{
		return (a > b ? a : b);
	}
	static int imin (int a, int b)
	{
		return (a < b ? a : b);
	}

	static double dmax (double a, double b)
	{
		return (a > b ? a : b);
	}
	static double dmin (double a, double b)
	{
		return (a < b ? a : b);
	}

	static double toRadians (double v)
	{
		return v * M_PI/ 180.0;
	}

	static double toDegrees (double v)
	{
		return v * 180.0 / M_PI;
	}

	static double fcos (double v)
	{
		return ::cos (v);
	}

	static double fsin (double v)
	{
		return ::sin (v);
	}
	static double arccos (double v)
	{
		return ::acos (v);
	}

	static double atan (double v)
	{
		return ::atan (v);
	}

	static double abs (double v)
	{
		return fabs(v);
	}

	static double floor (double v)
	{
		return ::floor (v);
	}	
};

