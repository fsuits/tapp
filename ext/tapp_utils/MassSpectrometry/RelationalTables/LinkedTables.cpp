// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#include "MassSpectrometry/RelationalTables/LinkedTables.h"

#include <algorithm>
#include <functional>

#include "MassSpectrometry/RelationalTables/TableInitialization.h"

using namespace std::placeholders;
using namespace TAPP::Filetypes::TAPP;

namespace TAPP::MassSpectrometry::RelationalTables
{
	/*** Constructors / Destructor **********************************************/

	LinkedTables::LinkedTables(void)
	{
	}

	LinkedTables::LinkedTables(const std::vector<PID>& pid_entries, const TAPP::Filetypes::Mzid::SpectrumIdentificationItem* item_example, const TAPP::Filetypes::Mzid::ProteinGroup* group_example, const char delimiter_char, const char invalid_char)
		: m_delimiter_(delimiter_char), m_invalid_(invalid_char)
	{
		// Initializes the tables.
		/*** MZXML ***/

		 m_event_table					= InitializeEventTable("events");
		 m_scan_table					= InitializeScanTable("scans");

		/*** MZID ***/

		m_fasta_sequence_table			= InitializeFastaSequenceTable("fasta_sequences");
		m_fragment_ion_table			= InitializeFragmentIonTable("fragment_ions", delimiter_char);
		m_identification_item_table		= InitializeIdentificationItemTable("identification_items", item_example, delimiter_char);
		m_identification_result_table	= InitializeIdentificationResultTable("identification_results", delimiter_char, invalid_char);
		m_peptide_table					= InitializePeptideTable("peptides", delimiter_char);
		m_peptide_evidence_table		= InitializePeptideEvidenceTable("peptide_evidence");
		m_peptide_modification_table	= InitializePeptideModificationTable("peptide_modifications");
		m_protein_group_table			= InitializeProteinGroupTable("protein_groups", group_example, delimiter_char);
		m_protein_hypothesis_table		= InitializeProteinHypothesisTable("protein_hypothesis", delimiter_char);

		/*** TAPP ***/

		m_cluster_table					= InitializeIsotopicClusterTable("clusters", delimiter_char, invalid_char);
		m_file_table					= InitializeFileTable("files");
		m_metapeak_table				= InitializeMetapeakTable("metapeaks", delimiter_char);
		m_peak_table					= InitializePeakTable("peaks");

		InsertPID_(pid_entries);
	}

	LinkedTables::LinkedTables(LinkedTables&& other) : m_event_table(std::move(other.m_event_table)), m_scan_table(std::move(other.m_scan_table)), m_fasta_sequence_table(std::move(other.m_fasta_sequence_table)),
	m_fragment_ion_table(std::move(other.m_fragment_ion_table)), m_identification_item_table(std::move(other.m_identification_item_table)),	m_identification_result_table(std::move(other.m_identification_result_table)),
	m_peptide_table(std::move(other.m_peptide_table)), m_peptide_evidence_table(std::move(other.m_peptide_evidence_table)),	m_peptide_modification_table(std::move(other.m_peptide_modification_table)),
	m_protein_group_table(std::move(other.m_protein_group_table)), m_protein_hypothesis_table(std::move(other.m_protein_hypothesis_table)),	m_cluster_table(std::move(other.m_cluster_table)),
	m_file_table(std::move(other.m_file_table)), m_metapeak_table(std::move(other.m_metapeak_table)), m_peak_table(std::move(other.m_peak_table)), m_delimiter_(other.m_delimiter_),
	m_invalid_(other.m_invalid_), m_cv_references_(std::move(other.m_cv_references_)), m_modification_names_(std::move(other.m_modification_names_)), m_cv_param_names_(std::move(other.m_cv_param_names_)),
	m_property_vectors_(std::move(other.m_property_vectors_))
	{
	}

	LinkedTables::~LinkedTables(void)
	{
	}

