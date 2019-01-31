// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#pragma once

#include <unordered_map>

namespace TAPP::Filetypes::MzXML
{
	/*** Structs ****************************************************************/

	/*	Scan
		<summary>Represents a scan from the MzXML file.</summary>
	*/
	struct Scan
	{
		size_t			scan_id;
		size_t			peaks;
		unsigned char	ms_level;
		char			polarity;
		double			min_mz;
		double			max_mz;
		double			base_peak_mz;
		double			base_peak_intensity;
		double			retention_time;
		double			total_ion_current;
	};

	/*	Event
		<summary>Represents a MS/MS event.</summary>
	*/
	struct Event
	{
		Scan*			spectral_scan;
		Scan*			precursor_scan;
		unsigned char	precursor_charge_state;
		double			precursor_intensity;
		double			precursor_mz;
	};

	/*	MzXML_File
		<summary>Represents the information present within a MzXML file.</summary>
	*/
	struct MzXML_File
	{
		MzXML_File(void);								// Default Constructor
		MzXML_File(const MzXML_File& other);			// Copy Constructor
		MzXML_File(MzXML_File&& other);					// Move Constructor
		~MzXML_File(void);								// Destructor

		MzXML_File& operator=(const MzXML_File& other);	// Copy Assignment
		MzXML_File& operator=(MzXML_File&& other);		// Move Assignment

		std::string							filename;
		std::unordered_map<size_t, Scan>	scans;
		std::unordered_map<size_t, Event>	events;
	};
}


