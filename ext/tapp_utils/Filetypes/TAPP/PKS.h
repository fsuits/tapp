#pragma once
#include <string>
#include <vector>

#include "IO/FileFormat/DSV/Parser.hpp"

namespace TAPP::Filetypes::TAPP
{
	/// <summary>Resembles the structure of a PKS file record.</summary>
	struct PKS
	{
		size_t id;
		double mz;
		double rt;
		double intensity;
		double volume;
		double v_centroid;
		double mz_sigma;
		double rt_sigma;
		size_t count;
		double local_background;
		double sn_volume;
		double sn_height;
		double sn_centroid;
	};

	/// <summary>Creates a parser that can read PKS filestreams.</summary>
	/// <param name="grammar">The data separated value grammar for the PKS file.</param>
	/// <param name="buffer_size">The maximum amount of buffered characters.</param>
	/// <returns>A parser that can read PKS filestreams.</returns>
	Filetypes::DSV::Parser<PKS> Create_PKS_Parser(const Filetypes::DSV::Grammar grammar, const size_t buffer_size);

	/// <summary>Parses a PKS file located at the passed filepath.</summary>
	/// <param name="filepath">The filepath corresponding to the PKS file to parse.</param>
	/// <param name="grammar">The data separated value grammar for the PKS file.</param>
	/// <param name="buffer_size">The maximum amount of buffered characters.</param>
	/// <returns>A vector containing all the PKS records for the passed filepath.</returns>
	std::vector<PKS> ParsePKS(const std::string filepath, const Filetypes::DSV::Grammar grammar, const size_t buffer_size);

	/// <summary>Parses PKS files located at the passed filepaths.</summary>
	/// <param name="filepath">The filepaths corresponding to the PKS files to parse.</param>
	/// <param name="grammar">The data separated value grammar for the PKS file.</param>
	/// <param name="buffer_size">The maximum amount of buffered characters.</param>
	/// <returns>A vector containing vectors that each hold the full list of PKS records, corresponding to the order of the filepaths passed.</returns>
	std::vector<std::vector<PKS>> ParseMultiplePKS(const std::vector<std::string>& filepaths, const Filetypes::DSV::Grammar grammar, const size_t buffer_size);

	/// <summary>Writes PKS records into the passed outstream.</summary>
	/// <param name="out">The outstream to which the records will be written.</param>
	/// <param name="records">The records to write into the outstream.</param>
	/// <param name="grammar">The data separated value grammar for the IPL file.</param>
	/// <param name="output_header">Whether or not to write the header into the outstream.</param>
	void WritePKS(std::ostream& out, const std::vector<PKS>& records, const Filetypes::DSV::Grammar grammar, const bool output_header);
}
