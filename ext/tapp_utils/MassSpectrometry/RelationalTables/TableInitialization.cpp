// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#include "MassSpectrometry/RelationalTables/TableInitialization.h"

#include <algorithm>

namespace TAPP::MassSpectrometry::RelationalTables
{
	/*** MZXML ******************************************************************/

	TAPP::Collections::Table<Event> InitializeEventTable(const std::string table_name)
	{
		return TAPP::Collections::Table<Event>(table_name, { "event_spectral_scan", "event_precursor_scan", "event_precursor_chargestate", "event_precursor_intensity", "event_precursor_mz" },
			[](std::vector<Event*>& events, const std::string& column_name)
		{
			std::vector<std::string> records;
			records.reserve(events.size());

			if (column_name == "event_spectral_scan")
			{
				for (Event* event : events)
				{
					records.push_back(std::to_string(event->spectral_scan->scan_id));
				}
			}
			else if (column_name == "event_precursor_scan")
			{
				for (Event* event : events)
				{
					records.push_back(std::to_string(event->precursor_scan->scan_id));
				}
			}
			else if (column_name == "event_precursor_chargestate")
			{
				for (Event* event : events)
				{
					records.push_back(std::to_string(event->precursor_charge_state));
				}
			}
			else if (column_name == "event_precursor_intensity")
			{
				for (Event* event : events)
				{
					records.push_back(std::to_string(event->precursor_intensity));
				}
			}
			else if (column_name == "event_precursor_mz")
			{
				for (Event* event : events)
				{
					records.push_back(std::to_string(event->precursor_mz));
				}
			}

			return records;
		});
	}

	TAPP::Collections::Table<Scan> InitializeScanTable(const std::string table_name)
	{
		return TAPP::Collections::Table<Scan>(table_name, { "scan_id", "scan_amount_of_peaks", "scan_ms_level", "scan_polarity", "scan_min_mz", "scan_max_mz", "scan_event_peak_mz", "scan_event_peak_intensity", "scan_retention_time", "scan_total_ion_current" },
			[](std::vector<Scan*>& scans, const std::string& column_name)
		{
			std::vector<std::string> records;
			records.reserve(scans.size());

			if (column_name == "scan_id")
			{
				for (Scan* scan : scans)
				{
					records.push_back(std::to_string(scan->scan_id));
				}
			}
			else if (column_name == "scan_amount_of_peaks")
			{
				for (Scan* scan : scans)
				{
					records.push_back(std::to_string(scan->amount_of_peaks));
				}
			}
			else if (column_name == "scan_ms_level")
			{
				for (Scan* scan : scans)
				{
					records.push_back(std::to_string(scan->ms_level));
				}
			}
			else if (column_name == "scan_polarity")
			{
				for (Scan* scan : scans)
				{
					records.push_back(std::to_string(scan->polarity));
				}
			}
			else if (column_name == "scan_min_mz")
			{
				for (Scan* scan : scans)
				{
					records.push_back(std::to_string(scan->min_mz));
				}
			}
			else if (column_name == "scan_max_mz")
			{
				for (Scan* scan : scans)
				{
					records.push_back(std::to_string(scan->max_mz));
				}
			}
			else if (column_name == "scan_event_peak_mz")
			{
				for (Scan* scan : scans)
				{
					records.push_back(std::to_string(scan->event_peak_mz));
				}
			}
			else if (column_name == "scan_event_peak_intensity")
			{
				for (Scan* scan : scans)
				{
					records.push_back(std::to_string(scan->event_peak_intensity));
				}
			}
			else if (column_name == "scan_retention_time")
			{
				for (Scan* scan : scans)
				{
					records.push_back(std::to_string(scan->retention_time));
				}
			}
			else if (column_name == "scan_total_ion_current")
			{
				for (Scan* scan : scans)
				{
					records.push_back(std::to_string(scan->total_ion_current));
				}
			}

			return records;
		});
	}

	/*** MZID *******************************************************************/

	TAPP::Collections::Table<FastaSequence> InitializeFastaSequenceTable(const std::string table_name)
	{
		return TAPP::Collections::Table<FastaSequence>(table_name, { "fasta_sequence_id", "fasta_sequence_description" },
			[](std::vector<FastaSequence*>& sequences, const std::string& column_name)
		{
			std::vector<std::string> records;
			records.reserve(sequences.size());

			if (column_name == "fasta_sequence_id")
			{
				for (FastaSequence* sequence : sequences)
				{
					records.push_back(sequence->id);
				}
			}
			else if (column_name == "fasta_sequence_description")
			{
				for (FastaSequence* sequence : sequences)
				{
					records.push_back(sequence->description);
				}
			}

			return records;
		});
		
	}

