// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#include "Filetypes/Mzid/Mzid_Parser.h"

#include <stdexcept>

using namespace TAPP::Utilities::StringManipulation;

namespace TAPP::Filetypes::Mzid
{
	/*** Constructors ***********************************************************/

	// Standard Constructor
	Mzid_Parser::Mzid_Parser(bool allow_file_errors, size_t buffer_size) : StreamParser(buffer_size), m_ignore_file_errors_(allow_file_errors)
	{
	}

	/*** Protected Functions ****************************************************/

	void Mzid_Parser::Closure$(void)
	{
	}

	void Mzid_Parser::ParseString$(std::string& buffer)
	{
		try
		{
			switch (m_current_stage_)
			{
				case SEQUENCES:			if (ParseSequences_(buffer)) { m_current_stage_ = PEPTIDES; } break;
				case PEPTIDES:			if (ParsePeptides_(buffer)) { m_current_stage_ = EVIDENCE; } break;
				case EVIDENCE:			if (ParseEvidences_(buffer)) { m_current_stage_ = IDENTIFICATIONS; } break;
				case IDENTIFICATIONS:	if (ParseSpectrumIdentificationResults_(buffer)) { m_current_stage_ = PROTEIN; } break;
				case PROTEIN:			if (ParseProteinGroup_(buffer)) { m_current_stage_ = END; } break;
				case END:				buffer.clear(); break;
			}
		}
		catch (std::runtime_error& error)
		{
			throw std::runtime_error("Encountered an error while parsing file: " + m_file_reader$.GetFilepath() + "\n" + error.what());
		}
	}

	void Mzid_Parser::Setup$(const std::string& filepath)
	{
		m_current_stage_ = SEQUENCES;
		m_return_object$ = Mzid_File();
		m_return_object$.filename = FilepathToFilename(filepath);
	}

	/*** Private Functions ******************************************************/

