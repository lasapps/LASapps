/*
===============================================================================

FILE:  lasappsutility.h

CONTENTS:

This file is part of LASapps collection of applications for 
LIDAR data LAS files processing and visualizing.

THANKS:

Thanks to Benoit St-Onge for ideas, concepts and supervision.

PROGRAMMER:

stephane.poirier@oifii.org  -  http://www.oifii.org

SUPPORT:

new releases - http://www.lasapps.org
user support - https://groups.google.com/forum/#!forum/geo_spi-users

COPYRIGHT:

(c) 2017, Stephane Poirier

This is free software; you can redistribute and/or modify it under the
terms of the GNU Lesser General Licence as published by the Free Software
Foundation. See the LICENSE.txt file for more information.

This software is distributed WITHOUT ANY WARRANTY and without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

CHANGE HISTORY:

3 April 2017 -- created in Benoit St-Onge's Lab at UQAM in Montreal

===============================================================================
*/


void byebye(bool error = false, bool wait = false);

double taketime();

bool direxists(const char* dirname);

bool isdir(const char* fullpathname);

std::string getcurrentdirectory();

//gets path, it is the path without the filename
std::string getpathonly(std::string path);

//gets pathname, it is the fullfilename without the extension
std::string getpathnameonly(std::string path);

//gets filename without path and without extension
std::string getfilenameonly(std::string path);

//gets filename's extension only (without the ".")
std::string getextensiononly(std::string path);

//return true is "*" found in filename
bool haswildcard(std::string path);

//gets string preceeding the wildcard char (that is "*")
std::string getfilterprefix(std::string path);

//gets string after the wildcard char (that is "*")
std::string getfiltersuffix(std::string path);

std::string StringToUpper(std::string strToConvert);

bool term_progress(std::ostream& os, double complete);

