#pragma once
#include <string>
#include <vector>

#include "IO/FileFormat/DSV/Parser.hpp"

namespace TAPP::Filetypes::TAPP
{
	/// <summary>Resembles the structure of a IPL file record.</summary>
	struct IPL
	{
		size_t				cluster_id;
		size_t				cluster_group;
		size_t				event_peak;
		int					event_id;
		std::string			identification_id;
		char				identification_method;
		unsigned char		chargestate;
		std::vector<size_t>	cluster_peaks;
	};

	/// <summary>Creates a parser that can read IPL filestreams.</summary>
	/// <param name="grammar">The data separated value grammar for the IPL file.</param>
	/// <param name="buffer_size">The maximum amount of buffered characters.</param>
	/// <returns>A parser that can read IPL filestreams.</returns>
	Filetypes::DSV::Parser<IPL> Create_IPL_Parser(const Filetypes::DSV::Grammar grammar, const size_t buffer_size);

	/// <summary>Parses a IPL file located at the passed filepath.</summary>
	/// <param name="filepath">The filepath corresponding to the IPL file to parse.</param>
	/// <param name="grammar">The data separated value grammar for the IPL file.</param>
	/// <param name="buffer_size">The maximum amount of buffered characters.</param>
	/// <returns>A vector containing all the IPL records for the passed filepath.</returns>
	std::vector<IPL> ParseIPL(const std::string filepath, const Filetypes::DSV::Grammar grammar, const size_t buffer_size);

	/// <summary>Parses IPL files located at the passed filepaths.</summary>
	/// <param name="filepath">The filepaths corresponding to the IPL files to parse.</param>
	/// <param name="grammar">The data separated value grammar for the IPL file.</param>
	/// <param name="buffer_size">The maximum amount of buffered characters.</param>
	/// <returns>A vector containing vectors that each hold the full list of IPL records, corresponding to the order of the filepaths passed.</returns>
	std::vector<std::vector<IPL>> ParseMultipleIPL(const std::vector<std::string>& filepaths, const Filetypes::DSV::Grammar grammar, const size_t buffer_size);

	/// <summary>Writes IPL records into the passed outstream.</summary>
	/// <param name="out">The outstream to which the records will be written.</param>
	/// <param name="records">The records to write into the outstream.</param>
	/// <param name="grammar">The data separated value grammar for the IPL file.</param>
	/// <param name="output_header">Whether or not to write the header into the outstream.</param>
	void WriteIPL(std::ostream& out, const std::vector<IPL>& records, const Filetypes::DSV::Grammar grammar, const bool output_header);
}