	TAPP::Collections::Table<FragmentIon> InitializeFragmentIonTable(const std::string table_name, const char delimiter)
	{
		return TAPP::Collections::Table<FragmentIon>(table_name, { "fragment_ion_name", "fragment_ion_indices", "fragment_ion_charge", "fragment_ion_mz_values", "fragment_ion_intensity_values", "fragment_ion_error_values" },
			[delimiter](std::vector<FragmentIon*>& fragmentations, const std::string& column_name)
		{
			std::vector<std::string> records;
			records.reserve(fragmentations.size());

			std::string delimiter_as_string(1, delimiter);

			if (column_name == "fragment_ion_name")
			{
				for (FragmentIon* fragment : fragmentations)
				{
					records.push_back(fragment->name);
				}
			}
			else if (column_name == "fragment_ion_indices")
			{
				for (FragmentIon* fragment : fragmentations)
				{
					records.push_back(fragment->indices);
				}
			}
			else if (column_name == "fragment_ion_charge")
			{
				for (FragmentIon* fragment : fragmentations)
				{
					records.push_back(std::to_string(fragment->charge));
				}
			}
			else if (column_name == "fragment_ion_mz_values")
			{
				for (FragmentIon* fragment : fragmentations)
				{
					std::string values("");
					for (double value : fragment->mz_values)
					{
						values += std::to_string(value) + delimiter_as_string;
					}
					values.erase(values.size() - 1);

					records.push_back(values);
				}
			}
			else if (column_name == "fragment_ion_intensity_values")
			{
				for (FragmentIon* fragment : fragmentations)
				{
					std::string values("");
					for (double value : fragment->intensity_values)
					{
						values += std::to_string(value) + delimiter_as_string;
					}
					values.erase(values.size() - 1);
					
					records.push_back(values);
				}
			}
			else if (column_name == "fragment_ion_error_values")
			{
				for (FragmentIon* fragment : fragmentations)
				{
					std::string values("");
					for (double value : fragment->error_values)
					{
						values += std::to_string(value) + delimiter_as_string;
					}
					values.erase(values.size() - 1);

					records.push_back(values);
				}
			}

			return records;
		});
	}

	TAPP::Collections::Table<IdentificationItem> InitializeIdentificationItemTable(const std::string table_name, const TAPP::Filetypes::Mzid::SpectrumIdentificationItem* item, const char delimiter)
	{
		std::vector<std::string> column_names(
		{
			"identification_item_id",
			"identification_item_chargestate",
			"identification_item_calculated_mz",
			"identification_item_experimental_mz",
			"identification_item_passed",
			"identification_item_rank",
			"identification_item_peptide_reference",
			"identification_item_fragmentations",
			"identification_item_peptide_evidence"
		});

		if (item)
		{
			for (const CV_Parameter& parameter : item->cv_parameters)
			{
				std::string lower_case(parameter.first);
				std::transform(lower_case.begin(), lower_case.end(), lower_case.begin(), ::tolower);

				column_names.push_back("identification_item_" + lower_case);
			}
		}

		return TAPP::Collections::Table<IdentificationItem>(table_name, column_names, 
			[delimiter](std::vector<IdentificationItem*>& items, const std::string& column_name)
		{
			std::vector<std::string> records;
			records.reserve(items.size());

			std::string delimiter_as_string(1, delimiter);

			if (column_name == "identification_item_id")
			{
				for (IdentificationItem* item : items)
				{
					records.push_back(item->id);
				}
			}
			else if (column_name == "identification_item_chargestate")
			{
				for (IdentificationItem* item : items)
				{
					records.push_back(std::to_string(item->chargestate));
				}
			}
			else if (column_name == "identification_item_calculated_mz")
			{
				for (IdentificationItem* item : items)
				{
					records.push_back(std::to_string(item->calculated_mz));
				}
			}
			else if (column_name == "identification_item_experimental_mz")
			{
				for (IdentificationItem* item : items)
				{
					records.push_back(std::to_string(item->experimental_mz));
				}
			}
			else if (column_name == "identification_item_passed")
			{
				for (IdentificationItem* item : items)
				{
					records.push_back(std::to_string(item->passed));
				}
			}
			else if (column_name == "identification_item_rank")
			{
				for (IdentificationItem* item : items)
				{
					records.push_back(std::to_string(item->rank));
				}
			}
			else if (column_name == "identification_item_peptide_reference")
			{
				for (IdentificationItem* item : items)
				{
					records.push_back(item->peptide_reference->id);
				}
			}
			else if (column_name == "identification_item_fragmentations")
			{
				for (IdentificationItem* item : items)
				{
					std::string fragmentations("");
					for (FragmentIon* fragment : item->fragmentations)
					{
						fragmentations += fragment->name + delimiter_as_string;
					}
					fragmentations.erase(fragmentations.size() - 1);

					records.push_back(fragmentations);
				}
			}
			else if (column_name == "identification_item_peptide_evidence")
			{
				for (IdentificationItem* item : items)
				{
					std::string evidence_string("");
					for (PeptideEvidence* evidence : item->peptide_evidence)
					{
						evidence_string += evidence->id + delimiter_as_string;
					}
					evidence_string.erase(evidence_string.size() - 1);

					records.push_back(evidence_string);
				}
			}
			else
			{
				for (size_t parameter = 0; parameter < items[0]->cv_parameters.size(); ++parameter)
				{
					if (column_name == std::string("identification_item_" + items[0]->cv_parameters[parameter].first))
					{
						for (IdentificationItem* item : items)
						{
							records.push_back(std::to_string(item->cv_parameters[parameter].second));
						}
						break;
					}
				}
			}

			return records;
		});
	}

