// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#include "Filetypes/MzXML/MzXML_Parser.h"

using namespace TAPP::Utilities::StringManipulation;

namespace TAPP::Filetypes::MzXML
{
	/*** Constructors ***********************************************************/

	// Standard Constructor
	MzXML_Parser::MzXML_Parser(size_t buffer_size) : StreamParser(buffer_size)
	{
	}

	/*** Protected Functions ****************************************************/

	void MzXML_Parser::ParseString$(std::string& buffer)
	{
		if (buffer.find("<scan") != std::string::npos)
		{
			ParseScans_(buffer);
		}
		else
		{
			buffer.clear();
		}
	}

	void MzXML_Parser::Closure$(void)
	{
	}

	void MzXML_Parser::Setup$(const std::string& filepath)
	{
		m_return_object$ = MzXML_File();
		m_return_object$.filename = FilepathToFilename(filepath);
	}

	/*** Private Functions ******************************************************/

	// Parses the scan information.
	void MzXML_Parser::ParseScans_(std::string& buffer)
	{
		// Acquires the initial positions for the parsing.
		size_t scan_previous = 0;
		size_t scan_start = buffer.find("<scan");
		size_t scan_end = buffer.find("<peaks", scan_start);

		// Only parses complete pieces of data.
		while (scan_start != std::string::npos && scan_end != std::string::npos)
		{
			// Locates and parses the scan number.
			size_t scan_number = std::stoi(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("num", scan_start)));

			// Parses, creates and inserts the Scan object. It then checks if an Event object needs to be created.
			ParseEvent_
			(
				m_return_object$.scans.insert({ scan_number,
				{
					scan_number,
					(size_t)std::stoi(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("peaksCount", scan_start) + 10)),
					(unsigned char)std::stoi(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("msLevel", scan_start) + 7)),
					GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("polarity", scan_start) + 8)[0],
					std::stod(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("lowMz", scan_start) + 5)),
					std::stod(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("highMz", scan_start) + 6)),
					std::stod(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("basePeakMz", scan_start) + 10)),
					std::stod(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("basePeakIntensity", scan_start) + 17)),
					std::stod(GetEmbeddedCharacters(buffer, 'T', 'S', buffer.find("retentionTime", scan_start) + 13)),
					std::stod(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("totIonCurrent", scan_start) + 13))
				} }).first->second,
				buffer,
				scan_start,
				scan_end
			);

			scan_previous = scan_end;
			scan_start = buffer.find("<scan", scan_end);
			scan_end = buffer.find("<peaks", scan_start);
		}

		// Trims the buffer so it only includes unparsed characters.
		buffer = buffer.substr(buffer.find('<', scan_previous));
	}

	// Parses the event information.
	void MzXML_Parser::ParseEvent_(Scan& scan, std::string& buffer, size_t start_position, size_t end_position)
	{
		// Checks if event information is available.
		size_t precursor_scan_position = buffer.find("precursorScanNum", start_position);
		if (precursor_scan_position < end_position && precursor_scan_position != std::string::npos)
		{
			m_return_object$.events.insert({ scan.scan_id,
			{
				&scan,
				&m_return_object$.scans.find(std::stoi(GetEmbeddedCharacters(buffer, m_brackets_, precursor_scan_position)))->second,
				(unsigned char)std::stoi(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("precursorCharge", precursor_scan_position))),
				atof(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("precursorIntensity", precursor_scan_position)).c_str()),
				atof(GetEmbeddedCharacters(buffer, '>', '<', precursor_scan_position).c_str())
			}});
		}
	}
}
