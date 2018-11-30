#pragma once
#include <string>
#include <vector>

#include "IO/FileFormat/DSV/Parser.hpp"

namespace TAPP::Filetypes::TAPP
{
	/// <summary>Represents a single Header record..</summary>
	typedef std::pair<std::string, std::string> Header;

	/// <summary>Creates a parser that can read Header filestreams.</summary>
	/// <param name="grammar">The data separated value grammar for the Header file.</param>
	/// <param name="buffer_size">The maximum amount of buffered characters.</param>
	/// <returns>A parser that can read Header filestreams.</returns>
	Filetypes::DSV::Parser<Header> Create_Header_Parser(const Filetypes::DSV::Grammar grammar, const size_t buffer_size);

	/// <summary>Parses a Header file located at the passed filepath.</summary>
	/// <param name="filepath">The filepath corresponding to the Header file to parse.</param>
	/// <param name="grammar">The data separated value grammar for the Header file.</param>
	/// <param name="buffer_size">The maximum amount of buffered characters.</param>
	/// <returns>A vector containing all the Header records for the passed filepath.</returns>
	std::vector<Header> ParseHeader(const std::string filepath, const Filetypes::DSV::Grammar grammar, const size_t buffer_size);

	/// <summary>Parses Header files located at the passed filepaths.</summary>
	/// <param name="filepath">The filepaths corresponding to the Header files to parse.</param>
	/// <param name="grammar">The data separated value grammar for the Header file.</param>
	/// <param name="buffer_size">The maximum amount of buffered characters.</param>
	/// <returns>A vector containing vectors that each hold the full list of Header records, corresponding to the order of the filepaths passed.</returns>
	std::vector<std::vector<Header>> ParseMultipleHeader(const std::vector<std::string>& filepaths, const Filetypes::DSV::Grammar grammar, const size_t buffer_size);

	/// <summary>Writes Header records into the passed outstream.</summary>
	/// <param name="out">The outstream to which the records will be written.</param>
	/// <param name="records">The records to write into the outstream.</param>
	/// <param name="grammar">The data separated value grammar for the IPL file.</param>
	/// <param name="output_header">Whether or not to write the header into the outstream.</param>
	void WriteHeader(std::ostream& out, const std::vector<Header>& records, const Filetypes::DSV::Grammar grammar, const bool output_header);
}