	LinkedTables& LinkedTables::operator=(LinkedTables&& other)
	{
		// Swaps the current information.
		m_event_table = std::move((other.m_event_table));
		m_scan_table = std::move((other.m_scan_table));
		m_fasta_sequence_table = std::move((other.m_fasta_sequence_table));
		m_fragment_ion_table = std::move((other.m_fragment_ion_table));
		m_identification_item_table = std::move((other.m_identification_item_table));
		m_identification_result_table = std::move((other.m_identification_result_table));
		m_peptide_table = std::move((other.m_peptide_table));
		m_peptide_evidence_table = std::move((other.m_peptide_evidence_table));
		m_peptide_modification_table = std::move((other.m_peptide_modification_table));
		m_protein_group_table = std::move((other.m_protein_group_table));
		m_protein_hypothesis_table = std::move((other.m_protein_hypothesis_table));
		m_cluster_table = std::move((other.m_cluster_table));
		m_file_table = std::move((other.m_file_table));
		m_metapeak_table = std::move((other.m_metapeak_table));
		m_peak_table = std::move((other.m_peak_table));

		m_delimiter_ = other.m_delimiter_;
		m_invalid_ = other.m_invalid_;
		m_cv_references_ = std::move((other.m_cv_references_));
		m_modification_names_ = std::move((other.m_modification_names_));
		m_cv_param_names_ = std::move((other.m_cv_param_names_));
		m_property_vectors_ = std::move((other.m_property_vectors_));

		// Cleans the information contained in the other object, just in case.
		other.m_event_table.clear();
		other.m_scan_table.clear();
		other.m_fasta_sequence_table.clear();
		other.m_fragment_ion_table.clear();
		other.m_identification_item_table.clear();
		other.m_identification_result_table.clear();
		other.m_peptide_table.clear();
		other.m_peptide_evidence_table.clear();
		other.m_peptide_modification_table.clear();
		other.m_protein_group_table.clear();
		other.m_protein_hypothesis_table.clear();
		other.m_cluster_table.clear();
		other.m_file_table.clear();
		other.m_metapeak_table.clear();
		other.m_peak_table.clear();

		other.m_delimiter_ = 0;
		other.m_invalid_ = 0;
		other.m_cv_references_.clear();
		other.m_modification_names_.clear();
		other.m_cv_param_names_.clear();
		other.m_property_vectors_.clear();

		return *this;
	}


	/*** Public Functions *******************************************************/

