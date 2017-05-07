LASapps: open source software for LiDAR processing

You find all individual LASapps in the .\LASapps\x64\Release (64 bit apps) 
and .\LASapps\Release (32 bit apps) directories. Start them by double-clicking 
or run them in the DOS command line. 

new releases - http://www.lasapps.org
user support - https://groups.google.com/forum/#!forum/geo_spi-users

open source apps:

* lasclip.exe clips LIDAR data from the binary LAS format against a shapefile 
			  containing polygons. For each polygon, the app creates a LAS 
			  output file containing the points lying within that polygon.
			  The 64 bit version of lasclip.exe loads all LAS file points in
			  memory while the 32 bit version loads only one point at a time
			  in memory.
* lasbatchclip.exe clips multiple LIDAR data LAS files against multiple
				   SHAPEFILE files containing polygons. To each LAS file 
				   matches a SHAPEFILE file containing a list of polygons. 
				   For each polygon, this app creates an output LAS file 
				   containing the points lying within that polygon. This 
				   app is multithread and relies on LASapps lasclip to do
				   the clipping.
* lasindex.exe creates a spatial index LAX file for fast spatial queries,
			   lasindex.exe is part of LAStools.


For Windows all binaries are included. All open source apps can be compiled
from the source code. For Visual Studio 2013, there is a project file. LASapps
is compiled on top of a slightly modified version of LAStools. For convinience,
this modified version of LAStools is made available on http://www.lasapps.org
as well. LASApps has been tested under Windows 7 and with LAS file format 1.1.

---

Please read the "LICENSE.txt" file for information on the legal use and licensing
of LASapps.  

(c) 2017 stephane.poirier@oifii.org - http://www.oifii.org
