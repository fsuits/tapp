// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#include "Filetypes/MzXML/MzXML_File.h"

namespace TAPP::Filetypes::MzXML
{
	/*** Constructors / Destructor **********************************************/

	// Default Constructor
	MzXML_File::MzXML_File(void)
	{
	}

	// Move Constructor
	MzXML_File::MzXML_File(MzXML_File&& other) : filename(std::move(other.filename)), scans(std::move(other.scans)), events(std::move(other.events))
	{
	}

	// Copy Constructor
	MzXML_File::MzXML_File(const MzXML_File& other) : filename(other.filename), scans(other.scans), events(other.events)
	{
		for (const auto& other_event : other.events)
		{
			auto this_event = events.find(other_event.first);

			this_event->second.spectral_scan	= &scans.find(other_event.second.spectral_scan->scan_id)->second;
			this_event->second.precursor_scan	= &scans.find(other_event.second.precursor_scan->scan_id)->second;
		}
	}

	// Destructor
	MzXML_File::~MzXML_File(void)
	{
	}

	/*** Operators **************************************************************/

	// Copy Assignment
	MzXML_File& MzXML_File::operator=(const MzXML_File& other)
	{
		filename	= other.filename;
		scans		= other.scans;
		events		= other.events;

		for (const auto& other_event : other.events)
		{
			auto this_event = events.find(other_event.first);

			this_event->second.spectral_scan = &scans.find(other_event.second.spectral_scan->scan_id)->second;
			this_event->second.precursor_scan = &scans.find(other_event.second.precursor_scan->scan_id)->second;
		}

		return *this;
	}

	// Move Assignment
	MzXML_File& MzXML_File::operator=(MzXML_File&& other)
	{
		filename.swap(other.filename);
		scans.swap(other.scans);
		events.swap(other.events);

		other.filename.clear();
		other.scans.clear();
		other.events.clear();

		return *this;
	}
}
