/*
===============================================================================

FILE:  lasreaderlasram.h

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

#ifndef LAS_READER_LAS_RAM_H
#define LAS_READER_LAS_RAM_H

#include "lasreader_las.hpp"
#include <vector>

//class LASreaderLASRAM : public virtual LASreaderLAS
class LASreaderLASRAM : public LASreaderLAS
{
public:
	LASpoint* ppoint;
protected:
	vector<LASpoint*> laspointvector;

public:
	virtual BOOL open(const char* file_name, I32 io_buffer_size, BOOL peek_only);
	virtual BOOL read_allpoints();
	LASreaderLASRAM();
	virtual ~LASreaderLASRAM(); //virtual ~LASreaderLASRAM();
	virtual BOOL seek(const I64 p_index);

protected:
	virtual BOOL read_point_default();
	virtual BOOL read_point_inside_rectangle();
	virtual BOOL read_point_inside_rectangle_indexed();

};

#endif