	TAPP::Collections::Table<IdentificationResult> InitializeIdentificationResultTable(const std::string table_name, const char delimiter, const char invalid)
	{
		return TAPP::Collections::Table<IdentificationResult>(table_name, { "identification_result_id", "identification_result_index", "identification_result_retention_time", "identification_result_scan", "identification_result_items" },
			[delimiter, invalid](std::vector<IdentificationResult*>& results, const std::string& column_name)
		{
			std::vector<std::string> records;
			records.reserve(results.size());

			std::string delimiter_as_string(1, delimiter);
			std::string invalid_as_string(1, invalid);

			if (column_name == "identification_result_id")
			{
				for (IdentificationResult* result : results)
				{
					records.push_back(result->id);
				}
			}
			else if (column_name == "identification_result_index")
			{
				for (IdentificationResult* result : results)
				{
					records.push_back(std::to_string(result->index));
				}
			}
			else if (column_name == "identification_result_retention_time")
			{
				for (IdentificationResult* result : results)
				{
					records.push_back(std::to_string(result->retention_time));
				}
			}
			else if (column_name == "identification_result_scan")
			{
				for (IdentificationResult* result : results)
				{
					if (result->scan)
					{
						records.push_back(std::to_string(result->scan->scan_id));
					}
					else
					{
						records.push_back(invalid_as_string);
					}
				}
			}
			else if (column_name == "identification_result_items")
			{
				for (IdentificationResult* result : results)
				{
					std::string items("");
					for (IdentificationItem* item : result->identification_items)
					{
						items += item->id + delimiter_as_string;
					}
					items.erase(items.size() - 1);

					records.push_back(items);
				}
			}

			return records;
		});
	}

	TAPP::Collections::Table<Peptide> InitializePeptideTable(const std::string table_name, const char delimiter)
	{
		return TAPP::Collections::Table<Peptide>(table_name, { "peptide_id", "peptide_sequence", "peptide_modifications" },
			[delimiter](std::vector<Peptide*>& peptides, const std::string& column_name)
		{
			std::vector<std::string> records;
			records.reserve(peptides.size());

			std::string delimiter_as_string(1, delimiter);

			if (column_name == "peptide_id")
			{
				for (Peptide* peptide : peptides)
				{
					records.push_back(peptide->id);
				}
			}
			else if (column_name == "peptide_sequence")
			{
				for (Peptide* peptide : peptides)
				{
					records.push_back(peptide->sequence);
				}
			}
			else if (column_name == "peptide_modifications")
			{
				for (Peptide* peptide : peptides)
				{
					std::string modifications("");
					for (PeptideModification* modification : peptide->modifications)
					{
						modifications += *modification->name + delimiter_as_string;
					}
					modifications.erase(modifications.size() - 1);

					records.push_back(modifications);
				}
			}

			return records;
		});
	}

