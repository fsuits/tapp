// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#include "Filetypes/TAPP/IPL.h"

#include <fstream>
#include <functional>

#include "Utilities/StringManipulation.h"

namespace TAPP::Filetypes::TAPP
{
	Filetypes::DSV::Parser<IPL> Create_IPL_Parser(const Filetypes::DSV::Grammar grammar, const size_t buffer_size)
	{
		return Filetypes::DSV::Parser<IPL>(grammar, true, buffer_size, [](const std::vector<std::string>& header_columns, const std::vector<std::vector<Filetypes::DSV::ValuePosition>>& value_columns)
		{
			// Creates a vector which can hold all the new records.
			std::vector<IPL> records;
			records.reserve(value_columns.size());

			// Loops through each set of values, converting them into records.
			for (const std::vector<Filetypes::DSV::ValuePosition>& row : value_columns)
			{
				// Reads the peak values from the string.
				std::vector<size_t> cluster_peaks;
				if (row.size() == 8)
				{
					std::vector<std::string> peak_values(Utilities::StringManipulation::Split(std::string(row[7].begin, row[7].end), ','));
					cluster_peaks.reserve(peak_values.size());

					for (std::string& peak_id : peak_values)
					{
						cluster_peaks.push_back(std::stoi(peak_id));
					}
				}

				records.push_back(
				{
					(size_t)std::stoi(std::string(row[0].begin, row[0].end)),
					(size_t)std::stoi(std::string(row[1].begin, row[1].end)),
					(size_t)std::stoi(std::string(row[2].begin, row[2].end)),
					(int)std::stoi(std::string(row[3].begin, row[3].end)),
					std::string(row[4].begin, row[4].end),
					*row[5].begin,
					(unsigned char)std::stoi(std::string(row[6].begin, row[6].end)),
					std::move(cluster_peaks)
				});
			}

			return records;
		});
	}

	std::vector<IPL> ParseIPL(const std::string filepath, const Filetypes::DSV::Grammar grammar, const size_t buffer_size)
	{
		// Acquires the parser.
		Filetypes::DSV::Parser<IPL> parser(Create_IPL_Parser(grammar, buffer_size));

		// Parses the file.
		return parser.Parse(filepath);
	}

	std::vector<std::vector<IPL>> ParseMultipleIPL(const std::vector<std::string>& filepaths, const Filetypes::DSV::Grammar grammar, const size_t buffer_size)
	{
		std::vector<std::vector<IPL>> IPL_files;
		IPL_files.reserve(filepaths.size());

		// Acquires the parser.
		Filetypes::DSV::Parser<IPL> parser(Create_IPL_Parser(grammar, buffer_size));

		// Loops through each file, parsing it into the vector.
		for (const std::string& filepath : filepaths)
		{
			IPL_files.push_back(parser.Parse(filepath));
		}

		return IPL_files;
	}

	void WriteIPL(std::ostream& out, const std::vector<IPL>& records, const Filetypes::DSV::Grammar grammar, const bool output_header)
	{
		if (output_header)
		{
			std::vector<std::string> header_columns{ "cluster_id", "cluster_group", "event_peak_id", "event_id", "identification_id", "identification_method", "chargestate", "cluster_peaks" };

			for (const std::string& header_column : header_columns)
			{
				out << header_column << grammar.value_delimiter;
			}

			out << grammar.record_delimiter;
		}

		for (const IPL& record : records)
		{
			out								<< record.cluster_id											<< grammar.value_delimiter
											<< record.cluster_group											<< grammar.value_delimiter
											<< record.event_peak											<< grammar.value_delimiter
											<< record.event_id												<< grammar.value_delimiter
			<< grammar.value_encapsulation	<< record.identification_id		<< grammar.value_encapsulation	<< grammar.value_delimiter
											<< record.identification_method									<< grammar.value_delimiter
											<< std::to_string(record.chargestate)							<< grammar.value_delimiter;

			out << grammar.value_encapsulation;
			for (size_t peak : record.cluster_peaks)
			{
				out << peak;

				if (peak != record.cluster_peaks.back())
				{
					out << ',';
				}
			}
			out << grammar.value_encapsulation << grammar.record_delimiter;
		}
	}
}