	void LinkedTables::InsertIdentificationTables(const size_t file_id, const std::vector<IPL>& ipl_entries, const Mzid_File& mzid_file)
	{
		//used to hash string ids.
		std::hash<std::string> hasher;

		/*** FASTA SEQUENCES *************************************/

		// Transfers the fasta sequences.
		std::unordered_map<size_t, FastaSequence*> sequence_map;
		sequence_map.reserve(mzid_file.sequences.size());
		for (auto sequence : mzid_file.sequences)
		{
			// Inserts the sequence into the table.
			auto sequence_iterator = m_fasta_sequence_table.insert(
			{
				sequence.second.description,
				sequence.second.id,
				std::vector<PeptideEvidence*>(),
				std::vector<ProteinHypothesis*>()
			});

			// Maps the new sequence.
			sequence_map.insert({ hasher(sequence.second.id), &sequence_iterator.first->second });
		}



		/*** PEPTIDES ********************************************/

		// Transfers the peptides.
		std::unordered_map<size_t, Peptide*> peptide_map;
		peptide_map.reserve(mzid_file.peptides.size());
		for (auto peptide : mzid_file.peptides)
		{
			// Inserts the peptide_modifications
			std::vector<PeptideModification*> peptide_modifications;
			for (auto peptide_modification : peptide.second.modifications)
			{
				m_peptide_modification_table.insert(
				{
					peptide_modification.accession,
					peptide_modification.location,
					peptide_modification.monoisotopicMassDelta,
					peptide_modification.residues,
					&m_cv_references_.insert({ hasher(*peptide_modification.cv_reference), *peptide_modification.cv_reference }).first->second,
					&m_modification_names_.insert({hasher(*peptide_modification.name), *peptide_modification.name }).first->second,
					nullptr
				});
			}

			// Inserts the peptide into the table.
			auto peptide_iterator = m_peptide_table.insert(
			{
				peptide.second.id,
				peptide.second.sequence,
				peptide_modifications,
				std::vector<IdentificationItem*>(),
				std::vector<PeptideEvidence*>()
			});

			for (auto peptide_modification : peptide_iterator.first->second.modifications)
			{
				peptide_modification->peptide_relation = &peptide_iterator.first->second;
			}

			peptide_map.insert({hasher(peptide.second.id), &peptide_iterator.first->second});
		}



		/*** PEPTIDE EVIDENCE ************************************/

		// Transfers the peptide evidence.
		std::unordered_map<size_t, PeptideEvidence*> evidence_map;
		evidence_map.reserve(mzid_file.peptide_evidence.size());
		for (auto evidence : mzid_file.peptide_evidence)
		{
			// Inserts the sequence into the table.
			auto evidence_iterator = m_peptide_evidence_table.insert(
			{
				evidence.second.id,
				evidence.second.start,
				evidence.second.end,
				evidence.second.pre,
				evidence.second.post,
				evidence.second.is_decoy,
				peptide_map.find(hasher(evidence.second.peptide_reference->id))->second,
				sequence_map.find(hasher(evidence.second.sequence_reference->id))->second,
				std::vector<IdentificationItem*>(),
				std::vector<ProteinHypothesis*>()
			});

			// Creates the new relations.
			peptide_map.find(hasher(evidence.second.peptide_reference->id))->second->peptide_evidence_relations.push_back(&evidence_iterator.first->second);
			sequence_map.find(hasher(evidence.second.sequence_reference->id))->second->evidence_relations.push_back(&evidence_iterator.first->second);

			// Maps the new sequence.
			evidence_map.insert({ hasher(evidence.second.id), &evidence_iterator.first->second });
		}



		/*** IDENTIFICATIOns *************************************/

		// Maps the scans for this file.
		File& file_reference = m_file_table.find(file_id)->second;
		std::unordered_map<size_t, Scan*> scan_map;
		scan_map.reserve(file_reference.scan_relations.size());

		for (auto scan : file_reference.scan_relations)
		{
			scan_map.insert({ scan->scan_id, scan });
		}

		// Reserves space for the identification results.
		file_reference.identification_result_relations.reserve(mzid_file.identification_results.size());

		// Copies the CV Param names from the mzid file.
		m_cv_param_names_.insert(mzid_file.cv_param_names.begin(), mzid_file.cv_param_names.end());

		// Transfers the identifcation results and items.
		std::unordered_map<size_t, IdentificationResult*> result_map;
		std::unordered_map<size_t, IdentificationItem*> item_map;
		result_map.reserve(mzid_file.identification_results.size());
		item_map.reserve(mzid_file.identification_items.size());
		
		// Loops through all the identification results.
		for (auto result : mzid_file.identification_results)
		{
			// Creates a vector for the items.
			std::vector<IdentificationItem*> item_vector;
			item_vector.reserve(result.second.items.size());

			// Loops through the items inside the current result.
			for (auto item : result.second.items)
			{
				// Loops through the CV_Params and creates a new vector.
				std::vector<CV_Parameter> cv_parameters;
				cv_parameters.reserve(item->cv_parameters.size());
				for (CV_Parameter parameter : item->cv_parameters)
				{
					cv_parameters.push_back(
					{
						*m_cv_param_names_.find(parameter.first),
						parameter.second
					});
				}

				/*** FRAGMENT IONS ***************************************/

				// loops through the fragment ions inside the current item.
				std::vector<FragmentIon*> fragment_ions;
				fragment_ions.reserve(item->fragmentations.size());
				for (auto fragment : item->fragmentations)
				{
					// Inserts the fragment.
					auto fragment_iterator = m_fragment_ion_table.insert(
					{
						fragment.indices,
						fragment.name,
						fragment.charge,
						fragment.mz_values,
						fragment.intensity_values,
						fragment.error_values,
						nullptr
					});

					// Creates a relation towards the current item.
					fragment_ions.push_back(&fragment_iterator.first->second);
				}

				/*** PEPTIDE EVIDENCE ************************************/

				// Loops through the peptide evidence and creates a vector.
				std::vector<PeptideEvidence*> evidence_vector;
				evidence_vector.reserve(item->peptide_evidences.size());
				for (auto evidence : item->peptide_evidences)
				{
					evidence_vector.push_back(evidence_map.find(hasher(evidence->id))->second);
				}

				/*** IDENTIFICATION ITEMS ********************************/

				// Inserts the actual item.
				auto item_iterator = m_identification_item_table.insert(
				{
					item->id,
					item->chargestate,
					item->calculated_mz,
					item->experimental_mz,
					item->passed,
					item->rank,
					peptide_map.find(hasher(item->peptide_reference->id))->second,
					cv_parameters,
					fragment_ions,
					evidence_vector,
					nullptr,
					std::vector<ProteinHypothesis*>()
				});

				// Creates the relationships.
				peptide_map.
					find(hasher(item->peptide_reference->id))->
					second->
					identification_item_relations.push_back(&item_iterator.first->second);

				for (auto fragment : fragment_ions)
				{
					fragment->identification_item_relation = &item_iterator.first->second;
				}

				for (auto evidence : evidence_vector)
				{
					evidence->identification_item_relations.push_back(&item_iterator.first->second);
				}

				// Maps the item.
				item_map.insert({ hasher(item_iterator.first->second.id), &item_iterator.first->second });

				// Adds the item to the vector.
				item_vector.push_back(&item_iterator.first->second);
			}

			/*** IDENTIFICATION RESULTS ******************************/

			Scan* scan_pointer(scan_map.find(result.second.scan_id)->second);

			auto result_iterator = m_identification_result_table.insert(
			{
				result.second.id,
				result.second.index,
				result.second.retention_time,
				scan_pointer,
				item_vector,
				std::vector<IsotopicCluster*>()
			});

			// Creates the relationship between result and item.
			for (auto item : item_vector)
			{
				item->identification_result_relation = &result_iterator.first->second;
			}

			// Creates a relationship towards the current file.
			file_reference.identification_result_relations.push_back(&result_iterator.first->second);

			// Creates a relationship towards the scan.
			scan_pointer->identification_result_relation = &result_iterator.first->second;

			// Maps the new result.
			result_map.insert({ hasher(result.second.id), &result_iterator.first->second });
		}

		// Creates an Isotopic Cluster Map
		std::unordered_map<size_t, IsotopicCluster*> cluster_map;
		cluster_map.reserve(file_reference.isotopic_cluster_relations.size());

		for (IsotopicCluster* cluster : file_reference.isotopic_cluster_relations)
		{
			cluster_map.insert({ cluster->id, cluster });
		}

		for (const IPL& ipl : ipl_entries)
		{
			auto cluster = cluster_map.find(ipl.cluster_id);
			auto identification = result_map.find(hasher(ipl.identification_id));

			if (cluster != cluster_map.end() && identification != result_map.end())
			{
				cluster->second->identification = identification->second;
				identification->second->isotopic_cluster_relations.push_back(cluster->second);
			}
		}



		/*** PROTEINS ********************************************/

		// Transfers the protein group and hypothesis.
		std::unordered_map<size_t, ProteinGroup*> group_map;
		group_map.reserve(mzid_file.protein_groups.size());
		for (auto group : mzid_file.protein_groups)
		{
			// Creates a vector for the all the hypotheses.
			std::vector<ProteinHypothesis*> hypotheses_vector;
			hypotheses_vector.reserve(group.second.hypotheses.size());

			// Loops through all the hypothesis
			for (auto hypothesis : group.second.hypotheses)
			{
				// Creates a vector with the required items.
				std::vector<IdentificationItem*> items;
				items.reserve(hypothesis.items.size());
				for (auto item : hypothesis.items)
				{
					items.push_back(item_map.find(hasher(item->id))->second);
				}

				// Creates the hypthesis struct and adds it to the table.
				auto hypthesis_iterator = m_protein_hypothesis_table.insert(
				{
						hypothesis.id,
						hypothesis.passed_threshold,
						evidence_map.find(hasher(hypothesis.evidence->id))->second,
						sequence_map.find(hasher(hypothesis.sequence->id))->second,
						items,
						nullptr
				});

				for (IdentificationItem* item : hypthesis_iterator.first->second.identification_items)
				{
					item->protein_hypotheses_relations.push_back(&hypthesis_iterator.first->second);
				}

				// Adds the hypothesis to the vector.
				hypotheses_vector.push_back(&hypthesis_iterator.first->second);
			}

			// Creates the CV Parameter vector.
			std::vector<CV_Parameter> cv_parameter_vector;
			cv_parameter_vector.reserve(group.second.cv_parameters.size());
			for (CV_Parameter& parameter : group.second.cv_parameters)
			{
				cv_parameter_vector.push_back({ *m_cv_param_names_.find(parameter.first), parameter.second });
			}

			// Inserts the sequence into the table.
			auto group_iterator = m_protein_group_table.insert(
			{
				group.second.id,
				cv_parameter_vector,
				hypotheses_vector
			});

			// Creates the relations towards the hypotheses.
			for (auto hypothesis : hypotheses_vector)
			{
				hypothesis->protein_group_relation = &group_iterator.first->second;
			}

			// Maps the new protein group.
			group_map.insert({ hasher(group.second.id), &group_iterator.first->second });
		}
	}

