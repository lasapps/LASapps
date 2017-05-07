/*
===============================================================================

  FILE:  lasclip.cpp
  
  CONTENTS:
  
    This app clips LIDAR data from the binary LAS format against a
    shapefile containing polygons. For each polygon, the app creates
	a LAS output file containing the points lying within that polygon.

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
  
	23 April 2017 -- added class LASreadOpenerRAM and class LASreaderLASRAM
     3 April 2017 -- created in Benoit St-Onge's Lab at UQAM in Montreal

===============================================================================
*/
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lasreader.hpp"
#include "laswaveform13reader.hpp"
#include "laswriter.hpp"

#include "lasreadopenerram.h"
#include "lasreaderlasram.h"

#include "ogrsf_frmts.h"
//#include <string>
//using namespace std;
#include <windows.h> //for direxists()
#include <Shlwapi.h> //for PathIsRelative()
#include "lasappsutility.h"
#include <iostream> //for term_progress()

void usage(bool error=false, bool wait=false)
{
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"lasclip -i test.las -poly test.shp -odir outputdir -fieldindexname Object_ID\n");
  fprintf(stderr,"lasclip -h\n");
  fprintf(stderr,"----------------------------------------------------------------------------\n");
  fprintf(stderr,"-i flag to specify LAS input file\n");
  fprintf(stderr,"-poly flag to specify SHAPEFILE input file expected to\n");
  fprintf(stderr,"      contain polygons.\n");
  fprintf(stderr,"-odir flag is optional, it specifies output directory,\n");
  fprintf(stderr,"      the default output directory is the input LAS file\n");
  fprintf(stderr,"      path.\n");
  fprintf(stderr,"-fieldindexname flag is optional, it specifies the field\n");
  fprintf(stderr,"                index name of the SHAPEFILE's attribute\n");
  fprintf(stderr,"                that will be used to tag LAS output files,\n");
  fprintf(stderr,"                the default is Object_ID. If the field index\n");
  fprintf(stderr,"                name is not found in the SHAPEFILE, default\n");
  fprintf(stderr,"                polygon index will be used to tag LAS ouput\n");
  fprintf(stderr,"                files. The field index name should be unique\n");
  fprintf(stderr,"                for each polygon.\n");
  fprintf(stderr,"-verbose flag is optional, if used it details the process.\n");
  fprintf(stderr,"-h flag is used to produce this usage help screen.\n");
  fprintf(stderr,"----------------------------------------------------------------------------\n");
  fprintf(stderr,"This process can be accelerated greatly if a LAX file is used.\n");
  fprintf(stderr,"One can generate this LAX file using LAStools' lasindex.exe and\n");
  fprintf(stderr,"the LAX filename should be identical to the LAS input file for \n");
  fprintf(stderr,"lasclip process to be accelerated. For example, using test.lax\n");
  fprintf(stderr,"along with test.las and test.shp.\n");
  fprintf(stderr,"----------------------------------------------------------------------------\n");
  fprintf(stderr,"There is no reprojection support in LASapps lasclip version 0.1.\n");
  fprintf(stderr,"The same SRS is expected to be used for both the LAS file and \n");
  fprintf(stderr,"the SHAPEFILE.\n");
  fprintf(stderr,"----------------------------------------------------------------------------\n");
  fprintf(stderr, "The SHAPEFILE layer name is expected to be the same as the SHAPEFILE name\n");
  fprintf(stderr, "(without the path and without the extension.\n");
  fprintf(stderr, "----------------------------------------------------------------------------\n");
  if (wait)
  {
    fprintf(stderr,"<press ENTER>\n");
    getc(stdin);
  }
  exit(error);
}



