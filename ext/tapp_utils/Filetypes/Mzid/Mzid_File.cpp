// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#include "Filetypes/Mzid/Mzid_File.h"

#include <functional>
#include <stdexcept>

namespace TAPP::Filetypes::Mzid
{
	/*** Constructors / Destructor **********************************************/

	// Default Constructor
	Mzid_File::Mzid_File(void)
	{
	}

	// Copy Constructor
	Mzid_File::Mzid_File(const Mzid_File& other) : filename(other.filename), cv_references(other.cv_references), file_sorted_identifications(other.file_sorted_identifications),
		identification_items(other.identification_items), identification_results(other.identification_results), modification_names(other.modification_names), peptides(other.peptides),
		peptide_evidence(other.peptide_evidence), protein_groups(other.protein_groups), sequences(other.sequences), cv_param_names(other.cv_param_names)
	{
		// Creates a hasher to recover references.
		std::hash<std::string> hasher;

		// Sets the pointers of the peptide modifications.
		for (const auto& other_peptide : other.peptides)
		{
			auto this_peptide = peptides.find(other_peptide.first);
			for (size_t mod = 0; mod < this_peptide->second.modifications.size(); ++mod)
			{
				this_peptide->second.modifications[mod].cv_reference = &cv_references.find(hasher(*other_peptide.second.modifications[mod].cv_reference))->second;
				this_peptide->second.modifications[mod].name = &modification_names.find(hasher(*other_peptide.second.modifications[mod].name))->second;
			}
		}

		// Sets the pointers of the peptide evidence.
		for (const auto& other_evidence : other.peptide_evidence)
		{
			auto this_evidence = peptide_evidence.find(other_evidence.first);

			this_evidence->second.peptide_reference = &peptides.find(hasher(other_evidence.second.peptide_reference->id))->second;
			this_evidence->second.sequence_reference = &sequences.find(hasher(other_evidence.second.sequence_reference->id))->second;
		}

		// Sets the pointers of the identification item.
		for (const auto& other_item : other.identification_items)
		{
			auto this_item = identification_items.find(other_item.first);

			this_item->second.peptide_reference = &peptides.find(hasher(other_item.second.peptide_reference->id))->second;

			this_item->second.peptide_evidences.clear();

			for (const PeptideEvidence* evidence : other_item.second.peptide_evidences)
			{
				this_item->second.peptide_evidences.push_back(&peptide_evidence.find(hasher(evidence->id))->second);
			}
		}

		// Sets the pointers of the identification result.
		for (const auto& other_result : other.identification_results)
		{
			auto this_result = identification_results.find(other_result.first);

			this_result->second.items.clear();

			for (const SpectrumIdentificationItem* item : other_result.second.items)
			{
				this_result->second.items.push_back(&identification_items.find(hasher(item->id))->second);
			}
		}

		// Sets the pointers of the protein hypothesis.
		for (const auto& other_group : other.protein_groups)
		{
			auto this_group = protein_groups.find(other_group.first);

			this_group->second.cv_parameters.clear();

			for (const auto& parameter : other_group.second.cv_parameters)
			{
				this_group->second.cv_parameters.push_back({ *cv_param_names.find(parameter.first), parameter.second });
			}

			for (size_t hypothesis = 0; hypothesis < this_group->second.hypotheses.size(); ++hypothesis)
			{
				this_group->second.hypotheses[hypothesis].evidence = &peptide_evidence.find(hasher(other_group.second.hypotheses[hypothesis].evidence->id))->second;
				this_group->second.hypotheses[hypothesis].sequence = &sequences.find(hasher(other_group.second.hypotheses[hypothesis].sequence->id))->second;

				this_group->second.hypotheses[hypothesis].items.clear();

				for (const SpectrumIdentificationItem* item : other_group.second.hypotheses[hypothesis].items)
				{
					this_group->second.hypotheses[hypothesis].items.push_back(&identification_items.find(hasher(item->id))->second);
				}
			}
		}
	}

	// Move Constructor
	Mzid_File::Mzid_File(Mzid_File&& other) : filename(std::move(other.filename)), cv_references(std::move(other.cv_references)), file_sorted_identifications(std::move(other.file_sorted_identifications)),
		identification_items(std::move(other.identification_items)), identification_results(std::move(other.identification_results)), modification_names(std::move(other.modification_names)),
		peptides(std::move(other.peptides)), peptide_evidence(std::move(other.peptide_evidence)), protein_groups(std::move(other.protein_groups)), sequences(std::move(other.sequences)),
		cv_param_names(std::move(other.cv_param_names))
	{
	}

	// Destructor
	Mzid_File::~Mzid_File(void)
	{
	}

	/*** Operators **************************************************************/

