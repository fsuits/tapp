// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Todo: Rename sequence to fasta or database sequence.

namespace TAPP::Filetypes::Mzid // C++ 17 will provide a better way for nesting. Example: Data::Mzid
{
	/*** Typedefs ***************************************************************/

	typedef std::pair<const std::string&, double> CV_Parameter; // Links a confidence to a tool name.

	/*** Structs ****************************************************************/

	struct Modification
	{
		unsigned short								accession;
		unsigned short								location;
		double										monoisotopicMassDelta;
		std::string*								cv_reference;
		std::string*								name;
		char										residues;
	};

	struct Peptide
	{
		std::string									id;
		std::string									sequence;
		std::vector<Modification>					modifications;
	};

	struct Sequence
	{
		std::string									description;
		std::string									id;
	};

	struct PeptideEvidence
	{
		unsigned short								end;
		unsigned short								start;
		std::string									id;
		bool										is_decoy;
		char										post;
		char										pre;
		Peptide*									peptide_reference;
		Sequence*									sequence_reference;
	};

	struct Iontype
	{
		std::string									indices;
		std::string									name;
		unsigned char								charge;
		std::vector<double>							mz_values;
		std::vector<double>							intensity_values;
		std::vector<double>							error_values;
	};

	struct SpectrumIdentificationItem
	{
		std::string									id;
		unsigned char								chargestate;
		double										calculated_mz;
		double										experimental_mz;
		bool										passed;
		unsigned short								rank;
		Peptide*									peptide_reference;
		std::vector<CV_Parameter>					cv_parameters;
		std::vector<PeptideEvidence*>				peptide_evidences;
		std::vector<Iontype>						fragmentations;
	};

	struct SpectrumIdentificationResult
	{
		std::string									id;
		size_t										index;
		double										retention_time;
		size_t										scan_id;
		std::vector<SpectrumIdentificationItem*>	items;
	};

	struct ProteinHypothesis
	{
		std::string									id;
		bool										passed_threshold;
		PeptideEvidence*							evidence;
		Sequence*									sequence;
		std::vector<SpectrumIdentificationItem*>	items;
	};

	struct ProteinGroup
	{
		std::string									id;
		std::vector<CV_Parameter>					cv_parameters;
		std::vector<ProteinHypothesis>				hypotheses;
	};

	/*** Mzid File Definition ***************************************************/

	struct Mzid_File
	{
		typedef std::vector<std::pair<size_t, SpectrumIdentificationResult*>>		ScanAnnotatedIdentificationResults; // std::vector<std::pair<size_t, SpectrumIdentificationResult*>>

		Mzid_File(void);								// Default Constructor
		Mzid_File(const Mzid_File& other);				// Copy Constructor
		Mzid_File(Mzid_File&& other);					// Move Constructor
		~Mzid_File(void);								// Destructor

		Mzid_File& operator=(const Mzid_File& other);	// Copy Assignment
		Mzid_File& operator=(Mzid_File&& other);		// Move Assignment

		std::string																	filename;
		std::unordered_map<size_t, std::string>										cv_references;
		std::unordered_map<std::string, std::vector<SpectrumIdentificationResult*>>	file_sorted_identifications;
		std::unordered_map<size_t, SpectrumIdentificationItem>						identification_items;
		std::unordered_map<size_t, SpectrumIdentificationResult>					identification_results;
		std::unordered_map<size_t, std::string>										modification_names;
		std::unordered_map<size_t, Peptide>											peptides;
		std::unordered_map<size_t, PeptideEvidence>									peptide_evidence;
		std::unordered_map<size_t, ProteinGroup>									protein_groups;
		std::unordered_map<size_t, Sequence>										sequences;
		std::unordered_set<std::string>												cv_param_names;

		ScanAnnotatedIdentificationResults GetScanAnnotatedIdentificationResults(const std::string& filename);
	};
}