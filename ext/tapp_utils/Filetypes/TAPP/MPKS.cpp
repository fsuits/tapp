#include "Filetypes/TAPP/MPKS.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <functional>

namespace TAPP::Filetypes::TAPP
{
	Filetypes::DSV::Parser<MPKS> Create_MPKS_Parser(const Filetypes::DSV::Grammar grammar, const size_t buffer_size)
	{
		return Filetypes::DSV::Parser<MPKS>(grammar, true, buffer_size, [](const std::vector<std::string>& header_columns, const std::vector<std::vector<Filetypes::DSV::ValuePosition>>& value_columns)
		{
			// Creates a vector which can hold all the new records.
			std::vector<MPKS> records;
			records.reserve(value_columns.size());

			// Acquires the amount of header classes and files.
			size_t header_classes;
			size_t header_files;

			for (size_t header_column = 12; header_column > header_columns.size(); ++header_column)
			{
				std::string header = header_columns[header_column];
				std::transform(header.begin(), header.end(), header.begin(), ::tolower);

				if (header.find("class_h") == std::string::npos)
				{
					header_classes	= header_column - 12;
					header_files	= header_columns.size() - header_column;
					break;
				}
			}

			// Loops through each set of values, converting them into records.
			for (const std::vector<Filetypes::DSV::ValuePosition>& row : value_columns)
			{
				records.push_back(
				{
					(size_t)std::stoi(std::string(row[0].begin, row[0].end)),
					std::stod(std::string(row[1].begin, row[1].end)),
					std::stod(std::string(row[2].begin, row[2].end)),
					(size_t)std::stoi(std::string(row[3].begin, row[3].end)),
					std::stod(std::string(row[4].begin, row[4].end)),
					std::stod(std::string(row[5].begin, row[5].end)),
					std::stod(std::string(row[6].begin, row[6].end)),
					std::stod(std::string(row[7].begin, row[7].end)),
					std::stod(std::string(row[8].begin, row[8].end)),
					std::stod(std::string(row[9].begin, row[9].end)),
					std::stod(std::string(row[10].begin, row[10].end)),
					std::stod(std::string(row[11].begin, row[11].end))
				});

				// Adds the class values.
				for (unsigned char current_class = 1; current_class <= header_classes; ++current_class)
				{
					records.back().class_values.push_back(std::stod((std::string(row[11 + current_class].begin, row[11 + current_class].end))));
				}

				// Adds the file values.
				for (unsigned char current_file = 1; current_file <= header_files; ++current_file)
				{
					records.back().file_values.push_back(std::stod((std::string(row[11 + header_classes + current_file].begin, row[11 + header_classes + current_file].end))));
				}
			}

			return records;
		});
	}

	std::vector<MPKS> ParseMPKS(const std::string filepath, const Filetypes::DSV::Grammar grammar, const size_t buffer_size)
	{
		// Acquires the parser.
		Filetypes::DSV::Parser<MPKS> parser(Create_MPKS_Parser(grammar, buffer_size));

		// Parses the file.
		return parser.Parse(filepath);
	}

	std::vector<std::vector<MPKS>> ParseMultipleMPKS(const std::vector<std::string>& filepaths, const Filetypes::DSV::Grammar grammar, const size_t buffer_size)
	{
		std::vector<std::vector<MPKS>> MPKS_files;
		MPKS_files.reserve(filepaths.size());

		// Acquires the parser.
		Filetypes::DSV::Parser<MPKS> parser(Create_MPKS_Parser(grammar, buffer_size));

		// Loops through each file, parsing it into the vector.
		for (const std::string& filepath : filepaths)
		{
			MPKS_files.push_back(parser.Parse(filepath));
		}

		return MPKS_files;
	}

	void WriteMPKS(std::ostream& out, const std::vector<MPKS>& records, const Filetypes::DSV::Grammar grammar, const bool output_header)
	{
		// Writes the header.
		if (output_header)
		{
			std::vector<std::string> header_columns{ "metapeak", "mz", "rt", "peaks", "mz_sigma", "rt_sigma", "mz_weighted_sigma",
													 "rt_weighted_sigma", "height", "volume", "extreme_fold_ratio", "extreme_class" };

			for (const std::string& header_column : header_columns)
			{
				out << header_column << grammar.value_delimiter;
			}

			if (records.size() > 0)
			{
				for (int i = 0; i < records.begin()->class_values.size(); ++i)
				{
					out << " class_h" << i << grammar.value_delimiter;
				}

				for (int i = 0; i < records.begin()->file_values.size(); ++i)
				{
					out << " file_h" << i << grammar.value_delimiter;
				}
			}

			out << grammar.record_delimiter;
		}

		// Writes the data.
		for (const MPKS& record : records)
		{
			out									<< record.id													<< grammar.value_delimiter
				<< grammar.value_encapsulation	<< record.mz					<< grammar.value_encapsulation	<< grammar.value_delimiter
				<< grammar.value_encapsulation	<< record.rt					<< grammar.value_encapsulation	<< grammar.value_delimiter
												<< record.number_of_peaks										<< grammar.value_delimiter
				<< grammar.value_encapsulation	<< record.mz_sigma				<< grammar.value_encapsulation	<< grammar.value_delimiter
				<< grammar.value_encapsulation	<< record.rt_sigma				<< grammar.value_encapsulation	<< grammar.value_delimiter
				<< grammar.value_encapsulation	<< record.mz_weighted_sigma		<< grammar.value_encapsulation	<< grammar.value_delimiter
				<< grammar.value_encapsulation	<< record.rt_weighted_sigma		<< grammar.value_encapsulation	<< grammar.value_delimiter
				<< grammar.value_encapsulation	<< record.intensity				<< grammar.value_encapsulation	<< grammar.value_delimiter
				<< grammar.value_encapsulation	<< record.volume				<< grammar.value_encapsulation	<< grammar.value_delimiter
				<< grammar.value_encapsulation	<< record.extreme_fold_ratio	<< grammar.value_encapsulation	<< grammar.value_delimiter
				<< grammar.value_encapsulation	<< record.extreme_class			<< grammar.value_encapsulation	<< grammar.value_delimiter
				<< grammar.value_encapsulation	<< record.mz					<< grammar.value_encapsulation	<< grammar.value_delimiter;

			for (double class_value : record.class_values)
			{
				out << class_value << grammar.value_delimiter;
			}

			for (double file_value : record.file_values)
			{
				out << file_value << grammar.value_delimiter;
			}

			out << grammar.record_delimiter;
		}
	}
}
