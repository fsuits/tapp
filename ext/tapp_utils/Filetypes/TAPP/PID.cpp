#include "Filetypes/TAPP/PID.h"

#include <fstream>
#include <functional>

namespace TAPP::Filetypes::TAPP
{
	Filetypes::DSV::Parser<PID> Create_PID_Parser(const Filetypes::DSV::Grammar grammar, const size_t buffer_size)
	{
		return Filetypes::DSV::Parser<PID>(grammar, true, buffer_size, [](const std::vector<std::string>& header_columns, const std::vector<std::vector<Filetypes::DSV::ValuePosition>>& value_columns)
		{
			// Creates a vector which can hold all the new records.
			std::vector<PID> records;
			records.reserve(value_columns.size());

			// Loops through each set of values, converting them into records.
			for (const std::vector<Filetypes::DSV::ValuePosition>& row : value_columns)
			{
				records.push_back(
				{
					(long)std::stoi(std::string(row[0].begin, row[0].end)),
					(unsigned char)std::stoi(std::string(row[4].begin, row[4].end)),
					(unsigned char)std::stoi(std::string(row[6].begin, row[6].end)),
					(size_t)std::stoi(std::string(row[5].begin, row[5].end)),
					std::stod(std::string(row[1].begin, row[1].end)),
					std::stod(std::string(row[2].begin, row[2].end)),
					std::stod(std::string(row[3].begin, row[3].end))
				});
			}

			return records;
		});
	}

	std::vector<PID> ParsePID(const std::string filepath, const Filetypes::DSV::Grammar grammar, const size_t buffer_size)
	{
		// Acquires the parser.
		Filetypes::DSV::Parser<PID> parser(Create_PID_Parser(grammar, buffer_size));

		// Parses the file.
		return parser.Parse(filepath);
	}

	std::vector<std::vector<PID>> ParseMultiplePID(const std::vector<std::string>& filepaths, const Filetypes::DSV::Grammar grammar, const size_t buffer_size)
	{
		std::vector<std::vector<PID>> PID_files;
		PID_files.reserve(filepaths.size());

		// Acquires the parser.
		Filetypes::DSV::Parser<PID> parser(Create_PID_Parser(grammar, buffer_size));

		// Loops through each file, parsing it into the vector.
		for (const std::string& filepath : filepaths)
		{
			PID_files.push_back(parser.Parse(filepath));
		}

		return PID_files;
	}

	void WritePID(std::ostream& out, const std::vector<PID>& records, const Filetypes::DSV::Grammar grammar, const bool output_header)
	{
		// Writes the header.
		if (output_header)
		{
			std::vector<std::string> header_columns{ "mpid", "mz", "rt", "height", "file_id", "peak_id", "class" };

			for (const std::string& header_column : header_columns)
			{
				out << header_column << grammar.value_delimiter;
			}

			out << grammar.record_delimiter;
		}

		// Writes the data.
		for (const PID& record : records)
		{
			out									<< record.metapeak_id												<< grammar.value_delimiter
				<< grammar.value_encapsulation	<< record.mz						<< grammar.value_encapsulation	<< grammar.value_delimiter
				<< grammar.value_encapsulation	<< record.rt						<< grammar.value_encapsulation	<< grammar.value_delimiter
				<< grammar.value_encapsulation	<< record.intensity					<< grammar.value_encapsulation	<< grammar.value_delimiter
												<< std::to_string(record.file_id)									<< grammar.value_delimiter
												<< record.peak_id													<< grammar.value_delimiter
												<< std::to_string(record.class_id)									<< grammar.value_delimiter
				<< grammar.record_delimiter;
		}
	}
}
