#pragma once
#include <string>
#include <vector>

#include "IO/FileFormat/DSV/Parser.hpp"

namespace TAPP::Filetypes::TAPP
{
	/// <summary>Resembles the structure of a MPKS file record.</summary>
	struct MPKS
	{
		size_t				id;
		double				mz;
		double				rt;
		size_t				number_of_peaks;
		double				mz_sigma;
		double				rt_sigma;
		double				mz_weighted_sigma;
		double				rt_weighted_sigma;
		double				intensity;
		double				volume;
		double				extreme_fold_ratio;
		double				extreme_class;

		std::vector<double>	class_values;
		std::vector<double> file_values;
	};

	/// <summary>Creates a parser that can read MPKS filestreams.</summary>
	/// <param name="grammar">The data separated value grammar for the MPKS file.</param>
	/// <param name="buffer_size">The maximum amount of buffered characters.</param>
	/// <returns>A parser that can read MPKS filestreams.</returns>
	Filetypes::DSV::Parser<MPKS> Create_MPKS_Parser(const Filetypes::DSV::Grammar grammar, const size_t buffer_size);

	/// <summary>Parses a MPKS file located at the passed filepath.</summary>
	/// <param name="filepath">The filepath corresponding to the MPKS file to parse.</param>
	/// <param name="grammar">The data separated value grammar for the MPKS file.</param>
	/// <param name="buffer_size">The maximum amount of buffered characters.</param>
	/// <returns>A vector containing all the MPKS records for the passed filepath.</returns>
	std::vector<MPKS> ParseMPKS(const std::string filepath, const Filetypes::DSV::Grammar grammar, const size_t buffer_size);

	/// <summary>Parses MPKS files located at the passed filepaths.</summary>
	/// <param name="filepath">The filepaths corresponding to the MPKS files to parse.</param>
	/// <param name="grammar">The data separated value grammar for the MPKS file.</param>
	/// <param name="buffer_size">The maximum amount of buffered characters.</param>
	/// <returns>A vector containing vectors that each hold the full list of MPKS records, corresponding to the order of the filepaths passed.</returns>
	std::vector<std::vector<MPKS>> ParseMultipleMPKS(const std::vector<std::string>& filepaths, const Filetypes::DSV::Grammar grammar, const size_t buffer_size);

	/// <summary>Writes MPKS records into the passed outstream.</summary>
	/// <param name="out">The outstream to which the records will be written.</param>
	/// <param name="records">The records to write into the outstream.</param>
	/// <param name="grammar">The data separated value grammar for the IPL file.</param>
	/// <param name="output_header">Whether or not to write the header into the outstream.</param>
	void WriteMPKS(std::ostream& out, const std::vector<MPKS>& records, const Filetypes::DSV::Grammar grammar, const bool output_header);
}