	// Copy Assignment
	Mzid_File& Mzid_File::operator=(const Mzid_File& other)
	{
		filename					= other.filename;
		cv_references				= other.cv_references;
		file_sorted_identifications	= other.file_sorted_identifications;
		identification_items		= other.identification_items;
		identification_results		= other.identification_results;
		modification_names			= other.modification_names;
		peptides					= other.peptides;
		peptide_evidence			= other.peptide_evidence;
		protein_groups				= other.protein_groups;
		sequences					= other.sequences;
		cv_param_names				= other.cv_param_names;

		// Creates a hasher to recover references.
		std::hash<std::string> hasher;

		// Sets the pointers of the peptide modifications.
		for (const auto& other_peptide : other.peptides)
		{
			auto this_peptide = peptides.find(other_peptide.first);
			for (size_t mod = 0; mod < this_peptide->second.modifications.size(); ++mod)
			{
				this_peptide->second.modifications[mod].cv_reference = &cv_references.find(hasher(*other_peptide.second.modifications[mod].cv_reference))->second;
				this_peptide->second.modifications[mod].name = &modification_names.find(hasher(*other_peptide.second.modifications[mod].name))->second;
			}
		}

		// Sets the pointers of the peptide evidence.
		for (const auto& other_evidence : other.peptide_evidence)
		{
			auto this_evidence = peptide_evidence.find(other_evidence.first);

			this_evidence->second.peptide_reference = &peptides.find(hasher(other_evidence.second.peptide_reference->id))->second;
			this_evidence->second.sequence_reference = &sequences.find(hasher(other_evidence.second.sequence_reference->id))->second;
		}

		// Sets the pointers of the identification item.
		for (const auto& other_item : other.identification_items)
		{
			auto this_item = identification_items.find(other_item.first);

			this_item->second.peptide_reference = &peptides.find(hasher(other_item.second.peptide_reference->id))->second;

			this_item->second.peptide_evidences.clear();

			for (const PeptideEvidence* evidence : other_item.second.peptide_evidences)
			{
				this_item->second.peptide_evidences.push_back(&peptide_evidence.find(hasher(evidence->id))->second);
			}
		}

		// Sets the pointers of the identification result.
		for (const auto& other_result : other.identification_results)
		{
			auto this_result = identification_results.find(other_result.first);

			this_result->second.items.clear();

			for (const SpectrumIdentificationItem* item : other_result.second.items)
			{
				this_result->second.items.push_back(&identification_items.find(hasher(item->id))->second);
			}
		}

		// Sets the pointers of the protein hypothesis.
		for (const auto& other_group : other.protein_groups)
		{
			auto this_group = protein_groups.find(other_group.first);

			this_group->second.cv_parameters.clear();

			for (const auto& parameter : other_group.second.cv_parameters)
			{
				this_group->second.cv_parameters.push_back({ *cv_param_names.find(parameter.first), parameter.second });
			}

			for (size_t hypothesis = 0; hypothesis < this_group->second.hypotheses.size(); ++hypothesis)
			{
				this_group->second.hypotheses[hypothesis].evidence = &peptide_evidence.find(hasher(other_group.second.hypotheses[hypothesis].evidence->id))->second;
				this_group->second.hypotheses[hypothesis].sequence = &sequences.find(hasher(other_group.second.hypotheses[hypothesis].sequence->id))->second;

				this_group->second.hypotheses[hypothesis].items.clear();

				for (const SpectrumIdentificationItem* item : other_group.second.hypotheses[hypothesis].items)
				{
					this_group->second.hypotheses[hypothesis].items.push_back(&identification_items.find(hasher(item->id))->second);
				}
			}
		}

		return *this;
	}

	// Move Assignment
	Mzid_File& Mzid_File::operator=(Mzid_File&& other)
	{
		filename.swap(other.filename);
		cv_references.swap(other.cv_references);
		file_sorted_identifications.swap(other.file_sorted_identifications);
		identification_items.swap(other.identification_items);
		identification_results.swap(other.identification_results);
		modification_names.swap(other.modification_names);
		peptides.swap(other.peptides);
		peptide_evidence.swap(other.peptide_evidence);
		protein_groups.swap(other.protein_groups);
		sequences.swap(other.sequences);
		cv_param_names.swap(other.cv_param_names);

		other.filename.clear();
		other.cv_references.clear();
		other.file_sorted_identifications.clear();
		other.identification_items.clear();
		other.identification_results.clear();
		other.modification_names.clear();
		other.peptides.clear();
		other.peptide_evidence.clear();
		other.protein_groups.clear();
		other.sequences.clear();
		other.cv_param_names.clear();

		return *this;
	}

	/*** Public Functions *******************************************************/

	Mzid_File::ScanAnnotatedIdentificationResults Mzid_File::GetScanAnnotatedIdentificationResults(const std::string& filename)
	{
		auto file_iterator = file_sorted_identifications.find(filename);

		if (file_sorted_identifications.size() == 1)
		{
			file_iterator = file_sorted_identifications.begin();
		}

		if (file_iterator != file_sorted_identifications.end())
		{
			ScanAnnotatedIdentificationResults pairings;
			pairings.reserve(file_iterator->second.size());

			for (SpectrumIdentificationResult* result : file_iterator->second)
			{
				pairings.push_back({ result->scan_id, result });
			}

			return pairings;
		}
		else
		{
			throw std::runtime_error("Filename isn't present within this Mzid file.");
		}
	}
}
