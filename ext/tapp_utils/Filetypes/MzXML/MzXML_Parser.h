// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#pragma once

#include <string>
#include <unordered_map>

#include "Filetypes/MzXML/MzXML_File.h"
#include "IO/StreamParser-deprecated.hpp"
#include "Utilities/StringManipulation.h"

namespace TAPP::Filetypes::MzXML
{
	// TODO: Improve speed by adding offsets to the string finds.

	class MzXML_Parser : public IO::StreamParser<MzXML_File>
	{
		public:
			/*** Constructors / Destructor **********************************************/

			MzXML_Parser(size_t buffer_size = 1024000);

		protected:
			/*** Functions **************************************************************/

			/*	Closure$
				<summary>The implementation allows the parser to clean up or perform other actions.</summary>
			*/
			void Closure$(void);
			/*	ParseString$
				<summary>Parses the passed buffer.</summary>
				<param name="buffer">A reference to the buffer string.</param>
			*/
			void ParseString$(std::string& buffer);
			/*	Setup$
				<summary>Prepares the parser for the next run.</summary>
				<param name="filepath">A constant reference to a string containing the filepath.</param>
			*/
			void Setup$(const std::string& filepath);

		private:
			/*** Variables **************************************************************/

			// Constant for the bracket type.
			const static Utilities::StringManipulation::BracketType	m_brackets_ = Utilities::StringManipulation::QOUTES;

			/*** Functions **************************************************************/
			void ParseScans_(std::string& buffer);
			void ParseEvent_(Scan& scan, std::string& buffer, size_t start_position, size_t end_position);
	};
}
