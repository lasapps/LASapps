/*
===============================================================================

FILE:  lasreaderlasram.cpp

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

21 April 2017 -- created in Benoit St-Onge's Lab at UQAM in Montreal

===============================================================================
*/


#include "lasreaderlasram.h"
#include "lasindex.hpp"

LASreaderLASRAM::LASreaderLASRAM()
{
}

LASreaderLASRAM::~LASreaderLASRAM()
{
	vector<LASpoint*>::iterator it;
	for (it = laspointvector.begin(); it != laspointvector.end(); it++)
	{
		delete (*it);
	}
	laspointvector.clear();
}

BOOL LASreaderLASRAM::open(const char* file_name, I32 io_buffer_size, BOOL peek_only)
{
	//open and read header
	BOOL bresult = LASreaderLAS::open(file_name, io_buffer_size, peek_only);
	if (bresult)
	{
		//return read_allpoints();
	}
	return bresult;
}

BOOL LASreaderLASRAM::read_allpoints()
{
	if (laspointvector.empty())
	{
		//read all points in RAM
		laspointvector.reserve(npoints);
		seek(0);//seek(6349736);
		while (LASreaderLAS::read_point_default())
		{
			LASpoint* plaspoint = new LASpoint;
			if (plaspoint)
			{
				// if the point needs to be copied set up the data fields
				if (plaspoint->init(&header, header.point_data_format, header.point_data_record_length))
				{
					*plaspoint = point;
					laspointvector.push_back(plaspoint);
				}
				else
				{
					fprintf(stderr, "ERROR: LASpoint init() failed, not enough memory.\n");
					return FALSE;
				}
			}
			else
			{
				fprintf(stderr, "ERROR: LASpoint new failed, not enough memory.\n");
				return FALSE;
			}
		}
		return TRUE;
	}
	return FALSE;
}

BOOL LASreaderLASRAM::seek(const I64 p_index)
{
	//return LASreaderLAS::seek(p_index);

	if (p_index < npoints)
	{
		p_count = p_index;
		return TRUE;
	}
	return FALSE;
}

BOOL LASreaderLASRAM::read_point_default()
{
	//return LASreaderLAS::read_point_default();

	if (p_count < laspointvector.size())
	{
		//point = *(laspointvector[p_count]);
		ppoint = laspointvector[p_count];
		p_count++;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL LASreaderLASRAM::read_point_inside_rectangle()
{
	while (read_point_default())
	{
		if (ppoint->inside_rectangle(r_min_x, r_min_y, r_max_x, r_max_y)) return TRUE;
	}
	return FALSE;
}

BOOL LASreaderLASRAM::read_point_inside_rectangle_indexed()
{
	while (index->seek_next((LASreader*)this))
	{
		if (read_point_default() && ppoint->inside_rectangle(r_min_x, r_min_y, r_max_x, r_max_y)) return TRUE;
	}
	return FALSE;
}
