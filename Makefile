CC=g++
CFLAGS=-std=c++11 -Wall -Werror -pedantic

geojson : main.o latlong.o mapobject.o geoutils.o
				$(CC) -O2 -o geojson main.o geoutils.o latlong.o mapobject.o

main.o : main.cpp
				$(CC) -c $(CFLAGS) main.cpp

geoutils.o : ext/GeoUtils.cpp ext/GeoUtils.h
				$(CC) -c $(CFLAGS) ext/GeoUtils.cpp -o geoutils.o

mapobject.o : ext/MapObject.cpp ext/MapObject.h ext/MMath.h
				$(CC) -c $(CFLAGS) ext/MapObject.cpp -o mapobject.o

latlong.o : ext/LatLong.cpp ext/LatLong.h ext/MapObject.h ext/MMath.h
				$(CC) -c $(CFLAGS) ext/LatLong.cpp -o latlong.o

install: geojson
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $< $(DESTDIR)$(PREFIX)/bin/geojson

.PHONY: clean
clean :
				-rm main.o latlong.o mapobject.o geoutils.o

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/geojson