	TAPP::Collections::Table<PeptideEvidence> InitializePeptideEvidenceTable(const std::string table_name)
	{
		return TAPP::Collections::Table<PeptideEvidence>(table_name, { "peptide_evidence_id", "peptide_evidence_start", "peptide_evidence_end", "peptide_evidence_pre", "peptide_evidence_post", "peptide_evidence_is_decoy", "peptide_evidence_peptide_reference", "peptide_evidence_sequence_reference" },
			[](std::vector<PeptideEvidence*>& peptides, const std::string& column_name)
		{
			std::vector<std::string> records;
			records.reserve(peptides.size());

			if (column_name == "peptide_evidence_id")
			{
				for (PeptideEvidence* peptide : peptides)
				{
					records.push_back(peptide->id);
				}
			}
			else if (column_name == "peptide_evidence_start")
			{
				for (PeptideEvidence* peptide : peptides)
				{
					records.push_back(std::to_string(peptide->start));
				}
			}
			else if (column_name == "peptide_evidence_end")
			{
				for (PeptideEvidence* peptide : peptides)
				{
					records.push_back(std::to_string(peptide->end));
				}
			}
			else if (column_name == "peptide_evidence_pre")
			{
				for (PeptideEvidence* peptide : peptides)
				{
					records.push_back(std::string(1, peptide->pre));
				}
			}
			else if (column_name == "peptide_evidence_post")
			{
				for (PeptideEvidence* peptide : peptides)
				{
					records.push_back(std::string(1, peptide->post));
				}
			}
			else if (column_name == "peptide_evidence_is_decoy")
			{
				for (PeptideEvidence* peptide : peptides)
				{
					records.push_back(std::to_string(peptide->is_decoy));
				}
			}
			else if (column_name == "peptide_evidence_peptide_reference")
			{
				for (PeptideEvidence* peptide : peptides)
				{
					records.push_back(peptide->petide_reference->id);
				}
			}
			else if (column_name == "peptide_evidence_sequence_reference")
			{
				for (PeptideEvidence* peptide : peptides)
				{
					records.push_back(peptide->sequence_reference->id);
				}
			}

			return records;
		});
	}

	TAPP::Collections::Table<PeptideModification> InitializePeptideModificationTable(const std::string table_name)
	{
		return TAPP::Collections::Table<PeptideModification>(table_name, { "peptide_modification_accession", "peptide_modification_location", "peptide_modification_monoisotopic_mass_delta", "peptide_modification_residues", "peptide_modification_cv_reference", "peptide_modification_name" },
			[](std::vector<PeptideModification*>& modifications, const std::string& column_name)
		{
			std::vector<std::string> records;
			records.reserve(modifications.size());

			if (column_name == "peptide_modification_accession")
			{
				for (PeptideModification* modification : modifications)
				{
					records.push_back(std::to_string(modification->accession));
				}
			}
			else if (column_name == "peptide_modification_location")
			{
				for (PeptideModification* modification : modifications)
				{
					records.push_back(std::to_string(modification->location));
				}
			}
			else if (column_name == "peptide_modification_monoisotopic_mass_delta")
			{
				for (PeptideModification* modification : modifications)
				{
					records.push_back(std::to_string(modification->monoisotopic_mass_delta));
				}
			}
			else if (column_name == "peptide_modification_residues")
			{
				for (PeptideModification* modification : modifications)
				{
					records.push_back(std::string(1, modification->residues));
				}
			}
			else if (column_name == "peptide_modification_cv_reference")
			{
				for (PeptideModification* modification : modifications)
				{
					records.push_back(*modification->cv_reference);
				}
			}
			else if (column_name == "peptide_modification_name")
			{
				for (PeptideModification* modification : modifications)
				{
					records.push_back(*modification->name);
				}
			}

			return records;
		});
	}

