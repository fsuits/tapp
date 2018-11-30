#include "Filetypes/TAPP/Header.h"

#include <fstream>
#include <functional>

// TODO: Implement Boost Lexical casts to convert iterators to values, rather than have the conversion to string at intermediary string.

namespace TAPP::Filetypes::TAPP
{
	Filetypes::DSV::Parser<Header> Create_Header_Parser(const Filetypes::DSV::Grammar grammar, const size_t buffer_size)
	{
		return Filetypes::DSV::Parser<Header>(grammar, true, buffer_size, [](const std::vector<std::string>& header_columns, const std::vector<std::vector<Filetypes::DSV::ValuePosition>>& value_columns)
		{
			// Creates a vector which can hold all the new records.
			std::vector<std::pair<std::string, std::string>> records;
			records.reserve(value_columns.size());

			// Loops through each set of values, converting them into records.
			for (const std::vector<Filetypes::DSV::ValuePosition>& row : value_columns)
			{
				records.push_back({ std::string(row[0].begin, row[0].end), std::string(row[1].begin, row[1].end) });
			}

			return records;
		});
	}

	std::vector<Header> ParseHeader(const std::string filepath, const Filetypes::DSV::Grammar grammar, const size_t buffer_size)
	{
		// Acquires the parser.
		Filetypes::DSV::Parser<Header> parser(Create_Header_Parser(grammar, buffer_size));

		// Parses the file.
		return parser.Parse(filepath);
	}

	std::vector<std::vector<Header>> ParseMultipleHeader(const std::vector<std::string>& filepaths, const Filetypes::DSV::Grammar grammar, const size_t buffer_size)
	{
		std::vector<std::vector<Header>> Header_files;
		Header_files.reserve(filepaths.size());

		// Acquires the parser.
		Filetypes::DSV::Parser<Header> parser(Create_Header_Parser(grammar, buffer_size));

		// Loops through each file, parsing it into the vector.
		for (const std::string& filepath : filepaths)
		{
			Header_files.push_back(parser.Parse(filepath));
		}

		return Header_files;
	}

	void WriteHeader(std::ostream& out, const std::vector<Header>& records, const Filetypes::DSV::Grammar grammar, const bool output_header)
	{
		// Writes the header.
		if (output_header)
		{
			out << "key" << grammar.value_delimiter << "value" << grammar.record_delimiter;
		}

		// Writes the data.
		for (const Header& record : records)
		{
			out << record.first << grammar.value_delimiter << grammar.value_encapsulation << record.second << grammar.value_encapsulation << grammar.record_delimiter;
		}
	}
}