	void LinkedTables::InsertScanTable(const size_t file_id, const std::vector<IPL>& ipl_entries, const MzXML_File& mzxml_file)
	{
		// Resizes the file its scan relation vector.

		File& file_reference = m_file_table.find(file_id)->second;
		file_reference.scan_relations.reserve(file_reference.scan_relations.size() + mzxml_file.scans.size());

		// Inserts the scans and maps them based on their id.
		std::unordered_map<size_t, Scan*> file_scans;
		for (const std::pair<size_t, TAPP::Filetypes::MzXML::Scan>& scan : mzxml_file.scans)
		{
			auto scan_iterator = m_scan_table.insert(
			{
				scan.second.scan_id,
				scan.second.peaks,
				scan.second.ms_level,
				scan.second.polarity,
				scan.second.min_mz,
				scan.second.max_mz,
				scan.second.base_peak_mz,
				scan.second.base_peak_intensity,
				scan.second.retention_time,
				scan.second.total_ion_current,
				&m_file_table.find(file_id)->second,
				nullptr,
				std::vector<Event*>()
			});

			// Creates the file relation.
			file_reference.scan_relations.push_back(&scan_iterator.first->second);

			// Adds the scan to the map.
			file_scans.insert({ scan_iterator.first->second.scan_id, &scan_iterator.first->second });
		}

		// Inserts the events and maps their locations to their spectral scan id.
		std::unordered_map<size_t, Event*> file_events;
		for (const std::pair<size_t, TAPP::Filetypes::MzXML::Event>& event : mzxml_file.events)
		{
			size_t spec = event.second.spectral_scan->scan_id;
			size_t pre = event.second.precursor_scan->scan_id;

			auto event_iterator = m_event_table.insert(
			{
				file_scans.find(event.second.spectral_scan->scan_id)->second,
				file_scans.find(event.second.precursor_scan->scan_id)->second,
				event.second.precursor_charge_state,
				event.second.precursor_intensity,
				event.second.precursor_mz
			});

			// Creates the relationship.
			file_scans.find(event.second.spectral_scan->scan_id)->second->event_relations.push_back(&event_iterator.first->second);
			file_scans.find(event.second.precursor_scan->scan_id)->second->event_relations.push_back(&event_iterator.first->second);

			// Maps the new event.
			file_events.insert({ event_iterator.first->second.spectral_scan->scan_id, &event_iterator.first->second });
		}

		// Creates a reference to the file peaks vector.
		std::vector<Peak*>& file_peaks(m_file_table.find(file_id)->second.peak_relations);

		// Maps the file peaks to their ids.
		std::unordered_map<size_t, Peak*> mapped_peaks;
		mapped_peaks.reserve(m_file_table.find(file_id)->second.peak_relations.size());
		for (Peak* peak : file_peaks)
		{
			mapped_peaks.insert({ peak->peak_id, peak });
		}

		// Creates a vector to track the clusters.
		std::vector<IsotopicCluster*> cluster_vector;
		cluster_vector.reserve(ipl_entries.size());

		// Inserts the initial cluster information.
		m_cluster_table.reserve(m_cluster_table.size() + ipl_entries.size());
		for (const IPL& entry : ipl_entries)
		{
			// Attempts to match the cluster to an event peak.
			Peak* event_peak = nullptr;
			std::unordered_map<size_t, Peak*>::iterator event_peak_iterator(mapped_peaks.find(entry.event_peak));
			if (event_peak_iterator != mapped_peaks.end())
			{
				event_peak = event_peak_iterator->second;
			}

			// Links all peaks to the cluster.
			std::vector<Peak*> peaks;
			peaks.reserve(entry.cluster_peaks.size());
			for (size_t peak : entry.cluster_peaks)
			{
				// TODO: In order to create a proper connection to all peaks, the .orph file needs to be parsed as well.
				auto peak_iterator = mapped_peaks.find(peak);
				if (peak_iterator != mapped_peaks.end())
				{
					peaks.push_back(peak_iterator->second);
				}
			}

			// Acquires an event pointer, if possible.
			Event* event_relation = nullptr;
			auto event_iterator = file_events.find(entry.event_id);
			if (event_iterator != file_events.end())
			{
				event_relation = event_iterator->second;
			}

			// Adds the cluster to the table. The link to the identification must be created at a later stage.
			auto cluster_iterator = m_cluster_table.insert(
			{
				entry.cluster_id,
				entry.cluster_group,
				entry.identification_method,
				entry.chargestate,
				event_peak,
				event_relation,
				nullptr,
				peaks,
				&file_reference
			});

			// Creates the relationships.
			if (event_iterator != file_events.end())
			{
				event_iterator->second->cluster_relation = &cluster_iterator.first->second;
			}

			for (size_t peak : entry.cluster_peaks)
			{
				// TODO: In order to create a proper connection to all peaks, the .orph file needs to be parsed as well.

				auto peak_iterator = mapped_peaks.find(peak);
				if (peak_iterator != mapped_peaks.end())
				{
					peak_iterator->second->cluster_relations.push_back(&cluster_iterator.first->second);
				}
			}

			// Adds the cluster to the vector.
			cluster_vector.push_back(&cluster_iterator.first->second);
		}

		// Shrinks the vector and then adds it to the file record.
		cluster_vector.shrink_to_fit();
		file_reference.isotopic_cluster_relations = cluster_vector;
	}

