#include "Filetypes/TAPP/PKS.h"

#include <fstream>
#include <functional>

// TODO: Implement Boost Lexical casts to convert iterators to values, rather than have the conversion to string at intermediary string.

namespace TAPP::Filetypes::TAPP
{
	Filetypes::DSV::Parser<PKS> Create_PKS_Parser(const Filetypes::DSV::Grammar grammar, const size_t buffer_size)
	{
		return Filetypes::DSV::Parser<PKS>(grammar, true, buffer_size, [](const std::vector<std::string>& header_columns, const std::vector<std::vector<Filetypes::DSV::ValuePosition>>& value_columns)
		{
			// Creates a vector which can hold all the new records.
			std::vector<PKS> records;
			records.reserve(value_columns.size());

			// Loops through each set of values, converting them into records.
			for (const std::vector<Filetypes::DSV::ValuePosition>& row : value_columns)
			{
				records.push_back(
				{
					(size_t)std::stoi(std::string(row[0].begin, row[0].end)),
					std::stod(std::string(row[1].begin, row[1].end)),
					std::stod(std::string(row[2].begin, row[2].end)),
					std::stod(std::string(row[3].begin, row[3].end)),
					std::stod(std::string(row[4].begin, row[4].end)),
					std::stod(std::string(row[5].begin, row[5].end)),
					std::stod(std::string(row[6].begin, row[6].end)),
					std::stod(std::string(row[7].begin, row[7].end)),
					(size_t)std::stoi(std::string(row[8].begin, row[8].end)),
					std::stod(std::string(row[9].begin, row[9].end)),
					std::stod(std::string(row[10].begin, row[10].end)),
					std::stod(std::string(row[11].begin, row[11].end)),
					std::stod(std::string(row[12].begin, row[12].end))
				});
			}

			return records;
		});
	}

	std::vector<PKS> ParsePKS(const std::string filepath, const Filetypes::DSV::Grammar grammar, const size_t buffer_size)
	{
		// Acquires the parser.
		Filetypes::DSV::Parser<PKS> parser(Create_PKS_Parser(grammar, buffer_size));

		// Parses the file.
		return parser.Parse(filepath);
	}

	std::vector<std::vector<PKS>> ParseMultiplePKS(const std::vector<std::string>& filepaths, const Filetypes::DSV::Grammar grammar, const size_t buffer_size)
	{
		std::vector<std::vector<PKS>> PKS_files;
		PKS_files.reserve(filepaths.size());

		// Acquires the parser.
		Filetypes::DSV::Parser<PKS> parser(Create_PKS_Parser(grammar, buffer_size));

		// Loops through each file, parsing it into the vector.
		for (const std::string& filepath : filepaths)
		{
			PKS_files.push_back(parser.Parse(filepath));
		}

		return PKS_files;
	}

	void WritePKS(std::ostream& out, const std::vector<PKS>& records, const Filetypes::DSV::Grammar grammar, const bool output_header)
	{
		// Writes the header.
		if (output_header)
		{
			std::vector<std::string> header_columns{ "N", "X", "Y", "Height", "Volume", "VCentroid", "XSigma", "YSigma", "Count", "LocalBkgnd", "SNVolume", "SNHeight", "SNCentroid" };

			for (const std::string& header_column : header_columns)
			{
				out << header_column << grammar.value_delimiter;
			}

			out << grammar.record_delimiter;
		}

		// Writes the data.
		for (const PKS& record : records)
		{
			out<< record.id					<< grammar.value_delimiter
					<< record.mz					<< grammar.value_delimiter
					<< record.rt					<< grammar.value_delimiter
					<< record.intensity				<< grammar.value_delimiter
					<< record.volume				<< grammar.value_delimiter
					<< record.v_centroid			<< grammar.value_delimiter
					<< record.mz_sigma				<< grammar.value_delimiter
					<< record.rt_sigma				<< grammar.value_delimiter
												<< record.count												<< grammar.value_delimiter
					<< record.local_background		<< grammar.value_delimiter
					<< record.sn_volume				<< grammar.value_delimiter
					<< record.sn_height				<< grammar.value_delimiter
					<< record.sn_centroid			<< grammar.value_delimiter
				<< grammar.record_delimiter;
		}
	}
}