	TAPP::Collections::Table<ProteinGroup> InitializeProteinGroupTable(const std::string table_name, const TAPP::Filetypes::Mzid::ProteinGroup* protein_group, const char delimiter)
	{
		std::vector<std::string> column_names(
		{
			"protein_group_id",
			"protein_group_hypotheses"
		});

		if (protein_group)
		{
			for (const CV_Parameter& parameter : protein_group->cv_parameters)
			{
				std::string lower_case(parameter.first);
				std::transform(lower_case.begin(), lower_case.end(), lower_case.begin(), ::tolower);

				column_names.push_back("protein_group_" + lower_case);
			}
		}

		return TAPP::Collections::Table<ProteinGroup>(table_name, column_names,
			[delimiter](std::vector<ProteinGroup*>& proteins, const std::string& column_name)
		{
			std::vector<std::string> records;
			records.reserve(proteins.size());

			std::string delimiter_as_string(1, delimiter);

			if (column_name == "protein_group_id")
			{
				for (ProteinGroup* protein : proteins)
				{
					records.push_back(protein->id);
				}
			}
			else if (column_name == "protein_group_hypotheses")
			{
				for (ProteinGroup* protein : proteins)
				{
					std::string hypotheses("");
					for (ProteinHypothesis* hypothesis : protein->hypotheses)
					{
						hypotheses += hypothesis->id + delimiter_as_string;
					}
					hypotheses.erase(hypotheses.size() - 1);

					records.push_back(hypotheses);
				}
			}
			else
			{
				for (size_t parameter = 0; parameter < proteins[0]->cv_parameters.size(); ++parameter)
				{
					if (column_name == std::string("identification_item_" + proteins[0]->cv_parameters[parameter].first))
					{
						for (ProteinGroup* protein : proteins)
						{
							records.push_back(std::to_string(protein->cv_parameters[parameter].second));
						}
						break;
					}
				}
			}

			return records;
		});
	}

	TAPP::Collections::Table<ProteinHypothesis> InitializeProteinHypothesisTable(const std::string table_name, const char delimiter)
	{
		return TAPP::Collections::Table<ProteinHypothesis>(table_name, { "protein_hypothesis_id", "protein_hypothesis_passed_threshold", "protein_hypothesis_evidence", "protein_hypothesis_sequence", "protein_hypothesis_identification_items" },
			[delimiter](std::vector<ProteinHypothesis*>& proteins, const std::string& column_name)
		{
			std::vector<std::string> records;
			records.reserve(proteins.size());

			std::string delimiter_as_string(1, delimiter);

			if (column_name == "protein_hypothesis_id")
			{
				for (ProteinHypothesis* protein : proteins)
				{
					records.push_back(protein->id);
				}
			}
			else if (column_name == "protein_hypothesis_passed_threshold")
			{
				for (ProteinHypothesis* protein : proteins)
				{
					records.push_back(std::to_string(protein->passed_threshold));
				}
			}
			else if (column_name == "protein_hypothesis_evidence")
			{
				for (ProteinHypothesis* protein : proteins)
				{
					records.push_back(protein->evidence->id);
				}
			}
			else if (column_name == "protein_hypothesis_sequence")
			{
				for (ProteinHypothesis* protein : proteins)
				{
					records.push_back(protein->sequence->id);
				}
			}
			else if (column_name == "protein_hypothesis_identification_items")
			{
				for (ProteinHypothesis* protein : proteins)
				{
					std::string items("");
					for (IdentificationItem* item : protein->identification_items)
					{
						items += item->id + delimiter_as_string;
					}
					items.erase(items.size() - 1);

					records.push_back(items);
				}
			}

			return records;
		});
	}

	/*** TAPP *******************************************************************/

	TAPP::Collections::Table<File> InitializeFileTable(const std::string table_name)
	{
		return TAPP::Collections::Table<File>(table_name, { "file_id", "file_class" },
			[](std::vector<File*>& files, const std::string& column_name)
		{
			std::vector<std::string> records;
			records.reserve(files.size());

			if (column_name == "file_id")
			{
				for (File* file : files)
				{
					records.push_back(std::to_string(file->file_id));
				}
			}
			else if (column_name == "file_class")
			{
				for (File* file : files)
				{
					records.push_back(std::to_string(file->identification_class));
				}
			}

			return records;
		});
	}

