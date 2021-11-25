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

#include <vector>
#include "ext/GeoUtils.h"
#include "ext/LatLong.h"

#include <chrono>

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////
// lines starting with # (hash) are skipped, considered comments.
// each valid line has longitude and latitude separated by comma. No spaces.
/////////////////////////////////////////////////////////////////////////////////////
bool getCoordinatesFromFile(const char *filename, vector<pair<double, double>> & data)
{
	FILE * input = fopen (filename, "r+t");

	if (!input)
		return false;

	double latitude, longitude;
	char buffer[256];

	while (!feof(input))
	{
		longitude = latitude = 0;

		long pos = ftell (input);

		// check for comment (#)
		fgets (buffer, 256, input);

		if (buffer[0] == '#') continue;

		fseek (input, pos, SEEK_SET);

		int ret = fscanf (input, "%lf,%lf", &longitude, &latitude);

		if (ret != EOF && (longitude != 0 || latitude != 0))
		{
			data.push_back(make_pair(longitude, latitude));
		}
	}

	fclose (input);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool createOutput (const char *filename, vector<pair<double, double>> & data)
{
	FILE * output = fopen (filename, "w+t");

	if (!output)
	{
		fprintf (stderr, "Cannot open %s for writing\n", filename);
		return false;
	}

	fprintf (output, "{ \"type\" : \"Polygon\", \n");
	fprintf (output, "\"coordinates\" : [ \n");

	fprintf (output, "[ \n");

	bool first = true;
	for (auto & pair : data)
	{
		if (!first) fprintf(output, ",\n");
		first = false;

		fprintf (output, "[%.5lf, %.5lf]", pair.first, pair.second);
	}

	fprintf (output, "\n ]]");

	fprintf (output, "\n}");

	fclose (output);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////

int function_Area_And_MinCircle (char * argv[], int which)
{
	vector <pair<double,double> > input;
	vector <pair<double,double> > output;

	int vertCount = atoi (argv[3]);

	if (vertCount < 3)
	{
		printf ("Invalid vertex count. Must be integer greater than 2\n");
		return -1;
	}

	double radiusKM = -1;
	
	if (which == 0)
	{
		radiusKM = atof (argv[4]);

		if (radiusKM <= 0)
		{
			printf ("Invalid radius\n");
			return -1;
		}
	}

	if (!getCoordinatesFromFile (argv[2], input))
	{
		fprintf (stderr, "Cannot open %s\n", argv[2]);
		return -1;
	}

	int ret = false;

	char * outFile = nullptr;

	double radiusMiles = radiusKM * 1000.0 / MapObject::MILE_2_METERS;

	printf ("Input: %ld coordinates\n", input.size());

	if (input.size() == 0)
	{
		fprintf (stderr, "No valid coordinates found in %s\n", argv[2]);
		return -1;
	}
    else if (input.size() == 1)
	{
		TLatLong coord (input.front().second, input.front().first);

		ret = GeoUtils::getPointsAroundCoordinate (coord, radiusMiles, vertCount, output);
	}
	else if (input.size() > 1)
	{
		std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

		if (which == 0)
		{
			outFile = strdup ("area.geojson");

			ret = GeoUtils::getConvexHull(input, output, radiusMiles, vertCount);

			auto end = std::chrono::high_resolution_clock::now();
        	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);        


			printf ("area completed in %lld ms\n", duration.count());
		}
		else 
		{
			outFile = strdup ("mincircle.geojson");

			double outRadiusMiles;
			TLatLong coord = GeoUtils::mincircle (input, outRadiusMiles);

			printf ("MinCircle (%lf %lf) Radius: %lf miles\n", coord.Latitude(), coord.Longitude(), outRadiusMiles );

			ret = GeoUtils::getPointsAroundCoordinate (coord, outRadiusMiles, vertCount, output);

			auto end = std::chrono::high_resolution_clock::now();
        	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);        

			printf ("mincircle completed in %lld ms\n", duration.count());
		}
	}

	if (ret)
	{
		if (createOutput (outFile, output))
		{
			printf ("Successfully created %s with %ld coordinates\n", outFile, output.size());
		}
	}

	free (outFile);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

int function_Equidistant (char * argv[])
{
	vector <pair<double,double> > input;
	vector <pair<double,double> > output;

	int vertCount = atoi (argv[3]);

	if (vertCount < 3)
	{
		printf ("Invalid vertex count. Must be integer greater than 2\n");
		return -1;
	}

	if (!getCoordinatesFromFile (argv[2], input))
	{
		fprintf (stderr, "Cannot open %s\n", argv[2]);
		return -1;
	}

	if (input.size() < 3)
	{
		fprintf (stderr, "Fewer than 3 coordinates in %s\n", argv[2]);
		return -1;
	}

	TLatLong pt1 (input[0].second, input[0].first);
	TLatLong pt2 (input[1].second, input[1].first);
	TLatLong pt3 (input[2].second, input[2].first);

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    TLatLong pt = GeoUtils::getEquidistantPoint (pt1, pt2, pt3).GetLatLong();

	auto end = std::chrono::high_resolution_clock::now();
        	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);        

	printf ("eqdist completed in %lld ms\n", duration.count());

    printf ("EQD %lf %lf\n", pt.Latitude(), pt.Longitude());

	printf ("%lf %lf %lf\n", TLatLong::AirDistance (pt, pt1),
				TLatLong::AirDistance(pt, pt2), TLatLong::AirDistance(pt, pt3));

	double distMiles = TLatLong::AirDistance (pt, pt1);
	int ret = GeoUtils::getPointsAroundCoordinate (pt, distMiles, vertCount, output);

	if (ret)
	{
		const char outFile[] = "circle.geojson";
		if (createOutput (outFile, output))
		{
			printf ("Successfully created %s with %ld coordinates\n", outFile, output.size());
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Syntax:
//
//  (A) ./geojson area input.csv 12 50 
//
//  where input.csv is CSV file with longitude,latitude on each line.
//  outputs output.geojson which can be visualized for example on www.geojson.io or similar sites.
//  50 is distance in kilometers. The resulting output polygon will cover all coordinates and area
//  around them within specified distance.
//  12 is number of vertices to create around each input coordinate. The greater, the "rounder"
//  will be output, at the expense of performance.
//
//  (B) ./geojson eqdist input.csv 12
//
//  (C) ./geojson mincircle input.csv 12
//
///////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[] )
{
	vector <pair<double,double> > input;
	vector <pair<double,double> > output;

	int function = -1;


	if (argc > 1)
	{
		if (strcmp (argv[1], "area") == 0)
		{
			function = 0;
		}
		else if (strcmp (argv[1], "mincircle") == 0)
		{
			function = 1;
		} 
		else if (strcmp (argv[1], "eqdist") == 0)
		{
			function = 2;
		}
	}

	if (function < 0)
	{
		printf ("expected arguments: area | mincircle | eqdist\n" );
		return EXIT_SUCCESS;
	}

	if (function == 0 && argc < 5)
	{
		printf ("Arguments: input (csv file), vertices count (greater than 2), radius in km\n");
		printf ("For example:\n");
		printf ("%s area input.csv 12 50\n", argv[0]);
		return EXIT_SUCCESS;
	}

	if (function == 1 && argc < 4)
	{
		printf ("Arguments: input (csv file), vertices count (greater than 2)\n");
		printf ("For example:\n");
		printf ("%s mincircle input.csv 12\n", argv[0]);
		return EXIT_SUCCESS;
	}

	if (function == 2 && argc < 4)
	{
		printf ("Arguments: input (csv file), vertices count (greater than 2)\n");
		printf ("For example:\n");
		printf ("%s eqdist input.csv 12\n", argv[0]);
		return EXIT_SUCCESS;
	}


	if (function == 0 || function == 1)
	{
		return function_Area_And_MinCircle (argv, function) < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
	}

	else if (function == 2)
	{
		return function_Equidistant (argv) < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
	}

	return EXIT_SUCCESS;
}
