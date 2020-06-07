/* geojson, Copyright (c) 2013-2020 Yuriy Yakimenko  */


### This is a utility which outputs .jeoson files.
==================================================

### Background:
===============

Part of the code (MapObject, LatLong, MMath) were originally developed in 2008-2010 in Java
when I was working on components of RailBandit. They were also ported into C++ for the backend,
and largely have not been modified since.

In 2013 I developed the `area`  function when Apple's iTunes added functionality to specify geojson
file to describe the transit coverage area for a transit application (RailBandit app was one). 

`Mincircle` and `Equidistant` parts are based on materials from Wikipedia and other open sources.

One visualization tool for vieweing results is here:

[Geojson.io](https://geojson.io)

(click Open > File, select `output.geojson`)

======================================================================

There are three main built-in functions invoked from command line:

1. Take a collection of coordinates (lat/longs) as input in .csv or .txt file and ouput the "enclosing boundary area" where
all bondary points will be at least given distance from input points. As parical example, take all NJ Transit stations locations
as input and output area within X km of these stations.

Syntax:

`./geojson area input.csv 12 50`

Here 50 is distance in kilometers, 12 is number of points around each input coordinate. This number must be greater than 2, and the 
greater this number is, the more "round" looking will be the boundary.

2. Second function is calculating equidistant point based on three geographic coordinates.

See `GeoUtils::getEquidistantPoint`

Syntax:

`./geojson eqdist input.csv 12`

_Note: only the first three coordinates in .csv file will be used in this case._

3. Find "min circle" (minimum bounding circle) based on a collection of input points.

See  `GeoUtils::mincircle`

[Wikipedia link for Smallest Circle problem](https://en.wikipedia.org/wiki/Smallest-circle_problem)

Syntax:

`./geojson mincirle input.csv 12`

//////////////////////////////////////////////////////////////////////////////////////////

### Known issues:
=================

I have not included a check in `eqdist` function to make sure all three locations are
different. Obviously the program will fail when input is bad (the same is true with other two functions).

I also have not included any checks for distance being too large, etc. 

### Ideas:
==========

Add [Appolonius problem](https://en.wikipedia.org/wiki/Problem_of_Apollonius) solution 
(spherical case) as #4 function into this utility.



