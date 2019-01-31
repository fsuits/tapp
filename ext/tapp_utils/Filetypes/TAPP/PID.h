// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#pragma once
#include <string>
#include <vector>

#include "IO/FileFormat/DSV/Parser.hpp"

namespace TAPP::Filetypes::TAPP
{
	/// <summary>Resembles the structure of a PID file record.</summary>
	struct PID
	{
		int64_t			metapeak_id;
		unsigned char	file_id;
		unsigned char	class_id;
		uint64_t		peak_id;
		double			mz;
		double			rt;
		double			intensity;
	};

	/// <summary>Creates a parser that can read PID filestreams.</summary>
	/// <param name="grammar">The data separated value grammar for the PID file.</param>
	/// <param name="buffer_size">The maximum amount of buffered characters.</param>
	/// <returns>A parser that can read PID filestreams.</returns>
	Filetypes::DSV::Parser<PID> Create_PID_Parser(const Filetypes::DSV::Grammar grammar, const size_t buffer_size);

	/// <summary>Parses a PID file located at the passed filepath.</summary>
	/// <param name="filepath">The filepath corresponding to the PID file to parse.</param>
	/// <param name="grammar">The data separated value grammar for the PID file.</param>
	/// <param name="buffer_size">The maximum amount of buffered characters.</param>
	/// <returns>A vector containing all the PID records for the passed filepath.</returns>
	std::vector<PID> ParsePID(const std::string filepath, const Filetypes::DSV::Grammar grammar, const size_t buffer_size);

	/// <summary>Parses PID files located at the passed filepaths.</summary>
	/// <param name="filepath">The filepaths corresponding to the PID files to parse.</param>
	/// <param name="grammar">The data separated value grammar for the PID file.</param>
	/// <param name="buffer_size">The maximum amount of buffered characters.</param>
	/// <returns>A vector containing vectors that each hold the full list of PID records, corresponding to the order of the filepaths passed.</returns>
	std::vector<std::vector<PID>> ParseMultiplePID(const std::vector<std::string>& filepaths, const Filetypes::DSV::Grammar grammar, const size_t buffer_size);

	/// <summary>Writes PID records into the passed outstream.</summary>
	/// <param name="out">The outstream to which the records will be written.</param>
	/// <param name="records">The records to write into the outstream.</param>
	/// <param name="grammar">The data separated value grammar for the IPL file.</param>
	/// <param name="output_header">Whether or not to write the header into the outstream.</param>
	void WritePID(std::ostream& out, const std::vector<PID>& records, const Filetypes::DSV::Grammar grammar, const bool output_header);
}