	/*** Private Functions ******************************************************/

	void LinkedTables::InsertPID_(const std::vector<PID>& pid_entries)
	{
		std::unordered_map<size_t, size_t> file_peaks;
		std::unordered_map<size_t, size_t> metapeak_peaks;

		// Loops through the list and creates the File and Metapeak entries.
		for (const PID& entry : pid_entries)
		{
			if (m_file_table.find(entry.file_id) == m_file_table.end())
			{
				m_file_table.insert
				(
					entry.file_id,
					{
						entry.file_id,
						entry.class_id,
						std::vector<IdentificationResult*>(),
						std::vector<IsotopicCluster*>(),
						std::vector<Peak*>(),
						std::vector<Scan*>()
					}
				);
			}

			if (m_metapeak_table.find(entry.metapeak_id) == m_metapeak_table.end())
			{
				m_metapeak_table.insert
				(
					entry.metapeak_id,
					{
						(size_t)entry.metapeak_id,
						std::vector<Peak*>()
					}
				);
			}

			auto file_peaks_iterator = file_peaks.insert({ entry.file_id, 0 });
			auto metapeak_peaks_iterator = metapeak_peaks.insert({ entry.metapeak_id, 0 });

			++file_peaks_iterator.first->second;
			++metapeak_peaks_iterator.first->second;
		}

		// Reserves space for the peaks inside the File and Metapeak vectors.
		for (std::pair<const size_t, size_t>& peak : file_peaks)
		{
			m_file_table.find(peak.first)->second.peak_relations.reserve(peak.second);
		}

		for (std::pair<const size_t, size_t>& peak : metapeak_peaks)
		{
			m_metapeak_table.find(peak.first)->second.peaks.reserve(peak.second);
		}

		// Adds the actual Peaks.
		for (const PID& entry : pid_entries)
		{
			auto peak_iterator = m_peak_table.insert
			({
				static_cast<size_t>(entry.peak_id),
				entry.mz,
				entry.rt,
				entry.intensity,
				&m_file_table.find(entry.file_id)->second,
				&m_metapeak_table.find(entry.metapeak_id)->second,
				std::vector<IsotopicCluster*>()
			});

			// Adds the peak relation to the File and Metapeak.
			m_file_table.find(entry.file_id)->second.peak_relations.push_back(&peak_iterator.first->second);
			m_metapeak_table.find(entry.metapeak_id)->second.peaks.push_back(&peak_iterator.first->second);
		}
	}
}
