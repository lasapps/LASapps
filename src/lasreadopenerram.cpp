/*
===============================================================================

FILE:  lasreadopenerram.cpp

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

#include "lasreadopenerram.h" 

#include "lasreader.hpp"

#include "lasindex.hpp"
#include "lasfilter.hpp"
#include "lastransform.hpp"

#include "lasreader_las.hpp"
#include "lasreader_bin.hpp"
#include "lasreader_shp.hpp"
#include "lasreader_qfit.hpp"
#include "lasreader_asc.hpp"
#include "lasreader_bil.hpp"
#include "lasreader_dtm.hpp"
#include "lasreader_txt.hpp"
#include "lasreadermerged.hpp"
#include "lasreaderbuffered.hpp"
#include "lasreaderpipeon.hpp"

#include "lasreaderlasram.h"

//this function should be an integral copy of the base class open member function, 
//LASreadOpener::open(const CHAR* other_file_name, BOOL reset_after_other),
//except it should create a LASreaderLASRAM object instead of a LASreaderLAS object.
LASreader* LASreadOpenerRAM::open(const CHAR* other_file_name, BOOL reset_after_other)
{
	if (filter) filter->reset();
	if (transform) transform->reset();

	if (file_names || other_file_name)
	{
		use_stdin = FALSE;
		if (file_name_current == file_name_number && other_file_name == 0) return 0;
		if ((file_name_number > 1) && merged)
		{
			LASreaderMerged* lasreadermerged = new LASreaderMerged();
			lasreadermerged->set_scale_factor(scale_factor);
			lasreadermerged->set_offset(offset);
			lasreadermerged->set_parse_string(parse_string);
			lasreadermerged->set_skip_lines(skip_lines);
			lasreadermerged->set_populate_header(populate_header);
			lasreadermerged->set_keep_lastiling(keep_lastiling);
			lasreadermerged->set_translate_intensity(translate_intensity);
			lasreadermerged->set_scale_intensity(scale_intensity);
			lasreadermerged->set_translate_scan_angle(translate_scan_angle);
			lasreadermerged->set_scale_scan_angle(scale_scan_angle);
			lasreadermerged->set_io_ibuffer_size(io_ibuffer_size);
			for (file_name_current = 0; file_name_current < file_name_number; file_name_current++) lasreadermerged->add_file_name(file_names[file_name_current]);
			if (!lasreadermerged->open())
			{
				fprintf(stderr, "ERROR: cannot open lasreadermerged with %d file names\n", file_name_number);
				delete lasreadermerged;
				return 0;
			}
			if (files_are_flightlines) lasreadermerged->set_files_are_flightlines(TRUE);
			if (apply_file_source_ID) lasreadermerged->set_apply_file_source_ID(TRUE);
			if (filter) lasreadermerged->set_filter(filter);
			if (transform) lasreadermerged->set_transform(transform);
			if (inside_tile) lasreadermerged->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
			if (inside_circle) lasreadermerged->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
			if (inside_rectangle) lasreadermerged->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
			if (pipe_on)
			{
				LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
				if (!lasreaderpipeon->open(lasreadermerged))
				{
					fprintf(stderr, "ERROR: cannot open lasreaderpipeon with lasreadermerged\n");
					delete lasreaderpipeon;
					return 0;
				}
				return lasreaderpipeon;
			}
			else
			{
				return lasreadermerged;
			}
		}
		else if ((buffer_size > 0) && ((file_name_number > 1) || (neighbor_file_name_number > 0)))
		{
			U32 i;
			if (other_file_name)
			{
				file_name = other_file_name;
				if (reset_after_other)
				{
					file_name_current = 0;
				}
			}
			else
			{
				file_name = file_names[file_name_current];
				file_name_current++;
			}
			LASreaderBuffered* lasreaderbuffered = new LASreaderBuffered();
			lasreaderbuffered->set_buffer_size(buffer_size);
			lasreaderbuffered->set_scale_factor(scale_factor);
			lasreaderbuffered->set_offset(offset);
			lasreaderbuffered->set_parse_string(parse_string);
			lasreaderbuffered->set_skip_lines(skip_lines);
			lasreaderbuffered->set_populate_header(populate_header);
			lasreaderbuffered->set_translate_intensity(translate_intensity);
			lasreaderbuffered->set_scale_intensity(scale_intensity);
			lasreaderbuffered->set_translate_scan_angle(translate_scan_angle);
			lasreaderbuffered->set_scale_scan_angle(scale_scan_angle);
			lasreaderbuffered->set_file_name(file_name);
			for (i = 0; i < file_name_number; i++)
			{
				if (file_name != file_names[i])
				{
					lasreaderbuffered->add_neighbor_file_name(file_names[i]);
				}
			}
			for (i = 0; i < neighbor_file_name_number; i++)
			{
				if (strcmp(file_name, neighbor_file_names[i]))
				{
					lasreaderbuffered->add_neighbor_file_name(neighbor_file_names[i]);
				}
			}
			if (filter) lasreaderbuffered->set_filter(filter);
			if (transform) lasreaderbuffered->set_transform(transform);
			if (!lasreaderbuffered->open())
			{
				fprintf(stderr, "ERROR: cannot open lasreaderbuffered with %d file names\n", file_name_number + neighbor_file_name_number);
				delete lasreaderbuffered;
				return 0;
			}
			if (inside_tile) lasreaderbuffered->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
			if (inside_circle) lasreaderbuffered->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
			if (inside_rectangle) lasreaderbuffered->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
			if (pipe_on)
			{
				LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
				if (!lasreaderpipeon->open(lasreaderbuffered))
				{
					fprintf(stderr, "ERROR: cannot open lasreaderpipeon with lasreaderbuffered\n");
					delete lasreaderpipeon;
					return 0;
				}
				return lasreaderpipeon;
			}
			else
			{
				return lasreaderbuffered;
			}
		}
		else
		{
			if (other_file_name)
			{
				file_name = other_file_name;
				if (reset_after_other)
				{
					file_name_current = 0;
				}
			}
			else
			{
				file_name = file_names[file_name_current];
				file_name_current++;
			}
			if (files_are_flightlines)
			{
				transform->setPointSource(file_name_current);
			}
			if (strstr(file_name, ".las") || strstr(file_name, ".laz") || strstr(file_name, ".LAS") || strstr(file_name, ".LAZ"))
			{
				LASreaderLAS* lasreaderlas;
				if (scale_factor == 0 && offset == 0)
				{
					if (auto_reoffset)
						lasreaderlas = new LASreaderLASreoffset();
					else
						//spi, begin
						//lasreaderlas = new LASreaderLAS();
						lasreaderlas = new LASreaderLASRAM();
						//spi, end
				}
				else if (scale_factor != 0 && offset == 0)
				{
					if (auto_reoffset)
						lasreaderlas = new LASreaderLASrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2]);
					else
						lasreaderlas = new LASreaderLASrescale(scale_factor[0], scale_factor[1], scale_factor[2]);
				}
				else if (scale_factor == 0 && offset != 0)
					lasreaderlas = new LASreaderLASreoffset(offset[0], offset[1], offset[2]);
				else
					lasreaderlas = new LASreaderLASrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
				if (!lasreaderlas->open(file_name, io_ibuffer_size))
				{
					fprintf(stderr, "ERROR: cannot open lasreaderlas with file name '%s'\n", file_name);
					delete lasreaderlas;
					return 0;
				}
				LASindex* index = new LASindex();
				if (index->read(file_name))
					lasreaderlas->set_index(index);
				else
					delete index;
				if (files_are_flightlines)
				{
					lasreaderlas->header.file_source_ID = file_name_current;
				}
				else if (apply_file_source_ID)
				{
					transform->setPointSource(lasreaderlas->header.file_source_ID);
				}
				if (filter) lasreaderlas->set_filter(filter);
				if (transform) lasreaderlas->set_transform(transform);
				if (inside_tile) lasreaderlas->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
				if (inside_circle) lasreaderlas->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
				if (inside_rectangle) lasreaderlas->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
				if (pipe_on)
				{
					LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
					if (!lasreaderpipeon->open(lasreaderlas))
					{
						fprintf(stderr, "ERROR: cannot open lasreaderpipeon with lasreaderlas\n");
						delete lasreaderpipeon;
						return 0;
					}
					return lasreaderpipeon;
				}
				else
				{
					return lasreaderlas;
				}
			}
			else if (strstr(file_name, ".bin") || strstr(file_name, ".BIN"))
			{
				LASreaderBIN* lasreaderbin;
				if (scale_factor == 0 && offset == 0)
					lasreaderbin = new LASreaderBIN();
				else if (scale_factor != 0 && offset == 0)
					lasreaderbin = new LASreaderBINrescale(scale_factor[0], scale_factor[1], scale_factor[2]);
				else if (scale_factor == 0 && offset != 0)
					lasreaderbin = new LASreaderBINreoffset(offset[0], offset[1], offset[2]);
				else
					lasreaderbin = new LASreaderBINrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
				if (!lasreaderbin->open(file_name))
				{
					fprintf(stderr, "ERROR: cannot open lasreaderbin with file name '%s'\n", file_name);
					delete lasreaderbin;
					return 0;
				}
				LASindex* index = new LASindex();
				if (index->read(file_name))
					lasreaderbin->set_index(index);
				else
					delete index;
				if (files_are_flightlines) lasreaderbin->header.file_source_ID = file_name_current;
				if (filter) lasreaderbin->set_filter(filter);
				if (transform) lasreaderbin->set_transform(transform);
				if (inside_tile) lasreaderbin->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
				if (inside_circle) lasreaderbin->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
				if (inside_rectangle) lasreaderbin->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
				if (pipe_on)
				{
					LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
					if (!lasreaderpipeon->open(lasreaderbin))
					{
						fprintf(stderr, "ERROR: cannot open lasreaderpipeon with lasreaderbin\n");
						delete lasreaderpipeon;
						return 0;
					}
					return lasreaderpipeon;
				}
				else
				{
					return lasreaderbin;
				}
			}
			else if (strstr(file_name, ".shp") || strstr(file_name, ".SHP"))
			{
				LASreaderSHP* lasreadershp;
				if (scale_factor == 0 && offset == 0)
					lasreadershp = new LASreaderSHP();
				else if (scale_factor != 0 && offset == 0)
					lasreadershp = new LASreaderSHPrescale(scale_factor[0], scale_factor[1], scale_factor[2]);
				else if (scale_factor == 0 && offset != 0)
					lasreadershp = new LASreaderSHPreoffset(offset[0], offset[1], offset[2]);
				else
					lasreadershp = new LASreaderSHPrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
				if (!lasreadershp->open(file_name))
				{
					fprintf(stderr, "ERROR: cannot open lasreadershp with file name '%s'\n", file_name);
					delete lasreadershp;
					return 0;
				}
				if (files_are_flightlines) lasreadershp->header.file_source_ID = file_name_current;
				if (filter) lasreadershp->set_filter(filter);
				if (transform) lasreadershp->set_transform(transform);
				if (inside_tile) lasreadershp->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
				if (inside_circle) lasreadershp->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
				if (inside_rectangle) lasreadershp->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
				if (pipe_on)
				{
					LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
					if (!lasreaderpipeon->open(lasreadershp))
					{
						fprintf(stderr, "ERROR: cannot open lasreaderpipeon with lasreadershp\n");
						delete lasreaderpipeon;
						return 0;
					}
					return lasreaderpipeon;
				}
				else
				{
					return lasreadershp;
				}
			}
			else if (strstr(file_name, ".qi") || strstr(file_name, ".QI"))
			{
				LASreaderQFIT* lasreaderqfit;
				if (scale_factor == 0 && offset == 0)
					lasreaderqfit = new LASreaderQFIT();
				else if (scale_factor != 0 && offset == 0)
					lasreaderqfit = new LASreaderQFITrescale(scale_factor[0], scale_factor[1], scale_factor[2]);
				else if (scale_factor == 0 && offset != 0)
					lasreaderqfit = new LASreaderQFITreoffset(offset[0], offset[1], offset[2]);
				else
					lasreaderqfit = new LASreaderQFITrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
				if (!lasreaderqfit->open(file_name))
				{
					fprintf(stderr, "ERROR: cannot open lasreaderqfit with file name '%s'\n", file_name);
					delete lasreaderqfit;
					return 0;
				}
				LASindex* index = new LASindex();
				if (index->read(file_name))
					lasreaderqfit->set_index(index);
				else
					delete index;
				if (files_are_flightlines) lasreaderqfit->header.file_source_ID = file_name_current;
				if (filter) lasreaderqfit->set_filter(filter);
				if (transform) lasreaderqfit->set_transform(transform);
				if (inside_tile) lasreaderqfit->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
				if (inside_circle) lasreaderqfit->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
				if (inside_rectangle) lasreaderqfit->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
				if (pipe_on)
				{
					LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
					if (!lasreaderpipeon->open(lasreaderqfit))
					{
						fprintf(stderr, "ERROR: cannot open lasreaderpipeon with lasreaderqfit\n");
						delete lasreaderpipeon;
						return 0;
					}
					return lasreaderpipeon;
				}
				else
				{
					return lasreaderqfit;
				}
			}
			else if (strstr(file_name, ".asc") || strstr(file_name, ".ASC"))
			{
				LASreaderASC* lasreaderasc;
				if (scale_factor == 0 && offset == 0)
					lasreaderasc = new LASreaderASC();
				else if (scale_factor != 0 && offset == 0)
					lasreaderasc = new LASreaderASCrescale(scale_factor[0], scale_factor[1], scale_factor[2]);
				else if (scale_factor == 0 && offset != 0)
					lasreaderasc = new LASreaderASCreoffset(offset[0], offset[1], offset[2]);
				else
					lasreaderasc = new LASreaderASCrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
				if (!lasreaderasc->open(file_name, comma_not_point))
				{
					fprintf(stderr, "ERROR: cannot open lasreaderasc with file name '%s'\n", file_name);
					delete lasreaderasc;
					return 0;
				}
				if (files_are_flightlines) lasreaderasc->header.file_source_ID = file_name_current;
				if (filter) lasreaderasc->set_filter(filter);
				if (transform) lasreaderasc->set_transform(transform);
				if (inside_tile) lasreaderasc->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
				if (inside_circle) lasreaderasc->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
				if (inside_rectangle) lasreaderasc->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
				if (pipe_on)
				{
					LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
					if (!lasreaderpipeon->open(lasreaderasc))
					{
						fprintf(stderr, "ERROR: cannot open lasreaderpipeon with lasreaderasc\n");
						delete lasreaderpipeon;
						return 0;
					}
					return lasreaderpipeon;
				}
				else
				{
					return lasreaderasc;
				}
			}
			else if (strstr(file_name, ".bil") || strstr(file_name, ".BIL"))
			{
				LASreaderBIL* lasreaderbil;
				if (scale_factor == 0 && offset == 0)
					lasreaderbil = new LASreaderBIL();
				else if (scale_factor != 0 && offset == 0)
					lasreaderbil = new LASreaderBILrescale(scale_factor[0], scale_factor[1], scale_factor[2]);
				else if (scale_factor == 0 && offset != 0)
					lasreaderbil = new LASreaderBILreoffset(offset[0], offset[1], offset[2]);
				else
					lasreaderbil = new LASreaderBILrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
				if (!lasreaderbil->open(file_name))
				{
					fprintf(stderr, "ERROR: cannot open lasreaderbil with file name '%s'\n", file_name);
					delete lasreaderbil;
					return 0;
				}
				if (files_are_flightlines) lasreaderbil->header.file_source_ID = file_name_current;
				if (filter) lasreaderbil->set_filter(filter);
				if (transform) lasreaderbil->set_transform(transform);
				if (inside_tile) lasreaderbil->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
				if (inside_circle) lasreaderbil->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
				if (inside_rectangle) lasreaderbil->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
				if (pipe_on)
				{
					LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
					if (!lasreaderpipeon->open(lasreaderbil))
					{
						fprintf(stderr, "ERROR: cannot open lasreaderpipeon with lasreaderbil\n");
						delete lasreaderpipeon;
						return 0;
					}
					return lasreaderpipeon;
				}
				else
				{
					return lasreaderbil;
				}
			}
			else if (strstr(file_name, ".dtm") || strstr(file_name, ".DTM"))
			{
				LASreaderDTM* lasreaderdtm;
				if (scale_factor == 0 && offset == 0)
					lasreaderdtm = new LASreaderDTM();
				else if (scale_factor != 0 && offset == 0)
					lasreaderdtm = new LASreaderDTMrescale(scale_factor[0], scale_factor[1], scale_factor[2]);
				else if (scale_factor == 0 && offset != 0)
					lasreaderdtm = new LASreaderDTMreoffset(offset[0], offset[1], offset[2]);
				else
					lasreaderdtm = new LASreaderDTMrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
				if (!lasreaderdtm->open(file_name))
				{
					fprintf(stderr, "ERROR: cannot open lasreaderdtm with file name '%s'\n", file_name);
					delete lasreaderdtm;
					return 0;
				}
				if (files_are_flightlines) lasreaderdtm->header.file_source_ID = file_name_current;
				if (filter) lasreaderdtm->set_filter(filter);
				if (transform) lasreaderdtm->set_transform(transform);
				if (inside_tile) lasreaderdtm->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
				if (inside_circle) lasreaderdtm->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
				if (inside_rectangle) lasreaderdtm->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
				if (pipe_on)
				{
					LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
					if (!lasreaderpipeon->open(lasreaderdtm))
					{
						fprintf(stderr, "ERROR: cannot open lasreaderpipeon with lasreaderdtm\n");
						delete lasreaderpipeon;
						return 0;
					}
					return lasreaderpipeon;
				}
				else
				{
					return lasreaderdtm;
				}
			}
			else
			{
				LASreaderTXT* lasreadertxt = new LASreaderTXT();
				if (ipts) lasreadertxt->set_pts(TRUE);
				else if (iptx) lasreadertxt->set_ptx(TRUE);
				if (translate_intensity != 0.0f) lasreadertxt->set_translate_intensity(translate_intensity);
				if (scale_intensity != 1.0f) lasreadertxt->set_scale_intensity(scale_intensity);
				if (translate_scan_angle != 0.0f) lasreadertxt->set_translate_scan_angle(translate_scan_angle);
				if (scale_scan_angle != 1.0f) lasreadertxt->set_scale_scan_angle(scale_scan_angle);
				lasreadertxt->set_scale_factor(scale_factor);
				lasreadertxt->set_offset(offset);
				if (number_attributes)
				{
					for (I32 i = 0; i < number_attributes; i++)
					{
						lasreadertxt->add_attribute(attribute_data_types[i], attribute_names[i], attribute_descriptions[i], attribute_scales[i], attribute_offsets[i], attribute_pre_scales[i], attribute_pre_offsets[i]);
					}
				}
				if (!lasreadertxt->open(file_name, parse_string, skip_lines, populate_header))
				{
					fprintf(stderr, "ERROR: cannot open lasreadertxt with file name '%s'\n", file_name);
					delete lasreadertxt;
					return 0;
				}
				if (files_are_flightlines) lasreadertxt->header.file_source_ID = file_name_current;
				if (filter) lasreadertxt->set_filter(filter);
				if (transform) lasreadertxt->set_transform(transform);
				if (inside_tile) lasreadertxt->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
				if (inside_circle) lasreadertxt->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
				if (inside_rectangle) lasreadertxt->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
				if (pipe_on)
				{
					LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
					if (!lasreaderpipeon->open(lasreadertxt))
					{
						fprintf(stderr, "ERROR: cannot open lasreaderpipeon with lasreadertxt\n");
						delete lasreaderpipeon;
						return 0;
					}
					return lasreaderpipeon;
				}
				else
				{
					return lasreadertxt;
				}
			}
		}
	}
	else if (use_stdin)
	{
		use_stdin = FALSE; populate_header = TRUE;
		if (itxt)
		{
			LASreaderTXT* lasreadertxt = new LASreaderTXT();
			if (ipts) lasreadertxt->set_pts(TRUE);
			else if (iptx) lasreadertxt->set_ptx(TRUE);
			if (translate_intensity != 0.0f) lasreadertxt->set_translate_intensity(translate_intensity);
			if (scale_intensity != 1.0f) lasreadertxt->set_scale_intensity(scale_intensity);
			if (translate_scan_angle != 0.0f) lasreadertxt->set_translate_scan_angle(translate_scan_angle);
			if (scale_scan_angle != 1.0f) lasreadertxt->set_scale_scan_angle(scale_scan_angle);
			lasreadertxt->set_scale_factor(scale_factor);
			lasreadertxt->set_offset(offset);
			if (number_attributes)
			{
				for (I32 i = 0; i < number_attributes; i++)
				{
					lasreadertxt->add_attribute(attribute_data_types[i], attribute_names[i], attribute_descriptions[i], attribute_scales[i], attribute_offsets[i], attribute_pre_scales[i], attribute_pre_offsets[i]);
				}
			}
			if (!lasreadertxt->open(stdin, 0, parse_string, skip_lines, FALSE))
			{
				fprintf(stderr, "ERROR: cannot open lasreadertxt with file name '%s'\n", file_name);
				delete lasreadertxt;
				return 0;
			}
			if (files_are_flightlines) lasreadertxt->header.file_source_ID = file_name_current;
			if (filter) lasreadertxt->set_filter(filter);
			if (transform) lasreadertxt->set_transform(transform);
			if (inside_tile) lasreadertxt->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
			if (inside_circle) lasreadertxt->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
			if (inside_rectangle) lasreadertxt->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
			if (pipe_on)
			{
				LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
				if (!lasreaderpipeon->open(lasreadertxt))
				{
					fprintf(stderr, "ERROR: cannot open lasreaderpipeon with lasreadertxt\n");
					delete lasreaderpipeon;
					return 0;
				}
				return lasreaderpipeon;
			}
			else
			{
				return lasreadertxt;
			}
		}
		else
		{
			LASreaderLAS* lasreaderlas;
			if (scale_factor == 0 && offset == 0)
				lasreaderlas = new LASreaderLAS();
			else if (scale_factor != 0 && offset == 0)
				lasreaderlas = new LASreaderLASrescale(scale_factor[0], scale_factor[1], scale_factor[2]);
			else if (scale_factor == 0 && offset != 0)
				lasreaderlas = new LASreaderLASreoffset(offset[0], offset[1], offset[2]);
			else
				lasreaderlas = new LASreaderLASrescalereoffset(scale_factor[0], scale_factor[1], scale_factor[2], offset[0], offset[1], offset[2]);
			if (!lasreaderlas->open(stdin))
			{
				fprintf(stderr, "ERROR: cannot open lasreaderlas from stdin \n");
				delete lasreaderlas;
				return 0;
			}
			if (filter) lasreaderlas->set_filter(filter);
			if (transform) lasreaderlas->set_transform(transform);
			if (inside_tile) lasreaderlas->inside_tile(inside_tile[0], inside_tile[1], inside_tile[2]);
			if (inside_circle) lasreaderlas->inside_circle(inside_circle[0], inside_circle[1], inside_circle[2]);
			if (inside_rectangle) lasreaderlas->inside_rectangle(inside_rectangle[0], inside_rectangle[1], inside_rectangle[2], inside_rectangle[3]);
			if (pipe_on)
			{
				LASreaderPipeOn* lasreaderpipeon = new LASreaderPipeOn();
				if (!lasreaderpipeon->open(lasreaderlas))
				{
					fprintf(stderr, "ERROR: cannot open lasreaderpipeon with lasreaderlas from stdin\n");
					delete lasreaderpipeon;
					return 0;
				}
				return lasreaderpipeon;
			}
			else
			{
				return lasreaderlas;
			}
		}
	}
	else
	{
		return 0;
	}
}
