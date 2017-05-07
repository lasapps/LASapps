/*
===============================================================================

FILE:  lasappsutility.cpp

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
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <windows.h> //for GetFileAttributesA()
#include <algorithm>


//static void byebye(bool error = false, bool wait = false)
void byebye(bool error = false, bool wait = false)
{
	if (wait)
	{
		fprintf(stderr, "<press ENTER>\n");
		getc(stdin);
	}
	exit(error);
}

//static double taketime()
double taketime()
{
	return (double)(clock()) / CLOCKS_PER_SEC;
}


bool direxists(const char* dirname)
{
	DWORD ftyp = GetFileAttributesA(dirname);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
	{
		return false;  //dirname does not exists!
		//fprintf(stderr, "ERROR: invalid output directory\n");
		//byebye(true);
	}

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

bool isdir(const char* fullpathname)
{
	return direxists(fullpathname);
}

std::string getcurrentdirectory()
{
	std::string dir;
	char buffer[_MAX_PATH];
	if (GetCurrentDirectoryA(_MAX_PATH - 1, buffer) == 0)
	{
		fprintf(stderr, "ERROR: GetCurrentDirectoryA() returning 0\n");
		byebye(true, false);
	}
	dir = buffer;
	return dir;
}

//gets path, it is the path without the filename
std::string getpathonly(std::string path)
{
	std::string pathonly;


	size_t sep = path.find_last_of("\\/");
	if (sep != std::string::npos) pathonly = path.substr(0, sep);
	else pathonly = "";

	return pathonly;
}

//gets pathname, it is the fullfilename without the extension
std::string getpathnameonly(std::string path)
{
	std::string pathnameonly;
	std::string ext;

	/*
	size_t sep = path.find_last_of("\\/");
	if (sep != std::string::npos)
	path = path.substr(sep + 1, path.size() - sep - 1);
	*/

	size_t dot = path.find_last_of(".");
	if (dot != std::string::npos)
	{
		pathnameonly = path.substr(0, dot);
		ext = path.substr(dot, path.size() - dot);
	}
	else
	{
		pathnameonly = path;
		ext = "";
	}
	return pathnameonly;
}

//gets filename without path and without extension
std::string getfilenameonly(std::string path)
{
	std::string filenameonly;
	std::string ext;

	size_t sep = path.find_last_of("\\/");
	if (sep != std::string::npos)
		path = path.substr(sep + 1, path.size() - sep - 1);

	size_t dot = path.find_last_of(".");
	if (dot != std::string::npos)
	{
		filenameonly = path.substr(0, dot);
		ext = path.substr(dot, path.size() - dot);
	}
	else
	{
		filenameonly = path;
		ext = "";
	}
	return filenameonly;
}


//gets filename's extension only (without the ".")
std::string getextensiononly(std::string path)
{
	std::string filenameonly;
	std::string ext;

	size_t sep = path.find_last_of("\\/");
	if (sep != std::string::npos)
		path = path.substr(sep + 1, path.size() - sep - 1);

	size_t dot = path.find_last_of(".");
	if (dot != std::string::npos)
	{
		filenameonly = path.substr(0, dot);
		ext = path.substr(dot+1, path.size() - (dot+1));
		if (ext.size() != 3) ext = "";
	}
	else
	{
		filenameonly = path;
		ext = "";
	}
	return ext;
}

//return true is "*" found in filename
bool haswildcard(std::string path)
{
	bool wildcardfound = false;

	size_t sep = path.find_last_of("\\/");
	if (sep != std::string::npos)
		path = path.substr(sep + 1, path.size() - sep - 1);

	size_t star = path.find_last_of("*");
	if (star != std::string::npos) wildcardfound = true;

	return wildcardfound;

}


//gets string preceeding the wildcard char (that is "*")
std::string getfilterprefix(std::string path)
{
	std::string prefix;
	std::string suffix;

	/*
	size_t sep = path.find_last_of("\\/");
	if (sep != std::string::npos)
		path = path.substr(sep + 1, path.size() - sep - 1);
	*/

	size_t star = path.find_last_of("*");
	if (star != std::string::npos)
	{
		prefix = path.substr(0, star);
		suffix = path.substr(star, path.size() - star);
	}
	else
	{
		//wildcard not found
		prefix = "";
		suffix = "";
	}
	return prefix;
}

//gets string after the wildcard char (that is "*")
std::string getfiltersuffix(std::string path)
{
	std::string prefix;
	std::string suffix;

	/*
	size_t sep = path.find_last_of("\\/");
	if (sep != std::string::npos)
	path = path.substr(sep + 1, path.size() - sep - 1);
	*/

	size_t star = path.find_last_of("*");
	if (star != std::string::npos)
	{
		prefix = path.substr(0, star);
		suffix = path.substr(star+1, path.size() - (star+1));
	}
	else
	{
		//wildcard not found
		prefix = "";
		suffix = "";
	}
	return suffix;
}

std::string StringToUpper(std::string strToConvert)
{
	std::transform(strToConvert.begin(), strToConvert.end(), strToConvert.begin(), ::toupper);

	return strToConvert;
}
bool term_progress(std::ostream& os, double complete)
{
	static int lastTick = -1;
	int tick = static_cast<int>(complete * 40.0);

	tick = (std::min)(40, (std::max)(0, tick));

	// Have we started a new progress run?
	if (tick < lastTick && lastTick >= 39)
		lastTick = -1;

	if (tick <= lastTick)
		return true;

	while (tick > lastTick)
	{
		lastTick++;
		if (lastTick % 4 == 0)
			os << (lastTick / 4) * 10;
		else
			os << ".";
	}

	if (tick == 40)
		os << " - done.\n";
	else
		os.flush();

	return true;
}