	TAPP::Collections::Table<IsotopicCluster> InitializeIsotopicClusterTable(const std::string table_name, const char delimiter, const char invalid)
	{
		return TAPP::Collections::Table<IsotopicCluster>(table_name, { "cluster_id", "cluster_group", "cluster_identification_method", "cluster_chargestate", "cluster_event_peak", "cluster_peaks", "cluster_event", "cluster_identification" },
			[delimiter, invalid](std::vector<IsotopicCluster*>& clusters, const std::string& column_name)
			{
				std::vector<std::string> records;
				records.reserve(clusters.size());

				std::string delimiter_as_string(1, delimiter);
				std::string invalid_as_string(1, invalid);

				if (column_name == "cluster_id")
				{
					for (IsotopicCluster* cluster : clusters)
					{
						records.push_back(std::to_string(cluster->id));
					}
				}
				else if (column_name == "cluster_group")
				{
					for (IsotopicCluster* cluster : clusters)
					{
						records.push_back(std::to_string(cluster->group));
					}
				}
				else if (column_name == "cluster_identification_method")
				{
					for (IsotopicCluster* cluster : clusters)
					{
						records.push_back(std::string(1, cluster->identification_method));
					}
				}
				else if (column_name == "cluster_chargestate")
				{
					for (IsotopicCluster* cluster : clusters)
					{
						records.push_back(std::to_string(cluster->chargestate));
					}
				}
				else if (column_name == "cluster_event_peak")
				{
					for (IsotopicCluster* cluster : clusters)
					{
						if (cluster->event_peak)
						{
							records.push_back(std::to_string(cluster->event_peak->peak_id));
						}
						else
						{
							records.push_back(invalid_as_string);
						}
					}
				}
				else if (column_name == "cluster_event")
				{
					for (IsotopicCluster* cluster : clusters)
					{
						if (cluster->event)
						{
							records.push_back(std::to_string(cluster->event->spectral_scan->scan_id));
						}
						else
						{
							records.push_back(invalid_as_string);
						}
					}
				}
				else if (column_name == "cluster_identification")
				{
					for (IsotopicCluster* cluster : clusters)
					{
						if (cluster->identification)
						{
							records.push_back(cluster->identification->id);
						}
						else
						{
							records.push_back(invalid_as_string);
						}
					}
				}
				else if (column_name == "cluster_peaks")
				{
					for (IsotopicCluster* cluster : clusters)
					{
						std::string peaks("");
						for (Peak* peak : cluster->clustered_peaks)
						{
							peaks += std::to_string(peak->peak_id) + delimiter_as_string;
						}
						
						peaks.erase(peaks.size() - 1);

						records.push_back(peaks);
					}
				}

				return records;
		});
	}

	TAPP::Collections::Table<Metapeak> InitializeMetapeakTable(const std::string table_name, const char delimiter)
	{
		return TAPP::Collections::Table<Metapeak>(table_name, { "metapeak_id", "metapeak_peaks" },
			[delimiter](std::vector<Metapeak*>& metapeaks, const std::string& column_name)
		{
			std::vector<std::string> records;
			records.reserve(metapeaks.size());

			std::string delimiter_as_string(1, delimiter);

			if (column_name == "metapeak_id")
			{
				for (Metapeak* metapeak : metapeaks)
				{
					records.push_back(std::to_string(metapeak->metapeak_id));
				}
			}
			else if (column_name == "metapeak_peaks")
			{
				for (Metapeak* metapeak : metapeaks)
				{
					std::string peaks("");
					for (Peak* peak : metapeak->peaks)
					{
						peaks += std::to_string(peak->peak_id) + delimiter_as_string;
					}

					peaks.erase(peaks.size() - 1);

					records.push_back(peaks);
				}
			}

			return records;
		});
	}

	TAPP::Collections::Table<Peak> InitializePeakTable(const std::string table_name)
	{
		return TAPP::Collections::Table<Peak>(table_name, { "peak_id", "peak_mz", "peak_rt", "peak_intensity" },
			[](std::vector<Peak*>& peaks, const std::string& column_name)
		{
			std::vector<std::string> records;
			records.reserve(peaks.size());

			if (column_name == "peak_id")
			{
				for (Peak* peak : peaks)
				{
					records.push_back(std::to_string(peak->peak_id));
				}
			}
			else if (column_name == "peak_mz")
			{
				for (Peak* peak : peaks)
				{
					records.push_back(std::to_string(peak->mz));
				}
			}
			else if (column_name == "peak_rt")
			{
				for (Peak* peak : peaks)
				{
					records.push_back(std::to_string(peak->rt));
				}
			}
			else if (column_name == "peak_intensity")
			{
				for (Peak* peak : peaks)
				{
					records.push_back(std::to_string(peak->intensity));
				}
			}

			return records;
		});
	}
}
