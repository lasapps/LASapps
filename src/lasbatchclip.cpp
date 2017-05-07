/*
===============================================================================

FILE:  lasbatchclip.cpp

CONTENTS:

This app clips multiple LIDAR data LAS files against multiple
SHAPEFILE files containing polygons. To each LAS file matches
a SHAPEFILE file containing a list of polygons. For each polygon,
this app creates an output LAS file containing the points lying 
within that polygon.

This app is multithread and relies on LASapps lasclip to do
the clipping.

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

5 April 2017 -- created in Benoit St-Onge's Lab at UQAM in Montreal

===============================================================================
*/
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>
//#include <string.h>
#include <string>
using namespace std;

#include <windows.h> //for direxists()
#include <Shlwapi.h> //for PathIsRelative()
#include "lasappsutility.h"
#include <iostream> //for term_progress()
#include <thread>
#include "dirent.h" //for opendir() and readdir()

//globals
string lasfilesfilterstring;
string shapefilesfilterstring;
string outputdirstring;
string fieldindexnamestring;
string lasclippathstring;
string lasclipworkingdirstring;

vector<vector<string>*> global_psyscommandvector_forcore;

int matchstringoffset = 0; //defaults to 0
int matchstringlength = 0; //defaults to 0, no additional substring matching


void usage(bool error = false, bool wait = false)
{
	fprintf(stderr, "usage:\n");
	fprintf(stderr, "lasbatchclip -i *.las -poly *.shp -odirbasedonlas -fieldindexname Object_ID -lasclippath c:\\lasclip.exe -lasclipworkingdir c:\\ -cores 8\n");
	fprintf(stderr, "lasbatchclip -h\n");
	fprintf(stderr, "----------------------------------------------------------------------------\n");
	fprintf(stderr, "-i flag to specify LAS input files\n");
	fprintf(stderr, "-poly flag to specify SHAPEFILE input files\n");
	fprintf(stderr, "      contain polygons.\n");
	fprintf(stderr, "-matchstringoffset flag is optional, it specifies the offset\n");
	fprintf(stderr, "                   in the LAS filename for the matching with\n");
	fprintf(stderr, "                   the SHAPEFILE filename.\n");
	fprintf(stderr, "-matchstringlength flag is optional, it specifies the length\n");
	fprintf(stderr, "                   in the LAS filename for the matching with\n");
	fprintf(stderr, "                   the SHAPEFILE filename.\n");
	fprintf(stderr, "-odir flag is optional, it specifies output directory,\n");
	fprintf(stderr, "      the default output directory is the input LAS file\n");
	fprintf(stderr, "      path.\n");
	fprintf(stderr, "-odirbasedonlas flag is optional, it tells lasbatchclip to\n");
	fprintf(stderr, "                automatically create output directory based\n");
	fprintf(stderr, "                on LAS input files.\n");
	fprintf(stderr, "-fieldindexname flag is optional, it specifies the field\n");
	fprintf(stderr, "                index name of the SHAPEFILE's attribute\n");
	fprintf(stderr, "                that will be used to tag LAS output files,\n");
	fprintf(stderr, "                the default is Object_ID. If the field index\n");
	fprintf(stderr, "                name is not found in the SHAPEFILE, default\n");
	fprintf(stderr, "                polygon index will be used to tag LAS ouput\n");
	fprintf(stderr, "                files. The field index name should be unique\n");
	fprintf(stderr, "                for each polygon.\n");
	fprintf(stderr, "-lasclippath flag is mandatory, it tells lasbatchclip where\n");
	fprintf(stderr, "             to find lasclip.exe\n");
	fprintf(stderr, "-lasclipworkingdir flag is optional, it tells lasbatchclip which\n");
	fprintf(stderr, "                   working directory to use for lasclip.exe\n");
	fprintf(stderr, "-cores flag is optional, if used it specifies the number of cores to be used.\n");
	fprintf(stderr, "-verbose flag is optional, if used it details the process.\n");
	fprintf(stderr, "-h flag is used to produce this usage help screen.\n");
	fprintf(stderr, "----------------------------------------------------------------------------\n");
	fprintf(stderr, "This process can be accelerated greatly if a LAX files are used.\n");
	fprintf(stderr, "One can generate these LAX files using LAStools' lasindex.exe and\n");
	fprintf(stderr, "the LAX filename should be identical to the LAS input file for \n");
	fprintf(stderr, "lasclip process to be accelerated. For example, using test.lax\n");
	fprintf(stderr, "along with test.las and test.shp.\n");
	fprintf(stderr, "----------------------------------------------------------------------------\n");
	fprintf(stderr, "There is no reprojection support in LASapps lasclip version 0.1.\n");
	fprintf(stderr, "The same SRS is expected to be used for both the LAS file and \n");
	fprintf(stderr, "the SHAPEFILE.\n");
	fprintf(stderr, "----------------------------------------------------------------------------\n");
	fprintf(stderr, "The SHAPEFILE layer name is expected to be the same as the SHAPEFILE name\n");
	fprintf(stderr, "(without the path and without the extension.\n");
	fprintf(stderr, "----------------------------------------------------------------------------\n");
	if (wait)
	{
		fprintf(stderr, "<press ENTER>\n");
		getc(stdin);
	}
	exit(error);
}