	bool Mzid_Parser::ParseEvidences_(std::string& buffer)
	{
		// Prepares the vector that will hold the new evidence objects.
		std::vector<PeptideEvidence> evidences; // Yes, the plural form used here is incorrect.
		evidences.reserve(CountSubstrings(buffer, "<PeptideEvidence "));

		// Acquires the initial evidence.
		size_t evidence_previous = 0;
		size_t evidence_start = buffer.find("<PeptideEvidence ");
		size_t evidence_end = buffer.find('\n', evidence_start);

		while (evidence_start != std::string::npos && evidence_end != std::string::npos)
		{
			evidences.push_back({
				(unsigned short)stoi(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("end", evidence_start))),
				(unsigned short)stoi(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("start", evidence_start))),
				GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("id=", evidence_start)),
				"true" == GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("isDecoy", evidence_start)),
				GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("post", evidence_start))[0],
				GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("pre", evidence_start))[0],
				&m_return_object$.peptides.find(m_hasher_(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("peptide_ref", evidence_start))))->second,
				&m_return_object$.sequences.find(m_hasher_(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("dBSequence_ref", evidence_start))))->second
			});

			evidence_previous = evidence_end;
			evidence_start = buffer.find("<PeptideEvidence ", evidence_end);
			evidence_end = buffer.find('\n', evidence_start);
		}

		// Merges the vector with the existing vector.
		m_return_object$.peptide_evidence.reserve(m_return_object$.peptide_evidence.size() + evidences.size());
		for (PeptideEvidence evidence : evidences)
		{
			m_return_object$.peptide_evidence.insert({ m_hasher_(evidence.id), evidence });
		}

		// Trims the buffer so it only includes unparsed characters.
		buffer = buffer.substr(buffer.find('\n', evidence_previous));

		return buffer.find("</SequenceCollection") != std::string::npos;
	}

	bool Mzid_Parser::ParseProteinGroup_(std::string& buffer)
	{
		// Prepares the vector that will hold the new ProteinGroup objects.
		std::vector<ProteinGroup> protein_groups;
		protein_groups.reserve(CountSubstrings(buffer, "<ProteinA"));

		// Acquires the initial protein grouping.
		size_t protein_previous = 0;
		size_t protein_start = buffer.find("<ProteinA");
		size_t protein_end = buffer.find("</ProteinA", protein_start);

		// Only parses a protein group if all of its information is present within the buffer.
		while (protein_start != std::string::npos && protein_end != std::string::npos)
		{
			std::vector<ProteinHypothesis> hypotheses(ParsePeptideHypothesis_(buffer, protein_start, protein_end));
			size_t hypothesis_end = buffer.find("</ProteinD", buffer.find(hypotheses.back().id, protein_start));

			// Adds the protein group information to the vector.
			protein_groups.push_back(
			{
				GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("id", protein_start)),
				ParseCV_Parameters_(buffer, hypothesis_end, protein_end),
				std::move(hypotheses)
			});

			// Searches for the next protein group.
			protein_previous = protein_end;
			protein_start = buffer.find("<ProteinA", protein_end);
			protein_end = buffer.find("</ProteinA", protein_start);
		}

		// Merges the vector with the existing vector.
		m_return_object$.peptides.reserve(m_return_object$.peptides.size() + protein_groups.size());
		for (ProteinGroup group : protein_groups)
		{
			m_return_object$.protein_groups.insert({ m_hasher_(group.id), group });
		}

		// Trims the buffer so it only includes unparsed characters.
		buffer = buffer.substr(buffer.find('>', protein_previous));

		return buffer.find("</ProteinDetectionList>") != std::string::npos;
	}

	std::vector<ProteinHypothesis> Mzid_Parser::ParsePeptideHypothesis_(std::string& buffer, size_t start_position, size_t end_position)
	{
		// Creates the initial hypothesis vector.
		std::vector<ProteinHypothesis> hypothesis;

		size_t hypothesis_start = buffer.find("<ProteinD", start_position);
		size_t hypothesis_end = buffer.find("</ProteinD", hypothesis_start);
		while (hypothesis_start < end_position && hypothesis_start != std::string::npos)
		{
			// Creates a new hypothesis object.
			hypothesis.push_back(ProteinHypothesis());

			// Adds the spectrum references.
			size_t spectrum_position = buffer.find("spectrumIdentificationItem_ref", hypothesis_start);
			while (spectrum_position != std::string::npos && spectrum_position < hypothesis_end)
			{
				// If the item entry cannot be located, throw an error.
				auto identification_item_entry = m_return_object$.identification_items.find(m_hasher_(GetEmbeddedCharacters(buffer, m_brackets_, spectrum_position)));
				if (!m_ignore_file_errors_ && identification_item_entry == m_return_object$.identification_items.end())
				{
					throw std::runtime_error("Couldn't locate identification item with id: " + GetEmbeddedCharacters(buffer, m_brackets_, spectrum_position));
				}
				else if (identification_item_entry != m_return_object$.identification_items.end())
				{
					hypothesis.back().items.push_back(&identification_item_entry->second);
				}
				spectrum_position = buffer.find("spectrumIdentificationItem_ref", spectrum_position + 22);
			}
			
			// Fills the remaining fields.
			hypothesis.back().evidence			= &m_return_object$.peptide_evidence.find(m_hasher_(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("peptideEvidence_ref", hypothesis_start))))->second;
			hypothesis.back().id				= GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("id", hypothesis_start));
			hypothesis.back().passed_threshold	= "true" == GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("passThreshold", hypothesis_start));
			hypothesis.back().sequence			= &m_return_object$.sequences.find(m_hasher_(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("dBSequence_ref", hypothesis_start))))->second;
		
			hypothesis_start	= buffer.find("<ProteinD", hypothesis_end);
			hypothesis_end		= buffer.find("</ProteinD", hypothesis_start);
		}

		return hypothesis;
	}

	bool Mzid_Parser::ParsePeptides_(std::string& buffer)
	{
		// Prepares the vector that will hold the new peptide objects.
		std::vector<Peptide> peptides;
		peptides.reserve(CountSubstrings(buffer, "<Peptide"));

		// Acquires the initial peptide.
		size_t peptide_previous = 0;
		size_t peptide_start = buffer.find("<Peptide");
		size_t peptide_end = buffer.find("</Peptide>", peptide_start);

		// Only parses a peptide if all of its information is present within the buffer.
		while (peptide_start != std::string::npos && peptide_end != std::string::npos)
		{
			size_t id_position = buffer.find("id=", peptide_start);
			size_t sequence_position = buffer.find("PeptideSequence", id_position);

			// Collects the modifications.
			std::vector<Modification> modifications;
			size_t modification_position = buffer.find("<Modification", sequence_position);
			while (modification_position < peptide_end && modification_position != std::string::npos)
			{
				// Acquires the various information present regarding the modifications.
				std::string reference(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("cvRef", modification_position)));
				std::string name(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("name", modification_position)));

				modifications.push_back({
					(unsigned short)stoi(GetEmbeddedCharacters(buffer, ':', '"', buffer.find("accession", modification_position))),
					(unsigned short)stoi(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("location", modification_position))),
					(double)stod(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("monoisotopicMassDelta", modification_position))),
					&m_return_object$.cv_references.insert({ m_hasher_(reference), reference }).first->second,
					&m_return_object$.modification_names.insert({ m_hasher_(name), name }).first->second,
					(char)GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("residues", modification_position))[0]
				});

				modification_position = buffer.find("<Modification", modification_position + 1);
			}

			// Creates and pushes the peptide struct.
			peptides.push_back({
				GetEmbeddedCharacters(buffer, m_brackets_, id_position),
				GetEmbeddedCharacters(buffer, '>','<', sequence_position),
				std::move(modifications)
			});

			// Searches for the next peptide.
			peptide_previous = peptide_end;
			peptide_start = buffer.find("<Peptide", peptide_end);
			peptide_end = buffer.find("</Peptide>", peptide_start);
		}

		// Merges the vector with the existing map.
		m_return_object$.peptides.reserve(m_return_object$.peptides.size() + peptides.size());
		for (Peptide peptide : peptides)
		{
			m_return_object$.peptides.insert({ m_hasher_(peptide.id), peptide });
		}

		// Trims the buffer so it only includes unparsed characters.
		buffer = buffer.substr(buffer.find('>', peptide_previous));

		return buffer.find("<PeptideEvidence") != std::string::npos;
	}

	bool Mzid_Parser::ParseSequences_(std::string& buffer)
	{
		// Prepares the vector that will hold the new sequence objects.
		std::vector<Sequence> sequences;
		sequences.reserve(CountSubstrings(buffer, "<DBS"));

		// Acquires the initial sequence.
		size_t sequence_previous = 0;
		size_t sequence_start = buffer.find("<DBS");
		size_t sequence_end = buffer.find("</DBS", sequence_start);

		// Checks if the value information needs to be parsed as well.
		bool value_present = true;
		size_t description_position = buffer.find("ue=", sequence_start, sequence_end - sequence_start);
		if (description_position == std::string::npos)
		{
			value_present = false;
		}

		// Only parses a sequence if all of its information is present within the buffer.
		while (sequence_start != std::string::npos && sequence_end != std::string::npos)
		{
			// Acquires the positions of the id information.
			size_t id_position = buffer.find("id", sequence_start);

			// Tries to acquire the description.
			std::string description;
			if (value_present)
			{
				description = GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("ue=", sequence_start, sequence_end - sequence_start));
			}

			// Acquires the information and places it into a struct, which is passed into the vector.
			sequences.push_back({
				std::move(description),
				GetEmbeddedCharacters(buffer, m_brackets_, id_position) });

			// Searches for the next sequence.
			sequence_previous	= sequence_end;
			sequence_start		= buffer.find("<DBS", sequence_end);
			sequence_end		= buffer.find("</DBS", sequence_start);
		}

		// Merges the vector with the existing vector.
		m_return_object$.sequences.reserve(m_return_object$.sequences.size() + sequences.size());
		for (Sequence sequence : sequences)
		{
			m_return_object$.sequences.insert({ m_hasher_(sequence.id), sequence });
		}

		// Trims the buffer so it only includes unparsed characters.
		size_t end_marker = buffer.find('>', sequence_previous);
		if (end_marker != std::string::npos)
		{
			buffer = buffer.substr(end_marker);
		}
		else
		{
			buffer = buffer.substr(sequence_previous);
		}

		// Returns true if this part of the buffer contained the sequence collection end tag.
		return buffer.find("<Peptide") != std::string::npos;
	}

	bool Mzid_Parser::ParseSpectrumIdentificationResults_(std::string& buffer)
	{
		size_t result_previous = 0;
		size_t result_start = buffer.find("<SpectrumIdentificationResult");
		size_t result_end = buffer.find("</SpectrumIdentificationResult", result_start);

		while (result_start != std::string::npos && result_end != std::string::npos)
		{
			// Acquires the positions for the required values.
			size_t file_position		= buffer.find("spectraData_ref", result_start) + 15;
			size_t index_position		= buffer.find("spectrumID", result_start) + 14;
			size_t id_position			= buffer.find("id", result_start) + 2;
			size_t scan_position		= buffer.find("scan", result_start);
			size_t retention_position	= buffer.find("retention time", result_start);

			// Calculates the scan and retention time values.
			size_t scan = (size_t)-1;
			double retention_time = 0;

			if (scan_position != std::string::npos)
			{
				scan = std::stoi(GetEmbeddedCharacters(buffer, '=', '&', scan_position));
			}

			if (retention_position != std::string::npos)
			{
				retention_time = std::atof(GetEmbeddedCharacters(buffer, m_brackets_, retention_position + 15).c_str());
			}

			// Acquires the iterator to a file entry, or creates a new one.
			auto file_iterator = m_return_object$.file_sorted_identifications.insert(
			{
				GetEmbeddedCharacters(buffer, m_brackets_, file_position),
				std::vector<SpectrumIdentificationResult*>()
			}).first;

			// Adds a new result to the identifications map.
			std::string id(GetEmbeddedCharacters(buffer, m_brackets_, id_position));

			// If any errors occur during processing of the identification result, either ignore the result or throw a runtime error.
			try
			{
				auto result_iterator = m_return_object$.identification_results.insert(
				{ 
					m_hasher_(id),
					{
						id,
						size_t(stoi(GetEmbeddedCharacters(buffer, '=', '"', index_position))),
						retention_time,
						scan,
						ParseSpectrumIdentificationItem_(buffer, result_start, result_end)
					}
				}).first;

				// Adds the identification to the file vector.
				file_iterator->second.push_back(&result_iterator->second);
			}
			catch (std::runtime_error& error)
			{
				if (!m_ignore_file_errors_)
				{
					throw std::runtime_error("Error during the parsing of result: " + id + "\n" + std::string(error.what()));
				}
			}

			result_previous = result_end;
			result_start = buffer.find("<SpectrumIdentificationResult", result_end);
			result_end = buffer.find("</SpectrumIdentificationResult>", result_start);
		}

		// Trims the buffer so it only includes unparsed characters.
		buffer = buffer.substr(buffer.find('>', result_previous));

		return buffer.find("</SpectrumIdentificationList>") != std::string::npos;
	}

	// Extracts the item information from the passed string.
	std::vector<SpectrumIdentificationItem*> Mzid_Parser::ParseSpectrumIdentificationItem_(std::string& buffer, size_t start_position, size_t end_position)
	{
		// Parses the Identification Items within the range of the current Identification Result.
		std::vector<SpectrumIdentificationItem> parsed_items;
		size_t item_begin = buffer.find("<SpectrumIdentificationItem", start_position);
		size_t item_end = buffer.find("</SpectrumIdentificationItem", item_begin);
		while (item_begin != std::string::npos && item_end < end_position)
		{
			// Acquires all the positions for the required variables.
			size_t passed_position = buffer.find("passThreshold", item_begin);
			size_t rank_position = buffer.find("rank", item_begin);
			size_t peptide_reference_position = buffer.find("peptide_ref", item_begin);
			size_t calculated_mz_position = buffer.find("calculatedMassToCharge", item_begin);
			size_t experimental_mz_position = buffer.find("experimentalMassToCharge", item_begin);
			size_t chargestate_position = buffer.find("chargeState", item_begin);
			size_t id_position = buffer.find("id=", item_begin);

			// Attempts to find the peptide, throws an error if it is unable to.
			auto peptide_entry = m_return_object$.peptides.find(m_hasher_(GetEmbeddedCharacters(buffer, m_brackets_, peptide_reference_position)));
			if (!m_ignore_file_errors_ && peptide_entry == m_return_object$.peptides.end())
			{
				throw std::runtime_error("Couldn't locate peptide with the id: " + GetEmbeddedCharacters(buffer, m_brackets_, peptide_reference_position));
			}

			// Extracts the data from the string, pushes the new Identification Item into the vector..
			parsed_items.push_back(
			{
				GetEmbeddedCharacters(buffer, m_brackets_, id_position),
				(unsigned char)std::stoi(GetEmbeddedCharacters(buffer, m_brackets_, chargestate_position)),
				std::atof(GetEmbeddedCharacters(buffer, m_brackets_, calculated_mz_position).c_str()),
				std::atof(GetEmbeddedCharacters(buffer, m_brackets_, experimental_mz_position).c_str()),
				"true" == GetEmbeddedCharacters(buffer, m_brackets_, passed_position),
				(unsigned short)std::stoi(GetEmbeddedCharacters(buffer, m_brackets_, rank_position)),
				&peptide_entry->second,
				ParseCV_Parameters_(buffer, item_begin, item_end),
				ParseSpectrumEvidences_(buffer, item_begin, item_end),
				ParseSpectrumIons_(buffer, item_begin, item_end)
			});

			int a = 10;
			unsigned int b = (unsigned int)a;
			unsigned int c(a);


			// Attempts to find additional identification items.
			item_begin = buffer.find("<SpectrumIdentificationItem", item_end);
			item_end = buffer.find("</SpectrumIdentificationItem", item_begin);
		}

		// Inserts the Identification Items into the Mzid_File object and creates the pointer return vector.
		std::vector<SpectrumIdentificationItem*> inserted_items;
		inserted_items.reserve(parsed_items.size());
		for (SpectrumIdentificationItem& item : parsed_items)
		{
			inserted_items.push_back(&m_return_object$.identification_items.insert({ m_hasher_(item.id), item }).first->second);
		}

		return inserted_items;
	}

	std::vector<CV_Parameter> Mzid_Parser::ParseCV_Parameters_(std::string& buffer, size_t start_position, size_t end_position)
	{
		std::vector<CV_Parameter> parameters;
		size_t parameter_position = buffer.find("cvParam", start_position);
		parameter_position = buffer.find("name", parameter_position);
		while (parameter_position != std::string::npos && parameter_position < end_position)
		{
			try
			{
				std::string value(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("value=", parameter_position)));
				double converted_value = 0;
				if (value == "true")
				{
					// Variable value is compared to "true" and results in a boolean.
					converted_value = value == "true";
				}
				else
				{
					converted_value = std::atof(value.c_str());
				}

				parameters.push_back(
				{
					*m_return_object$.cv_param_names.insert(GetEmbeddedCharacters(buffer, m_brackets_, parameter_position)).first,
					converted_value
				});
			}
			catch (...)
			{
				// Ignore value if an exception is thrown.
			}

			parameter_position = buffer.find("name", parameter_position+1);
		}

		return parameters;
	}

	std::vector<PeptideEvidence*> Mzid_Parser::ParseSpectrumEvidences_(std::string& buffer, size_t start_position, size_t end_position)
	{
		std::vector<PeptideEvidence*> evidences;

		size_t evidences_position = buffer.find("peptideEvidence_ref", start_position) + 20;
		while (evidences_position != std::string::npos && evidences_position < end_position)
		{
			// If the peptide evidence cannot be located, throw error.
			auto evidence_entry = m_return_object$.peptide_evidence.find(m_hasher_(GetEmbeddedCharacters(buffer, m_brackets_, evidences_position)));
			if (evidence_entry == m_return_object$.peptide_evidence.end())
			{
				throw std::runtime_error("Couldn't locate peptide evidence with id: " + GetEmbeddedCharacters(buffer, m_brackets_, evidences_position));
			}
			else
			{
				evidences.push_back(&evidence_entry->second);
			}

			evidences_position = buffer.find("peptideEvidence_ref", evidences_position + 20);
		}

		return evidences;
	}

	std::vector<Iontype> Mzid_Parser::ParseSpectrumIons_(std::string& buffer, size_t start_position, size_t end_position)
	{
		std::vector<Iontype> ions;

		size_t iontype_position = buffer.find("<IonType", start_position);
		while (iontype_position != std::string::npos && iontype_position < end_position)
		{
			// Extracts the information.
			std::vector<std::string> mz_strings = Split(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("Measure_MZ", iontype_position) + 18), ' ');
			std::vector<std::string> int_strings = Split(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("Measure_Int", iontype_position) + 19), ' ');
			std::vector<std::string> error_strings = Split(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("Measure_Error", iontype_position) + 21), ' ');

			// Converts the information into boolean vectors.
			std::vector<double> mz_values;
			mz_values.reserve(mz_strings.size());
			for (std::string& value : mz_strings)
			{
				mz_values.push_back(std::atof(value.c_str()));
			}

			std::vector<double> int_values;
			int_values.reserve(int_strings.size());
			for (std::string& value : int_strings)
			{
				int_values.push_back(std::atof(value.c_str()));
			}

			std::vector<double> error_values;
			error_values.reserve(error_strings.size());
			for (std::string& value : error_strings)
			{
				error_values.push_back(std::atof(value.c_str()));
			}

			// Creates an ion object and pushes this into a vector.
			ions.push_back({
				GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("index", iontype_position) + 4),
				GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("name", iontype_position) + 3),
				(unsigned char)std::stoi(GetEmbeddedCharacters(buffer, m_brackets_, buffer.find("charge", iontype_position) + 5)),
				mz_values,
				int_values,
				error_values
			});

			iontype_position = buffer.find("<IonType", iontype_position + 1);
		}

		return ions;
	}
}