int main(int argc, char *argv[])
{
  int i;
  bool verbose = false; // true;
  CHAR separator_sign = ' ';
  CHAR* separator = "space";
  double start_time = 0.0;

#ifdef LASCLIP_RAM
  LASreadOpenerRAM lasreadopener;
#else
  LASreadOpener lasreadopener;
#endif

  LASwriteOpener laswriteopener;
  //laswriteopener.set_format("txt");
  laswriteopener.set_format("las");

  std::string shapefilename;
  std::string shapefilelayername;
  std::string outputdirname;
  std::string fieldindexname;

  if (argc == 1)
  {
	fprintf(stderr,"%s is better run in the command line\n", argv[0]);
    CHAR file_name[256];
    fprintf(stderr,"enter LAS input file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
	lasreadopener.set_file_name(file_name);
	
	//lasreadopener.set_file_name("D:\\oifii-org\\httpdocs\\ns-org\\nsd\\bs\\Petawawa\\PRF_2016_C2__297231_5092707\\slice-1.las");
	//lasreadopener.set_file_name("c:\\oifii-org\\httpdocs\\ns-org\\nsd\\bs\\Petawawa\\PRF_2016_C2__297231_5092707\\slice-1.las");
	//lasreadopener.set_file_name("c:\\oifii-org\\httpdocs\\ns-org\\nsd\\bs\\Petawawa\\f_300_5094_petawawa_allhits_cgvd28(lasopt).laz");
	//lasreadopener.set_file_name("c:\\oifii-org\\httpdocs\\ns-org\\nsd\\bs\\Petawawa\\f_300_5094_petawawa_allhits_cgvd28.las");
	//lasreadopener.set_file_name("p:\\f_300_5094_petawawa_allhits_cgvd28.las");
	/*
	fprintf(stderr,"enter output file: "); fgets(file_name, 256, stdin);
    file_name[strlen(file_name)-1] = '\0';
    laswriteopener.set_file_name(file_name);
	*/
	fprintf(stderr, "enter input SHAPEFILE name: "); fgets(file_name, 256, stdin);
	file_name[strlen(file_name) - 1] = '\0';
	//shapefilename = "D:\\oifii-org\\httpdocs\\ns-org\\nsd\\bs\\Petawawa\\PRF_2016_C2__297231_5092707\\slice-1.shp";
	//shapefilename = "c:\\oifii-org\\httpdocs\\ns-org\\nsd\\bs\\Petawawa\\300_5094_petawawa_allhits_cgvd28_CHM_crowns(clipped).shp";
	//shapefilename = "p:\\300_5094_petawawa_allhits_cgvd28_CHM_crowns(clipped).shp";
	//shapefilename = "c:\\oifii-org\\httpdocs\\ns-org\\nsd\\bs\\Petawawa\\300_5094_petawawa_allhits_cgvd28_CHM_crowns.shp";
	//shapefilename = "p:\\300_5094_petawawa_allhits_cgvd28_CHM_crowns.shp";
	shapefilename = file_name;
	shapefilelayername = getfilenameonly(shapefilename);

	//outputdirname = getpathnameonly(lasreadopener.get_file_name());
	outputdirname = getpathonly(lasreadopener.get_file_name());
	/*
	if (outputdirname.empty()) outputdirname = getpathonly(argv[0]);
	*/
	if (outputdirname.empty())
	{
		outputdirname = getcurrentdirectory();
	}


	CHAR fieldindex_name[256];
	fprintf(stderr, "enter FIELDINDEX name: "); fgets(fieldindex_name, 256, stdin);
	fieldindex_name[strlen(fieldindex_name) - 1] = '\0';
	fieldindexname = fieldindex_name;
  }  
  else
  {
    for (i = 1; i < argc; i++)
    {
      if (argv[i][0] == '–') argv[i][0] = '-';
    }
  }

  //defaults
  if (fieldindexname.empty()) fieldindexname = "Object_ID";


  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '\0')
    {
      continue;
    }
    else if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0)
    {
	  fprintf(stderr, "LASapps lasclip version 0.1\n");
	  fprintf(stderr, "based on LAStools' LASlib version %d\n", LAS_TOOLS_VERSION);
      usage();
    }
    else if (strcmp(argv[i],"-v") == 0 || strcmp(argv[i],"-verbose") == 0)
    {
      verbose = true;
    }
    else if (strcmp(argv[i],"-version") == 0)
    {
		fprintf(stderr, "LASapps lasclip version 0.1\n");
		fprintf(stderr, "based on LAStools' LASlib version %d\n", LAS_TOOLS_VERSION);
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
		lasreadopener.add_file_name(argv[i]);
		argv[i][0] = '\0';
		outputdirname = getpathonly(lasreadopener.get_file_name());
		if (outputdirname.empty())
		{
			outputdirname = getcurrentdirectory();
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
		shapefilename = argv[i];
		argv[i][0] = '\0';
		shapefilelayername = getfilenameonly(shapefilename);
	}
	else if (strcmp(argv[i], "-odir") == 0)
	{
		if ((i + 1) >= argc)
		{
			fprintf(stderr, "ERROR: '%s' needs 1 argument: string\n", argv[i]);
			usage(true);
		}
		i++;
		outputdirname = argv[i];
		argv[i][0] = '\0';
		if (PathIsRelative(outputdirname.c_str()))
		{
			/*
			outputdirname = getpathonly(lasreadopener.get_file_name()) + "\\" + outputdirname;
			*/
			string laspathonly = getpathonly(lasreadopener.get_file_name());
			if (laspathonly.empty())
			{
				laspathonly = getcurrentdirectory();
			}
			outputdirname = laspathonly + "\\" + outputdirname;
		}
	}
	else if (strcmp(argv[i], "-fieldindexname") == 0)
	{
		if ((i + 1) >= argc)
		{
			fprintf(stderr, "ERROR: '%s' needs 1 argument: string\n", argv[i]);
			usage(true);
		}
		i++;
		fieldindexname = argv[i];
		argv[i][0] = '\0';
	}
	/*
	else if ((argv[i][0] != '-') && (lasreadopener.get_file_name_number() == 0))
    {
      lasreadopener.add_file_name(argv[i]);
      argv[i][0] = '\0';
    }
	*/
    else
    {
      fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
      usage(true);
    }
  }
  /*
  else
  {
	  byebye(true, argc==1);
  }
  */

  // check input
  if (!lasreadopener.active())
  {
    fprintf(stderr,"ERROR: no input specified\n");
    byebye(true, argc == 1);
  }

  //////////////////////////////////////////
  // possibly loop over multiple input files
  //////////////////////////////////////////
  while (lasreadopener.active())
  {
    if (verbose) start_time = taketime();

	/////////////////
    // open lasreader
	/////////////////
    LASreader* lasreader = lasreadopener.open();
    if (lasreader == 0)
    {
      fprintf(stderr, "ERROR: could not open lasreader\n");
      byebye(true, argc==1);
    }

	///////////////////////////////////
    // (maybe) open laswaveform13reader
	///////////////////////////////////
    LASwaveform13reader* laswaveform13reader = lasreadopener.open_waveform13(&lasreader->header);

	//////////////////////////////
    // get a pointer to the header
	//////////////////////////////

    LASheader* header = &(lasreader->header);

	///////////////////////////////
	//open shapefile using GDAL/OGR
	///////////////////////////////
	GDALAllRegister();

	GDALDataset* poDS;
	poDS = (GDALDataset*)GDALOpenEx(shapefilename.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (poDS == NULL)
	{
		fprintf(stderr, "Error: Open shapefile failed.\n");
		byebye(true, argc == 1);
	}

	OGRLayer* poLayer;
	//poLayer = poDS->GetLayerByName("slice-1");
	//poLayer = poDS->GetLayerByName("300_5094_petawawa_allhits_cgvd28_CHM_crowns(clipped)");
	//poLayer = poDS->GetLayerByName("300_5094_petawawa_allhits_cgvd28_CHM_crowns");
	poLayer = poDS->GetLayerByName(shapefilelayername.c_str());
	
	if (poLayer==NULL)
	{
		fprintf(stderr, "Error: Get shapefile layer by name failed.\n");
		GDALClose(poDS);
		byebye(true, argc == 1);
	}

	I64 numberoffeatures = poLayer->GetFeatureCount();



#ifdef _WIN32
	if (verbose) fprintf(stderr, "processing %I64d points against %I64d features.\n", lasreader->npoints, numberoffeatures);
#else
	if (verbose) fprintf(stderr, "processing %lld points against %lld features.\n", lasreader->npoints, numberoffeatures);
#endif


	///////////////////////////////////////////////
	//create output folder name for micro las files
	///////////////////////////////////////////////
	if (!direxists(outputdirname.c_str()))
	{
		if (_mkdir(outputdirname.c_str()) == -1)
		{
			fprintf(stderr, "ERROR: can't create output dir\n");
			byebye(true, argc == 1);
		}
	}

	// prepare the header for the surviving points
	strncpy(lasreader->header.system_identifier, "LASapps", 32);
	lasreader->header.system_identifier[31] = '\0';
	char temp[64];
	//sprintf(temp, "lasclip (version %d)", LAS_TOOLS_VERSION);
	sprintf(temp, "lasclip (version 0.1)");
	strncpy(lasreader->header.generating_software, temp, 32);
	lasreader->header.generating_software[31] = '\0';

	OGRPoint myOGRPoint;
	OGRSpatialReference mySRS;
	//mySRS.SetWellKnownGeogCS("EPSG:26918");
	//mySRS.SetWellKnownGeogCS("EPSG:2959");
	//myOGRPoint.assignSpatialReference(&mySRS);
	LASpoint* pLASpoint = new LASpoint;
	// if the point needs to be copied set up the data fields
	pLASpoint->init(&lasreader->header, lasreader->header.point_data_format, lasreader->header.point_data_record_length);

	//////////////////////////////////////////
	//load all LAS input file points in memory
	//////////////////////////////////////////
	if (dynamic_cast <LASreaderLASRAM*>(lasreader)) 
	{
		if ((dynamic_cast <LASreaderLASRAM*>(lasreader))->read_allpoints()==FALSE)
		{
			fprintf(stderr, "ERROR: LASreaderLASRAM read_allpoints() failed, not enough memory.\n");
			byebye(true, argc == 1);
		}
	}
	/////////////////////////////
	//browse through each polygon
	/////////////////////////////
	OGRFeature *poFeature;
	poLayer->ResetReading();
	//while (lasreader->read_point() && ((poFeature = poLayer->GetNextFeature()) != NULL))
	I64 ii = 0;
	while ( (poFeature = poLayer->GetNextFeature()) != NULL)
	{
		OGRGeometry *poGeometry;
		poGeometry = poFeature->GetGeometryRef();

		if (poGeometry != NULL
			&& (wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon))
		{
			OGREnvelope myOGREnvelope;
			poGeometry->getEnvelope(&myOGREnvelope);
			//OGRPolygon* poPolygon = (OGRPolygon*)poGeometry;
			if (verbose && false) fprintf(stderr, "found polygon\n");

			bool isnumericcrownid = true;
			I64 crownid = ii;
			std::string crownidstring = "";

			OGRFeatureDefn* poFDefn = poLayer->GetLayerDefn();
			if (poFDefn==NULL)
			{
				fprintf(stderr, "ERROR: calling OGR GetLayerDefn().\n");
				byebye(true, argc == 1);
			}
			//int iField = poFDefn->GetFieldIndex("Object_ID");
			int iField = poFDefn->GetFieldIndex(fieldindexname.c_str());
			if (iField==-1)
			{
				fprintf(stderr, "WARNING: fieldindexname not found\n");
				fprintf(stderr, "WARNING: will name microlas files using default index\n");

				//byebye(true, argc == 1);
			}
			else
			{
				//improved to suit simon's need as well as rachel's need
				OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
				if (poFieldDefn->GetType() == OFTInteger)
				{
					crownid = poFeature->GetFieldAsInteger(iField);
				}
				else if (poFieldDefn->GetType() == OFTInteger64)
				{
					crownid = poFeature->GetFieldAsInteger64(iField);
				}
				else if (poFieldDefn->GetType() == OFTString)
				{
					crownidstring = poFeature->GetFieldAsString(iField);
					isnumericcrownid = false;
				}
				else
				{
					fprintf(stderr, "WARNING: fieldindexname not of type OFTInteger, OFTInteger64 nor OFTString\n");
					fprintf(stderr, "WARNING: will name microlas files using default index\n");

					//byebye(true, argc == 1);
				}
			}
			
			if (verbose && false) fprintf(stderr, "%I64d features remaining\n", numberoffeatures-ii);

			if (!laswriteopener.active())
			{
				// create name from input name
				char pchar[64];
				sprintf(pchar, "%I64d", crownid);
				std::string microlasfilename;
				if (isnumericcrownid)
				{
					//microlasfilename = lasfilenamewithoutextension + "\\" + pchar + ".las";
					microlasfilename = outputdirname + "\\" + getfilenameonly(lasreadopener.get_file_name_only()) + "_" + pchar + ".las";
				}
				else
				{
					//microlasfilename = lasfilenamewithoutextension + "\\" + crownidstring + ".las";
					microlasfilename = outputdirname + "\\" + getfilenameonly(lasreadopener.get_file_name_only()) + "_" + crownidstring + ".las";
				}
				laswriteopener.set_file_name(microlasfilename.c_str());
			}
			else
			{
				fprintf(stderr, "ERROR: laswriteopener is active.\n");
				byebye(true, argc == 1);
			}

			/////////////////
			// open laswriter
			/////////////////
			LASwriter* laswriter = laswriteopener.open(&lasreader->header);

			if (laswriter == 0)
			{
				fprintf(stderr, "ERROR: could not open laswriter\n");
				byebye(true, argc == 1);
			}

			/*
			OGRPoint myOGRPoint;
			OGRSpatialReference mySRS;
			//mySRS.SetWellKnownGeogCS("EPSG:26918");
			//mySRS.SetWellKnownGeogCS("EPSG:2959");
			//myOGRPoint.assignSpatialReference(&mySRS);
			LASpoint* pLASpoint = new LASpoint;
			// if the point needs to be copied set up the data fields
			pLASpoint->init(&lasreader->header, lasreader->header.point_data_format, lasreader->header.point_data_record_length);
			*/

			lasreader->seek(0);
			lasreader->inside_none();
			lasreader->inside_rectangle(myOGREnvelope.MinX, myOGREnvelope.MinY, myOGREnvelope.MaxX, myOGREnvelope.MaxY);
			if (dynamic_cast <LASreaderLASRAM*>(lasreader))
			{
				LASreaderLASRAM* lasreaderlasram = dynamic_cast <LASreaderLASRAM*>(lasreader);
				while (lasreaderlasram->read_point())
				{
					myOGRPoint.setX(lasreaderlasram->ppoint->get_x());
					myOGRPoint.setY(lasreaderlasram->ppoint->get_y());

					if (myOGRPoint.Within(poGeometry))
					{
						//fprintf(stdout, "keeping point\n");
						/* //try to avoid copying ppoint again
						*pLASpoint = *(lasreaderlasram->ppoint);
						laswriter->write_point(pLASpoint);
						laswriter->update_inventory(pLASpoint);
						*/
						laswriter->write_point(lasreaderlasram->ppoint);
						laswriter->update_inventory(lasreaderlasram->ppoint);
					}
				}
			}
			else
			{
				while (lasreader->read_point())
				{
					myOGRPoint.setX(lasreader->point.get_x());
					myOGRPoint.setY(lasreader->point.get_y());

					if (myOGRPoint.Within(poGeometry))
					{
						//fprintf(stdout, "keeping point\n");
						*pLASpoint = lasreader->point;
						laswriter->write_point(pLASpoint);
						laswriter->update_inventory(pLASpoint);
					}
				}
			}

			/*
			delete pLASpoint;
			*/
			laswriter->update_header(&lasreader->header, TRUE);
			laswriter->close();
			delete laswriter;

			laswriteopener.set_file_name(0);

			//fprintf(stdout, "%f,%f,%f\n", poPoint->getX() - lasreader->point.get_x(), poPoint->getY() - lasreader->point.get_y(), poPoint->getZ() - lasreader->point.get_z());
			//fprintf(stdout, "%f,%f,%f\n", lasreader->point.get_x(), lasreader->point.get_y(), lasreader->point.get_z());
			if (verbose)
				term_progress(std::cout, (ii + 1) / static_cast<double>(numberoffeatures));

			ii++; //valid polygon counter
		}
		else
		{
			fprintf(stderr, "WARNING: not a polygon geometry, ignoring this geometry\n");
		}

		OGRFeature::DestroyFeature(poFeature);

	}

	delete pLASpoint;

	//lasreader->inside_none();
#ifdef _WIN32
	//if (verbose) fprintf(stderr, "clipping %I64d points of '%s' against %I64d polygons took %g sec.\n", lasreader->p_count, lasreadopener.get_file_name(), ii, taketime() - start_time);
	if (verbose) fprintf(stderr,"clipping %I64d points of '%s' against %I64d polygons took %g sec.\n", lasreader->npoints, lasreadopener.get_file_name(), ii, taketime()-start_time);
#else
    if (verbose) fprintf(stderr,"comparing %lld points of '%s' against %lld polygons took %g sec.\n", lasreader->npoints, lasreadopener.get_file_name(), ii, taketime()-start_time);
#endif

    // close the reader
    lasreader->close();
    delete lasreader;

    // (maybe) close the waveform reader
    if (laswaveform13reader)
    {
      laswaveform13reader->close();
      delete laswaveform13reader;
    }

	/*
    // close the files

    if (file_out != stdout) fclose(file_out);
	*/
	GDALClose(poDS);

  }


  byebye(false, argc==1);

  return 0;
}
