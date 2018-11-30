#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Filetypes/Mzid/Mzid_File.h"
#include "IO/StreamParser-deprecated.hpp"
#include "Utilities/StringManipulation.h"

namespace TAPP::Filetypes::Mzid
{
	// TODO:	Completely rewrite this class into something that does respect the xml structure.

	class Mzid_Parser : public IO::StreamParser<Mzid_File>
	{
		public:
			/*** Constructors / Destructor **********************************************/

			Mzid_Parser(bool ignore_file_errors = false, size_t buffer_size = 1024000);

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
			/*** Enumerables ************************************************************/

			enum Stage { END, EVIDENCE, IDENTIFICATIONS, PEPTIDES, PROTEIN, SEQUENCES };

			/*** Variables **************************************************************/

			// Constant for the bracket type.
			const static Utilities::StringManipulation::BracketType	m_brackets_ = Utilities::StringManipulation::QOUTES;

			bool					m_ignore_file_errors_;
			std::hash<std::string>	m_hasher_;
			Stage					m_current_stage_;

			/*** Functions **************************************************************/

			bool 										ParseEvidences_(std::string& buffer);
			bool 										ParsePeptides_(std::string& buffer);
			bool 										ParseProteinGroup_(std::string& buffer);
			bool 										ParseSequences_(std::string& buffer);
			bool										ParseSpectrumIdentificationResults_(std::string& buffer);

			std::vector<ProteinHypothesis>				ParsePeptideHypothesis_(std::string& buffer, size_t start_position, size_t end_position);
			std::vector<SpectrumIdentificationItem*>	ParseSpectrumIdentificationItem_(std::string& buffer, size_t start_position, size_t end_position);
			std::vector<CV_Parameter>					ParseCV_Parameters_(std::string& buffer, size_t start_position, size_t end_position);
			std::vector<PeptideEvidence*>				ParseSpectrumEvidences_(std::string& buffer, size_t start_position, size_t end_position);
			std::vector<Iontype>						ParseSpectrumIons_(std::string& buffer, size_t start_position, size_t end_position);
	};
}