void execute_syscommandvector(int threadid)
{
	//for (int i = 0; i < global_psyscommandvector_forcore[threadid][0].size(); i++)
	for (int i = 0; i < global_psyscommandvector_forcore[threadid]->size(); i++)
	{
		string syscommand = global_psyscommandvector_forcore[threadid][0][i].c_str();
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		// Start the child process. 
		if (!CreateProcess(NULL,   // No module name (use command line)
			const_cast<char*>(syscommand.c_str()),        // Command line
			NULL,			// Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			//0,              // No creation flags
			HIGH_PRIORITY_CLASS,
			NULL,           // Use parent's environment block
			lasclipworkingdirstring.c_str(),	// if NULL uses parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi)           // Pointer to PROCESS_INFORMATION structure
			)
		{
			printf("ERROR: CreateProcess failed (%d).\n", GetLastError());
			//byebye(true, false);
			return;
		}

		// Wait until child process exits.
		WaitForSingleObject(pi.hProcess, INFINITE);

		// Close process and thread handles. 
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

}

int main(int argc, char *argv[])
{
	int i;
	double start_time = 0.0;

	//defaults
	bool verbose = false; // true;
	int cores = 1;

	/*
	string lasfilesfilterstring;
	string shapefilesfilterstring;
	string outputdirstring;
	string lasclippathstring;
	string lasclipworkingdirstring;
	*/
	if (argc == 1)
	{
		fprintf(stderr, "%s is better run in the command line\n", argv[0]);

		CHAR lasfilesfilter[256];
		fprintf(stderr, "enter LAS input files filter: "); fgets(lasfilesfilter, 256, stdin);
		lasfilesfilter[strlen(lasfilesfilter) - 1] = '\0';
		lasfilesfilterstring = lasfilesfilter;

		CHAR shapefilesfilter[256];
		fprintf(stderr, "enter SHAPEFILE input files filter: "); fgets(shapefilesfilter, 256, stdin);
		shapefilesfilter[strlen(shapefilesfilter) - 1] = '\0';
		shapefilesfilterstring = shapefilesfilter;

		CHAR outputdir[256];
		fprintf(stderr, "enter output directory name: "); fgets(outputdir, 256, stdin);
		outputdir[strlen(outputdir) - 1] = '\0';
		outputdirstring = outputdir;
		if (outputdirstring.empty())
		{
			outputdirstring = getpathonly(lasfilesfilterstring);
			if (outputdirstring.empty())
			{
				outputdirstring = getcurrentdirectory();
			}
		}

		CHAR fieldindex_name[256];
		fprintf(stderr, "enter FIELDINDEX name: "); fgets(fieldindex_name, 256, stdin);
		fieldindex_name[strlen(fieldindex_name) - 1] = '\0';
		fieldindexnamestring = fieldindex_name;

		CHAR lasclippath[256];
		fprintf(stderr, "enter lasclip.exe fullpath: "); fgets(lasclippath, 256, stdin);
		lasclippath[strlen(lasclippath) - 1] = '\0';
		lasclippathstring = lasclippath;

		CHAR lasclipworkingdir[256];
		fprintf(stderr, "enter working directory for lasclip.exe: "); fgets(lasclipworkingdir, 256, stdin);
		lasclipworkingdir[strlen(lasclipworkingdir) - 1] = '\0';
		lasclipworkingdirstring = lasclipworkingdir;
	}
	else
	{
		for (i = 1; i < argc; i++)
		{
			if (argv[i][0] == '–') argv[i][0] = '-';
		}
	}

	//defaults
	if (fieldindexnamestring.empty()) fieldindexnamestring = "Object_ID";

	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '\0')
		{
			continue;
		}
		else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0)
		{
			fprintf(stderr, "LASapps lasbatchclip version 0.1\n");
			usage();
		}
		else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-verbose") == 0)
		{
			verbose = true;
		}
		else if (strcmp(argv[i], "-version") == 0)
		{
			fprintf(stderr, "LASapps lasbatchclip version 0.1\n");
			byebye();
		}
		else if (strcmp(argv[i], "-i") == 0)
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "ERROR: '%s' needs 1 argument: string\n", argv[i]);
				usage(true);
			}
			i++;
			lasfilesfilterstring = argv[i];
			argv[i][0] = '\0';
			if (lasfilesfilterstring.empty())
			{
				fprintf(stderr, "ERROR: lasfilesfilterstring is empty\n");
				byebye(true, argc == 1);
			}
		}
		else if (strcmp(argv[i], "-poly") == 0)
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "ERROR: '%s' needs 1 argument: string\n", argv[i]);
				usage(true);
			}
			i++;
			shapefilesfilterstring = argv[i];
			argv[i][0] = '\0';
			if (shapefilesfilterstring.empty())
			{
				fprintf(stderr, "ERROR: shapefilesfilterstring is empty\n");
				byebye(true, argc == 1);
			}
		}
		else if (strcmp(argv[i], "-matchstringoffset") == 0)
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "ERROR: '%s' needs 1 argument: string\n", argv[i]);
				usage(true);
			}
			i++;
			matchstringoffset = atoi(argv[i]);
			argv[i][0] = '\0';
		}
		else if (strcmp(argv[i], "-matchstringlength") == 0)
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "ERROR: '%s' needs 1 argument: string\n", argv[i]);
				usage(true);
			}
			i++;
			matchstringlength = atoi(argv[i]);
			argv[i][0] = '\0';
		}
		else if (strcmp(argv[i], "-odir") == 0)
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "ERROR: '%s' needs 1 argument: string\n", argv[i]);
				usage(true);
			}
			i++;
			outputdirstring = argv[i];
			argv[i][0] = '\0';
			if (PathIsRelative(outputdirstring.c_str()))
			{
				/*
				outputdirstring = getpathonly(lasfilesfilterstring) + "\\" + outputdirstring;
				*/
				string laspathonly = getpathonly(lasfilesfilterstring);
				if (laspathonly.empty())
				{
					laspathonly = getcurrentdirectory();
				}
				outputdirstring = laspathonly + "\\" + outputdirstring;
			}
		}
		else if (strcmp(argv[i], "-odirbasedonlas") == 0)
		{
			outputdirstring = "basedonlas"; //will be set later;
		}
		else if (strcmp(argv[i], "-fieldindexname") == 0)
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "ERROR: '%s' needs 1 argument: string\n", argv[i]);
				usage(true);
			}
			i++;
			fieldindexnamestring = argv[i];
			argv[i][0] = '\0';
		}
		else if (strcmp(argv[i], "-lasclippath") == 0)
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "ERROR: '%s' needs 1 argument: string\n", argv[i]);
				usage(true);
			}
			i++;
			lasclippathstring = argv[i];
			argv[i][0] = '\0';
		}
		else if (strcmp(argv[i], "-lasclipworkingdir") == 0)
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "ERROR: '%s' needs 1 argument: string\n", argv[i]);
				usage(true);
			}
			i++;
			lasclipworkingdirstring = argv[i];
			argv[i][0] = '\0';
		}
		else if (strcmp(argv[i], "-cores") == 0)
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "ERROR: '%s' needs 1 argument: string\n", argv[i]);
				usage(true);
			}
			i++;
			cores = atoi(argv[i]);
			argv[i][0] = '\0';
			if (cores < 0) cores = 1;
			unsigned concurentThreadsSupported = thread::hardware_concurrency();
			//if (cores > concurentThreadsSupported) cores = concurentThreadsSupported; //commented out to leave user fully manage number of concurrent threads
		}
		else
		{
			fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
			usage(true);
		}
	}

	// check input
	if (0)
	{
		fprintf(stderr, "ERROR: no input specified\n");
		byebye(true, argc == 1);
	}

	/* //did not seem to improve threads and child processes performance
	if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
	{
		//DWORD dwError = GetLastError();
		fprintf(stderr, "WARNING: could not set main process to HIGH_PRIORITY_CLASS\n");
	}
	*/

	if (verbose) start_time = taketime();

	////////////////////////////////////////////
	// collect multiple LAS (or LAZ) input files
	////////////////////////////////////////////
	vector<string> lasfilesvector;
	string lasfilesdirectory = getpathonly(lasfilesfilterstring);
	if (lasfilesdirectory.empty())
	{
		lasfilesdirectory = getcurrentdirectory();
	}
	string lasfilesextension = getextensiononly(lasfilesfilterstring);
	if (lasfilesextension != "las" && lasfilesextension != "LAS" && lasfilesextension != "laz" && lasfilesextension != "LAZ")
	{
		fprintf(stderr, "ERROR: the input LAS files filter uses unknown file extension %s\n", lasfilesextension.c_str());
		byebye(true, argc == 1);
	}
	bool lasfilesfilterhaswildcard = haswildcard(getfilenameonly(lasfilesfilterstring));
	if (!lasfilesfilterhaswildcard)
	{
		fprintf(stderr, "ERROR: no \"*\" found in input LAS files filter %s\n", lasfilesfilterstring.c_str());
		byebye(true, argc == 1);
	}
	string lasfilesfilterprefix = getfilterprefix(getfilenameonly(lasfilesfilterstring));
	string lasfilesfiltersuffix = getfiltersuffix(getfilenameonly(lasfilesfilterstring));
	DIR* pDIR;
	struct dirent* pdirent;
	if ((pDIR = opendir(lasfilesdirectory.c_str())) != NULL)
	{
		//find all the files and directories within directory
		while ((pdirent = readdir(pDIR)) != NULL) 
		{
			//printf("%s\n", pdirent->d_name);
			size_t c;
			string name = pdirent->d_name;
			if (name != "." && name != "..")
			{
				//if extension matches	
				string thisext = getextensiononly(name);
				string thisnamewithoutext = getfilenameonly(name);
				if (thisext == lasfilesextension || thisext == StringToUpper(lasfilesextension))
				{
					//if prefix matches
					if (lasfilesfilterprefix.empty() || (!lasfilesfilterprefix.empty() && (name.find(lasfilesfilterprefix) == 0) || name.find(StringToUpper(lasfilesfilterprefix)) == 0))
					{
						//if suffix matches						
						if (lasfilesfiltersuffix.empty() || (!lasfilesfiltersuffix.empty() && (thisnamewithoutext.rfind(lasfilesfiltersuffix) == (thisnamewithoutext.size() - lasfilesfiltersuffix.size())) || (thisnamewithoutext.rfind(StringToUpper(lasfilesfiltersuffix)) == (thisnamewithoutext.size() - lasfilesfiltersuffix.size()))))
						{
							//if item is not a directory
							string fullpathname = lasfilesdirectory + "\\" + name;
							if (!isdir(fullpathname.c_str()))
							{
								//then we have a complete match, keep this file
								if(verbose) fprintf(stderr, "found %s\n", name.c_str());
								lasfilesvector.push_back(fullpathname);
							}
						}
					}
				}
			}
		}
		closedir(pDIR);
	}
	else 
	{
		//could not open directory
		//perror("");
		//return EXIT_FAILURE;
		fprintf(stderr, "ERROR: can't open LAS files directory \"%s\"\n", lasfilesdirectory.c_str());
		byebye(true, argc == 1);
	}

	/////////////////////////////////////////
	// collect multiple SHAPEFILE input files
	/////////////////////////////////////////
	vector<string> shapefilesvector;
	string shapefilesdirectory = getpathonly(shapefilesfilterstring);
	if (shapefilesdirectory.empty())
	{
		shapefilesdirectory = getcurrentdirectory();
	}
	string shapefilesextension = getextensiononly(shapefilesfilterstring);
	if (shapefilesextension != "shp" && shapefilesextension != "SHP")
	{
		fprintf(stderr, "ERROR: the input SHAPEFILE files filter uses unknown file extension %s\n", shapefilesextension.c_str());
		byebye(true, argc == 1);
	}
	bool shapefilesfilterhaswildcard = haswildcard(getfilenameonly(shapefilesfilterstring));
	if (!shapefilesfilterhaswildcard)
	{
		fprintf(stderr, "ERROR: no \"*\" found in input SHAPEFILE files filter %s\n", shapefilesfilterstring.c_str());
		byebye(true, argc == 1);
	}
	string shapefilesfilterprefix = getfilterprefix(getfilenameonly(shapefilesfilterstring));
	string shapefilesfiltersuffix = getfiltersuffix(getfilenameonly(shapefilesfilterstring));
	//DIR* pDIR;
	//struct dirent* pdirent;
	if ((pDIR = opendir(shapefilesdirectory.c_str())) != NULL)
	{
		//find all the files and directories within directory
		while ((pdirent = readdir(pDIR)) != NULL)
		{
			//printf("%s\n", pdirent->d_name);
			size_t c;
			string name = pdirent->d_name;
			if (name != "." && name != "..")
			{
				//if extension matches	
				string thisext = getextensiononly(name);
				string thisnamewithoutext = getfilenameonly(name);
				if (thisext == shapefilesextension || thisext == StringToUpper(shapefilesextension))
				{
					//if prefix matches
					if (shapefilesfilterprefix.empty() || (!shapefilesfilterprefix.empty() && (name.find(shapefilesfilterprefix) == 0) || name.find(StringToUpper(shapefilesfilterprefix)) == 0))
					{
						//if suffix matches						
						if (shapefilesfiltersuffix.empty() || (!shapefilesfiltersuffix.empty() && (thisnamewithoutext.rfind(shapefilesfiltersuffix) == (thisnamewithoutext.size() - shapefilesfiltersuffix.size())) || (thisnamewithoutext.rfind(StringToUpper(shapefilesfiltersuffix)) == (thisnamewithoutext.size() - shapefilesfiltersuffix.size()))))
						{
							//if item is not a directory
							string fullpathname = shapefilesdirectory + "\\" + name;
							if (!isdir(fullpathname.c_str()))
							{
								//then we have a complete match, keep this file
								if(verbose) fprintf(stderr, "found %s\n", name.c_str());
								shapefilesvector.push_back(fullpathname);
							}
						}
					}
				}
			}
		}
		closedir(pDIR);
	}
	else
	{
		//could not open directory
		fprintf(stderr, "ERROR: can't open SHAPEFILE files directory \"%s\"\n", shapefilesdirectory.c_str());
		byebye(true, argc == 1);
	}


	//////////////////////////////////////
	//match LAS files with SHAPEFILE files
	//////////////////////////////////////
	if (lasfilesvector.size() == 0 || shapefilesvector.size() == 0)
	{
		fprintf(stderr, "ERROR: won't be able to match one to one LAS files with SHAPEFILE files\n");
		fprintf(stderr, "ERROR: found %d LAS files and found %d SHAPEFILE files\n", lasfilesvector.size(), shapefilesvector.size());
		byebye(true, argc == 1);
	}
	if (lasfilesvector.size() != shapefilesvector.size())
	{
		fprintf(stderr, "WARNING: number of LAS files differs from number of SHAPEFILE files\n");
		fprintf(stderr, "WARNING: found %d LAS files and found %d SHAPEFILE files\n", lasfilesvector.size(),shapefilesvector.size());
		byebye(true, argc == 1); //byebye(true, argc == 1);
	}
	vector<string> shapefilesmatchedvector;
	if (matchstringoffset < 0) matchstringoffset = 0;
	if (matchstringlength < -1) matchstringlength = -1;
	vector<string>::iterator it1;
	vector<string>::iterator it2;
	for (it1 = lasfilesvector.begin(); it1 != lasfilesvector.end(); ++it1)
	{
		if (matchstringlength != -1 && matchstringlength != 0)
		{
			//match string normally
			string filename = getfilenameonly(*it1);
			string filenameprefix = filename.substr(matchstringoffset, matchstringlength);
			for (it2 = shapefilesvector.begin(); it2 != shapefilesvector.end(); ++it2)
			{
				if ((getfilenameonly(*it2)).find(filenameprefix) == 0)
				{
					//match found
					shapefilesmatchedvector.push_back(*it2);
					shapefilesvector.erase(it2);
					break;
				}
			}
		}
		else if (matchstringlength==-1)
		{
			//match string length will adapt
			string filename = getfilenameonly(*it1);
			if (matchstringoffset > (filename.size() - 1)) matchstringoffset = filename.size() - 1;
			string filenameprefix = filename.substr(matchstringoffset, filename.size()-matchstringoffset);
			for (it2 = shapefilesvector.begin(); it2 != shapefilesvector.end(); ++it2)
			{
				if ((getfilenameonly(*it2)).find(filenameprefix) == 0)
				{
					//match found
					shapefilesmatchedvector.push_back(*it2);
					shapefilesvector.erase(it2);
					break;
				}
			}
		}
		else if (matchstringlength==0)
		{
			//no match string used, match in the same order files are found
			//nothing to do for now, will copy vector as is 
		}
		else
		{
			fprintf(stderr, "ERROR: unforseen case with matchstringlength %d\n", matchstringlength);
			byebye(true, argc == 1);
		}
	}
	if (matchstringlength==0)
	{
		//copy vector
		shapefilesmatchedvector = shapefilesvector;
	}
	if (lasfilesvector.size() != shapefilesmatchedvector.size())
	{
		fprintf(stderr, "WARNING: number of LAS files differs from number of matched SHAPEFILE files\n");
		fprintf(stderr, "WARNING: found %d LAS files and found %d matching SHAPEFILE files\n", lasfilesvector.size(), shapefilesmatchedvector.size());
		byebye(true, argc == 1); //byebye(true, argc == 1);
	}


	////////////////////////////
	//construct outputdir vector
	////////////////////////////
	if (outputdirstring.empty())
	{
		//defaults to las files directory
		outputdirstring = getpathonly(lasfilesfilterstring);
		if (outputdirstring.empty())
		{
			outputdirstring = getcurrentdirectory();
		}
	}
	else
	{
		//outputdirstring is already set and does not change
	}
	vector<string> outputdirvector;
	vector<string>::iterator it;
	for (it = lasfilesvector.begin(); it != lasfilesvector.end(); ++it)
	{
		if (outputdirstring == "basedonlas")  
		{
			//option basedonlas, new dir will be created using las file names
			/*
			outputdirvector.push_back(getpathonly(lasfilesfilterstring) + "\\" + getfilenameonly(*it));
			*/
			string laspathonly = getpathonly(lasfilesfilterstring);
			if (laspathonly.empty())
			{
				laspathonly = getcurrentdirectory();
			}
			outputdirvector.push_back(laspathonly + "\\" + getfilenameonly(*it));
		}
		else
		{
			outputdirvector.push_back(outputdirstring);
		}
	}

	////////////////////////////
	//construct system cmd lines
	////////////////////////////
	string quote = "\"";
	string syscommand;
	vector<string> syscommandvector;
	//vector<string>::iterator it1;
	//vector<string>::iterator it2;
	vector<string>::iterator it3;
	//for (it1 = lasfilesvector.begin(), it2 = shapefilesvector.begin(), it3 = outputdirvector.begin(); it1 != lasfilesvector.end() && it2 != shapefilesvector.end() && it3 != outputdirvector.end(); ++it1, ++it2, ++it3)
	for (it1 = lasfilesvector.begin(), it2 = shapefilesmatchedvector.begin(), it3 = outputdirvector.begin(); it1 != lasfilesvector.end() && it2 != shapefilesmatchedvector.end() && it3 != outputdirvector.end(); ++it1, ++it2, ++it3)
	{
		syscommand = quote + lasclippathstring + quote + " -i " + quote + *it1 + quote + " -poly " + quote + *it2 + quote + " -odir " + quote + *it3 + quote + " -fieldindexname " + fieldindexnamestring;
		if(verbose) fprintf(stderr, "%s\n", syscommand.c_str());
		syscommandvector.push_back(syscommand);
	}

	////////////////////////////////////////
	//distribute system cmd lines over cores
	////////////////////////////////////////
	if (cores > syscommandvector.size()) cores = syscommandvector.size();
	//vector<vector<string>*> global_psyscommandvector_forcore;
	for (i = 0; i < cores; i++)
	{
		vector<string>* psyscommandvector = new vector<string>;
		if (psyscommandvector) global_psyscommandvector_forcore.push_back(psyscommandvector);
		else 
		{
			fprintf(stderr, "ERROR: allocating vector<string>\n");
			byebye(true, argc == 1);
		}
	}
	//for each core, a list of system cmd lines is prepared
	i = -1;
	//vector<string>::iterator it;
	for (it = syscommandvector.begin(); it != syscommandvector.end(); ++it)
	{
		i++;
		if (i == cores) i = 0;
		global_psyscommandvector_forcore[i]->push_back(*it);
	}


	//////////////////////////
	//execute system cmd lines
	//////////////////////////
	vector<thread*> pthreadvector;
	for (i = 0; i < cores; i++)
	{
		//creates one thread per core
		thread* pthread = new thread;
		if (pthread) pthreadvector.push_back(pthread);
		else
		{
			fprintf(stderr, "ERROR: allocating thread\n");
			byebye(true, argc == 1);
		}
		//each thread executes its assigned serie of system cmd lines
		*pthread = thread(execute_syscommandvector, i);
		/*
		if (!SetThreadPriority(pthread->native_handle(), THREAD_PRIORITY_HIGHEST)) //THREAD_PRIORITY_TIME_CRITICAL
		{
			fprintf(stderr, "WARNING: cannot set thread priority to THREAD_PRIORITY_HIGHEST\n");
		}
		*/
	}

	////////////////////////////////
	//wait for all threads to finish
	////////////////////////////////
	for (i = 0; i < cores; i++)
	{
		pthreadvector[i]->join();
	}

	/////////////////
	//exit gracefully
	/////////////////
	//delete dynamically allocated objects
	for (i = 0; i < cores; i++)
	{
		//delete vector<string>*
		if (global_psyscommandvector_forcore[i]) delete global_psyscommandvector_forcore[i];

		//delete thread*
		if (pthreadvector[i]) delete pthreadvector[i];
	}
#ifdef _WIN64
	if (verbose) fprintf(stderr, "called lasclip.exe %I64d times over %I64d cores took %g sec.\n", syscommandvector.size(), cores, taketime() - start_time);
#else
#ifdef _WIN32
	if (verbose) fprintf(stderr, "called lasclip.exe %d times over %d cores took %f sec.\n", syscommandvector.size(), cores, taketime() - start_time);
#else
	if (verbose) fprintf(stderr, "called lasclip.exe  %lld times over %lld cores took %g sec.\n", syscommandvector.size(), cores, taketime() - start_time);
#endif
#endif

	byebye(false, argc == 1);
	return 0;
}
