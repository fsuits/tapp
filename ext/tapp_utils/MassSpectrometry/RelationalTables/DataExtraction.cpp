#include "MassSpectrometry/RelationalTables/DataExtraction.h"

#include <functional>

using namespace std::placeholders;

namespace TAPP::MassSpectrometry::RelationalTables
{
	// The default constructor for the DataExtraction class.
	DataExtraction::DataExtraction(LinkedTables& linked_tables, OutputCharacters& output_characters, size_t maximum_buffer_size)
		: m_linked_tables_(linked_tables), m_output_characters_(output_characters), m_writer_(maximum_buffer_size)
	{
	}

	// Creates the blueprint for the property table based on the reference table.
	std::vector<PropertyOutputRecord> DataExtraction::CreatePropertyTableBlueprint(std::vector<ReferenceTableRecord>& reference_table, std::vector<IsotopicCluster*>& approved_clusters, ClusterSelectionMethod cluster_compare)
	{
		// Maps the approved clusters. Each vector holds a file specific map.
		std::vector<std::unordered_map<size_t, IsotopicCluster*>> mapped_clusters(m_linked_tables_.m_file_table.size());
		for (IsotopicCluster* cluster : approved_clusters)
		{
			for (Peak* peak : cluster->clustered_peaks)
			{
				mapped_clusters[cluster->file_relation->file_id].insert({ peak->peak_id, cluster });
			}
		}

		// Converts the reference table into a more suitable format for outputting.
		std::vector<PropertyOutputRecord> property_output_table;
		property_output_table.reserve(reference_table.size());
		for (ReferenceTableRecord reference_record : reference_table)
		{
			// Loops through all the peaks, acquiring their pointers from the approved cluster list where possible.
			std::vector<std::vector<RelationalTables::IsotopicCluster*>> file_separated_clusters(m_linked_tables_.m_file_table.size());
			for (auto peak_list : reference_record.file_separated_peaks)
			{
				for (RelationalTables::Peak* peak : peak_list)
				{
					// Acquires a pointer to a cluster.
					auto attached_cluster = mapped_clusters[peak->file_relation->file_id].find(peak->peak_id);
					IsotopicCluster* cluster_pointer = nullptr;

					if (attached_cluster != mapped_clusters[peak->file_relation->file_id].end())
					{
						cluster_pointer = attached_cluster->second;
					}

					// Pushes the pointer into the correct vector.
					if (cluster_compare && !file_separated_clusters[peak->file_relation->file_id].empty())
					{
						if (file_separated_clusters[peak->file_relation->file_id][0])
						{
							file_separated_clusters[peak->file_relation->file_id][0] = cluster_compare(file_separated_clusters[peak->file_relation->file_id][0], cluster_pointer);
						}
						else
						{
							file_separated_clusters[peak->file_relation->file_id][0] = cluster_pointer;
						}
					}
					else
					{
						file_separated_clusters[peak->file_relation->file_id].push_back(cluster_pointer);
					}
				}
			}

			property_output_table.push_back({ reference_record.metapeak->metapeak_id, file_separated_clusters });
		}

		return property_output_table;
	}

	// Creates the reference table based on the linked_tables information combined with the approved clusters.
	std::vector<ReferenceTableRecord> DataExtraction::CreateReferenceTable(std::vector<IsotopicCluster*>& approved_clusters, PeakSelectionMethod peak_compare)
	{
		// Creates a vector to hold all the records for the Reference Table.
		std::vector<ReferenceTableRecord> reference_table;
		reference_table.reserve(m_linked_tables_.m_metapeak_table.size());
		for (auto& metapeak : m_linked_tables_.m_metapeak_table)
		{
			std::vector<std::vector<Peak*>> file_separated_peaks(m_linked_tables_.m_file_table.size());
			for (Peak* member_peak : metapeak.second.peaks)
			{
				if (peak_compare && !file_separated_peaks[member_peak->file_relation->file_id].empty())
				{
					file_separated_peaks[member_peak->file_relation->file_id][0] = peak_compare(file_separated_peaks[member_peak->file_relation->file_id][0], member_peak);
				}
				else
				{
					// Inserts the peak into the new record at the correct file.
					file_separated_peaks[member_peak->file_relation->file_id].push_back(member_peak);
				}
			}

			// Adds the reference record.
			reference_table.push_back({ &metapeak.second, file_separated_peaks });
		}

		std::sort(reference_table.begin(), reference_table.end(), [](ReferenceTableRecord& a, ReferenceTableRecord& b)
		{
			return a.metapeak->metapeak_id < b.metapeak->metapeak_id;
		});

		return reference_table;
	}

	// Creates a map that can be used to identify which property belongs to which table.
	std::unordered_map<std::string, std::string> DataExtraction::CreatePropertyToTableMap(void)
	{
		std::unordered_map<std::string, std::string> property_to_table_map;

		// Inserts the properties for the isotopic clusters.
		for (std::string& property : m_linked_tables_.m_cluster_table.GetColumnNames())
		{
			property_to_table_map.insert({ property, m_linked_tables_.m_cluster_table.GetTableName() });
		}

		// Inserts the properties for the events.
		for (std::string& property : m_linked_tables_.m_event_table.GetColumnNames())
		{
			property_to_table_map.insert({ property, m_linked_tables_.m_event_table.GetTableName() });
		}

		// Inserts the properties for the fasta sequences.
		for (std::string& property : m_linked_tables_.m_fasta_sequence_table.GetColumnNames())
		{
			property_to_table_map.insert({ property, m_linked_tables_.m_fasta_sequence_table.GetTableName() });
		}

		// Inserts the properties for the files.
		for (std::string& property : m_linked_tables_.m_file_table.GetColumnNames())
		{
			property_to_table_map.insert({ property, m_linked_tables_.m_file_table.GetTableName() });
		}

		// Inserts the properties for the fragment ions.
		for (std::string& property : m_linked_tables_.m_fragment_ion_table.GetColumnNames())
		{
			property_to_table_map.insert({ property, m_linked_tables_.m_fragment_ion_table.GetTableName() });
		}

		// Inserts the properties for the identification items.
		for (std::string& property : m_linked_tables_.m_identification_item_table.GetColumnNames())
		{
			property_to_table_map.insert({ property, m_linked_tables_.m_identification_item_table.GetTableName() });
		}

		// Inserts the properties for the identification results.
		for (std::string& property : m_linked_tables_.m_identification_result_table.GetColumnNames())
		{
			property_to_table_map.insert({ property, m_linked_tables_.m_identification_result_table.GetTableName() });
		}

		// Inserts the properties for the metapeaks.
		for (std::string& property : m_linked_tables_.m_metapeak_table.GetColumnNames())
		{
			property_to_table_map.insert({ property, m_linked_tables_.m_metapeak_table.GetTableName() });
		}

		// Inserts the properties for the peptide evidence.
		for (std::string& property : m_linked_tables_.m_peptide_evidence_table.GetColumnNames())
		{
			property_to_table_map.insert({ property, m_linked_tables_.m_peptide_evidence_table.GetTableName() });
		}

		// Inserts the properties for the peptide modifications.
		for (std::string& property : m_linked_tables_.m_peptide_modification_table.GetColumnNames())
		{
			property_to_table_map.insert({ property, m_linked_tables_.m_peptide_modification_table.GetTableName() });
		}

		// Inserts the properties for the peptides.
		for (std::string& property : m_linked_tables_.m_peptide_table.GetColumnNames())
		{
			property_to_table_map.insert({ property, m_linked_tables_.m_peptide_table.GetTableName() });
		}

		// Inserts the properties for the protein groups.
		for (std::string& property : m_linked_tables_.m_protein_group_table.GetColumnNames())
		{
			property_to_table_map.insert({ property, m_linked_tables_.m_protein_group_table.GetTableName() });
		}

		// Inserts the properties for the protein hypotheses.
		for (std::string& property : m_linked_tables_.m_protein_hypothesis_table.GetColumnNames())
		{
			property_to_table_map.insert({ property, m_linked_tables_.m_protein_hypothesis_table.GetTableName() });
		}

		// Inserts the properties for the scans.
		for (std::string& property : m_linked_tables_.m_scan_table.GetColumnNames())
		{
			property_to_table_map.insert({ property, m_linked_tables_.m_scan_table.GetTableName() });
		}

		return property_to_table_map;
	}

	// Creates a functor switch that can be used to convert IsotopicCluster pointers to property output strings.
	// This switch is intended for internal usage only. However, it can be used externally as long as the lifetimes of all the objects are guaranteed.
	Utilities::FunctorSwitch<std::string> DataExtraction::CreateToStringSwitch(std::string& column_name_string, std::vector<IsotopicCluster*>& input_vector, std::vector<std::string>& output_vector)
	{
		Utilities::FunctorSwitch<std::string> temporary_switch_object;

		// Isotopic Cluster output function.
		temporary_switch_object.AddFunctor(m_linked_tables_.m_cluster_table.GetTableName(), [this, &column_name_string, &input_vector, &output_vector]
		{
			output_vector = this->m_linked_tables_.m_cluster_table.RecordsToString(input_vector, column_name_string);
		});

		// Event output function.
		temporary_switch_object.AddFunctor(m_linked_tables_.m_event_table.GetTableName(), [this, &column_name_string, &input_vector, &output_vector]
		{
			// Ensures that the vector will only contain the new strings.
			output_vector.clear();
			output_vector.reserve(input_vector.size());

			// Creates the vector that will be used to extract the record data.
			std::vector<Event*> event_vector;
			event_vector.reserve(input_vector.size());
			for (IsotopicCluster* cluster : input_vector)
			{
				if (cluster->event)
				{
					event_vector.push_back(cluster->event);
				}
			}

			// Extracts the records.
			std::vector<std::string> records(this->m_linked_tables_.m_event_table.RecordsToString(event_vector, column_name_string));

			// Creates a string containing the required characters.
			std::string invalid_string(1, this->m_output_characters_.invalid_character);

			// Adds the records to the output vector.
			size_t record_index = 0;
			for (IsotopicCluster* cluster : input_vector)
			{
				if (cluster->event)
				{
					output_vector.push_back(records[record_index]);
					++record_index;
				}
				else
				{
					output_vector.push_back(invalid_string);
				}
			}
		});

		// Scan output function.
		temporary_switch_object.AddFunctor(m_linked_tables_.m_scan_table.GetTableName(), [this, &column_name_string, &input_vector, &output_vector]
		{
			// Ensures that the vector will only contain the new strings.
			output_vector.clear();
			output_vector.reserve(input_vector.size());

			// Creates the vector that will be used to extract the record data.
			std::vector<Scan*> scan_vector;
			scan_vector.reserve(input_vector.size());
			for (IsotopicCluster* cluster : input_vector)
			{
				if (cluster->event)
				{
					scan_vector.push_back(cluster->event->spectral_scan);
					scan_vector.push_back(cluster->event->precursor_scan);
				}
			}

			// Extracts the records.
			std::vector<std::string> records(this->m_linked_tables_.m_scan_table.RecordsToString(scan_vector, column_name_string));

			// Creates a string containing the required characters.
			std::string invalid_string(1, this->m_output_characters_.invalid_character);
			std::string separator_string(1, this->m_output_characters_.record_delimiter);

			// Adds the records to the output vector.
			size_t record_index = 0;
			for (IsotopicCluster* cluster : input_vector)
			{
				if (cluster->event)
				{
					output_vector.push_back(records[record_index] + separator_string + records[record_index + 1]);
					record_index += 2;
				}
				else
				{
					output_vector.push_back(invalid_string + separator_string + invalid_string);
				}
			}
		});

		// FastaSequence output function.
		temporary_switch_object.AddFunctor(m_linked_tables_.m_fasta_sequence_table.GetTableName(), [this, &column_name_string, &input_vector, &output_vector]
		{
			// Ensures that the vector will only contain the new strings.
			output_vector.clear();
			output_vector.reserve(input_vector.size());

			// Creates the vector that will be used to extract the record data.
			std::vector<FastaSequence*> sequence_vector;
			sequence_vector.reserve(input_vector.size());
			for (IsotopicCluster* cluster : input_vector)
			{
				if (cluster->identification)
				{
					for (IdentificationItem* item : cluster->identification->identification_items)
					{
						for (PeptideEvidence* evidence : item->peptide_evidence)
						{
							sequence_vector.push_back(evidence->sequence_reference);
						}
					}
				}
			}

			// Extracts the records.
			std::vector<std::string> records(this->m_linked_tables_.m_fasta_sequence_table.RecordsToString(sequence_vector, column_name_string));

			// Creates a string containing the required characters.
			std::string invalid_string(1, this->m_output_characters_.invalid_character);
			std::string separator_string(1, this->m_output_characters_.record_delimiter);

			// Adds the records to the output vector.
			size_t record_index = 0;
			for (IsotopicCluster* cluster : input_vector)
			{
				std::string composed_record("");

				if (cluster->identification)
				{
					for (IdentificationItem* item : cluster->identification->identification_items)
					{
						for (PeptideEvidence* evidence : item->peptide_evidence)
						{
							composed_record += records[record_index] + separator_string;
							++record_index;
						}
					}
				}

				if (!composed_record.empty())
				{
					composed_record.erase(composed_record.size() - 1);
					output_vector.push_back(composed_record);
				}
				else
				{
					output_vector.push_back(invalid_string);
				}
			}
		});

		// FragmentIon output function.
		temporary_switch_object.AddFunctor(m_linked_tables_.m_fragment_ion_table.GetTableName(), [this, &column_name_string, &input_vector, &output_vector]
		{
			// Ensures that the vector will only contain the new strings.
			output_vector.clear();
			output_vector.reserve(input_vector.size());

			// Creates the vector that will be used to extract the record data.
			std::vector<FragmentIon*> fragment_vector;
			fragment_vector.reserve(input_vector.size());
			for (IsotopicCluster* cluster : input_vector)
			{
				if (cluster->identification)
				{
					for (IdentificationItem* item : cluster->identification->identification_items)
					{
						for (FragmentIon* fragment : item->fragmentations)
						{
							fragment_vector.push_back(fragment);
						}
					}
				}
			}

			// Extracts the records.
			std::vector<std::string> records(this->m_linked_tables_.m_fragment_ion_table.RecordsToString(fragment_vector, column_name_string));

			// Creates a string containing the required characters.
			std::string invalid_string(1, this->m_output_characters_.invalid_character);
			std::string separator_string(1, this->m_output_characters_.record_delimiter);

			// Adds the records to the output vector.
			size_t record_index = 0;
			for (IsotopicCluster* cluster : input_vector)
			{
				std::string composed_record("");

				if (cluster->identification)
				{
					for (IdentificationItem* item : cluster->identification->identification_items)
					{
						for (FragmentIon* fragment : item->fragmentations)
						{
							composed_record += records[record_index] + separator_string;
							++record_index;
						}
					}
				}

				if (!composed_record.empty())
				{
					composed_record.erase(composed_record.size() - 1);
					output_vector.push_back(composed_record);
				}
				else
				{
					output_vector.push_back(invalid_string);
				}
			}
		});

		// IdentificationItem output function.
		temporary_switch_object.AddFunctor(m_linked_tables_.m_identification_item_table.GetTableName(), [this, &column_name_string, &input_vector, &output_vector]
		{
			// Ensures that the vector will only contain the new strings.
			output_vector.clear();
			output_vector.reserve(input_vector.size());

			// Creates the vector that will be used to extract the record data.
			std::vector<IdentificationItem*> item_vector;
			item_vector.reserve(input_vector.size());
			for (IsotopicCluster* cluster : input_vector)
			{
				if (cluster->identification)
				{
					for (IdentificationItem* item : cluster->identification->identification_items)
					{
						item_vector.push_back(item);
					}
				}
			}

			// Extracts the records.
			std::vector<std::string> records(this->m_linked_tables_.m_identification_item_table.RecordsToString(item_vector, column_name_string));

			// Creates a string containing the required characters.
			std::string invalid_string(1, this->m_output_characters_.invalid_character);
			std::string separator_string(1, this->m_output_characters_.record_delimiter);

			// Adds the records to the output vector.
			size_t record_index = 0;
			for (IsotopicCluster* cluster : input_vector)
			{
				std::string composed_record("");

				if (cluster->identification)
				{
					for (IdentificationItem* item : cluster->identification->identification_items)
					{
						composed_record += records[record_index] + separator_string;
						++record_index;
					}
				}

				if (!composed_record.empty())
				{
					composed_record.erase(composed_record.size() - 1);
					output_vector.push_back(composed_record);
				}
				else
				{
					output_vector.push_back(invalid_string);
				}
			}
		});

		// IdentificationResult output function.
		temporary_switch_object.AddFunctor(m_linked_tables_.m_identification_result_table.GetTableName(), [this, &column_name_string, &input_vector, &output_vector]
		{
			// Ensures that the vector will only contain the new strings.
			output_vector.clear();
			output_vector.reserve(input_vector.size());

			// Creates the vector that will be used to extract the record data.
			std::vector<IdentificationResult*> result_vector;
			result_vector.reserve(input_vector.size());

			for (IsotopicCluster* cluster : input_vector)
			{
				if (cluster->identification)
				{
					result_vector.push_back(cluster->identification);
				}
			}

			// Extracts the records.
			std::vector<std::string> records(this->m_linked_tables_.m_identification_result_table.RecordsToString(result_vector, column_name_string));

			// Creates a string containing the required characters.
			std::string invalid_string(1, this->m_output_characters_.invalid_character);
			std::string separator_string(1, this->m_output_characters_.record_delimiter);

			// Adds the records to the output vector.
			size_t record_index = 0;
			for (IsotopicCluster* cluster : input_vector)
			{
				if (cluster->identification)
				{
					output_vector.push_back(records[record_index]);
					++record_index;
				}
				else
				{
					output_vector.push_back(invalid_string);
				}
			}
		});

		// PeptideEvidence output function.
		temporary_switch_object.AddFunctor(m_linked_tables_.m_peptide_evidence_table.GetTableName(), [this, &column_name_string, &input_vector, &output_vector]
		{
			// Ensures that the vector will only contain the new strings.
			output_vector.clear();
			output_vector.reserve(input_vector.size());

			// Creates the vector that will be used to extract the record data.
			std::vector<PeptideEvidence*> evidence_vector;
			evidence_vector.reserve(input_vector.size());
			for (IsotopicCluster* cluster : input_vector)
			{
				if (cluster->identification)
				{
					for (IdentificationItem* item : cluster->identification->identification_items)
					{
						for (PeptideEvidence* evidence : item->peptide_evidence)
						{
							evidence_vector.push_back(evidence);
						}
					}
				}
			}

			// Extracts the records.
			std::vector<std::string> records(this->m_linked_tables_.m_peptide_evidence_table.RecordsToString(evidence_vector, column_name_string));

			// Creates a string containing the required characters.
			std::string invalid_string(1, this->m_output_characters_.invalid_character);
			std::string separator_string(1, this->m_output_characters_.record_delimiter);

			// Adds the records to the output vector.
			size_t record_index = 0;
			for (IsotopicCluster* cluster : input_vector)
			{
				std::string composed_record("");

				if (cluster->identification)
				{
					for (IdentificationItem* item : cluster->identification->identification_items)
					{
						for (PeptideEvidence* evidence : item->peptide_evidence)
						{
							composed_record += records[record_index] + separator_string;
							++record_index;
						}
					}
				}

				if (!composed_record.empty())
				{
					composed_record.erase(composed_record.size() - 1);
					output_vector.push_back(composed_record);
				}
				else
				{
					output_vector.push_back(invalid_string);
				}
			}
		});

		// PeptideModification output function.
		temporary_switch_object.AddFunctor(m_linked_tables_.m_peptide_modification_table.GetTableName(), [this, &column_name_string, &input_vector, &output_vector]
		{
			// Ensures that the vector will only contain the new strings.
			output_vector.clear();
			output_vector.reserve(input_vector.size());

			// Creates the vector that will be used to extract the record data.
			std::vector<PeptideModification*> modification_vector;
			modification_vector.reserve(input_vector.size());

			for (IsotopicCluster* cluster : input_vector)
			{
				if (cluster->identification)
				{
					for (IdentificationItem* item : cluster->identification->identification_items)
					{
						for (PeptideModification* modification : item->peptide_reference->modifications)
						{
							modification_vector.push_back(modification);
						}
					}
				}
			}

			// Extracts the records.
			std::vector<std::string> records(this->m_linked_tables_.m_peptide_modification_table.RecordsToString(modification_vector, column_name_string));

			// Creates a string containing the required characters.
			std::string invalid_string(1, this->m_output_characters_.invalid_character);
			std::string separator_string(1, this->m_output_characters_.record_delimiter);

			// Adds the records to the output vector.
			size_t record_index = 0;
			for (IsotopicCluster* cluster : input_vector)
			{
				std::string composed_record("");

				if (cluster->identification)
				{
					for (IdentificationItem* item : cluster->identification->identification_items)
					{
						for (PeptideModification* modification : item->peptide_reference->modifications)
						{
							composed_record += records[record_index] + separator_string;
							++record_index;
						}
					}
				}

				if (!composed_record.empty())
				{
					composed_record.erase(composed_record.size() - 1);
					output_vector.push_back(composed_record);
				}
				else
				{
					output_vector.push_back(invalid_string);
				}
			}
		});

		// Peptide output function.
		temporary_switch_object.AddFunctor(m_linked_tables_.m_peptide_table.GetTableName(), [this, &column_name_string, &input_vector, &output_vector]
		{
			// Ensures that the vector will only contain the new strings.
			output_vector.clear();
			output_vector.reserve(input_vector.size());

			// Creates the vector that will be used to extract the record data.
			std::vector<Peptide*> peptide_vector;
			peptide_vector.reserve(input_vector.size());

			for (IsotopicCluster* cluster : input_vector)
			{
				if (cluster->identification)
				{
					for (IdentificationItem* item : cluster->identification->identification_items)
					{
						peptide_vector.push_back(item->peptide_reference);
					}
				}
			}

			// Extracts the records.
			std::vector<std::string> records(this->m_linked_tables_.m_peptide_table.RecordsToString(peptide_vector, column_name_string));

			// Creates a string containing the required characters.
			std::string invalid_string(1, this->m_output_characters_.invalid_character);
			std::string separator_string(1, this->m_output_characters_.record_delimiter);

			// Adds the records to the output vector.
			size_t record_index = 0;
			for (IsotopicCluster* cluster : input_vector)
			{
				std::string composed_record("");

				if (cluster->identification)
				{
					for (IdentificationItem* item : cluster->identification->identification_items)
					{
						if (item->peptide_reference)
						{
							composed_record += records[record_index] + separator_string;
							++record_index;
						}
					}
				}

				if (!composed_record.empty())
				{
					composed_record.erase(composed_record.size() - 1);
					output_vector.push_back(composed_record);
				}
				else
				{
					output_vector.push_back(invalid_string);
				}
			}
		});

		// ProteinGroup output function.
		temporary_switch_object.AddFunctor(m_linked_tables_.m_protein_group_table.GetTableName(), [this, &column_name_string, &input_vector, &output_vector]
		{
			// Ensures that the vector will only contain the new strings.
			output_vector.clear();
			output_vector.reserve(input_vector.size());

			// Creates the vector that will be used to extract the record data.
			std::vector<ProteinGroup*> protein_group_vector;
			protein_group_vector.reserve(input_vector.size());

			for (IsotopicCluster* cluster : input_vector)
			{
				if (cluster->identification)
				{
					
					for (IdentificationItem* item : cluster->identification->identification_items)
					{
						for (ProteinHypothesis* hypothesis : item->protein_hypotheses_relations)
						{
							protein_group_vector.push_back(hypothesis->protein_group_relation);
						}
					}
				}
			}

			// Extracts the records.
			std::vector<std::string> records(this->m_linked_tables_.m_protein_group_table.RecordsToString(protein_group_vector, column_name_string));

			// Creates a string containing the required characters.
			std::string invalid_string(1, this->m_output_characters_.invalid_character);
			std::string separator_string(1, this->m_output_characters_.record_delimiter);

			// Adds the records to the output vector.
			size_t record_index = 0;
			for (IsotopicCluster* cluster : input_vector)
			{
				std::string composed_record("");

				if (cluster->identification)
				{
					for (IdentificationItem* item : cluster->identification->identification_items)
					{
						for (ProteinHypothesis* hypothesis : item->protein_hypotheses_relations)
						{
							composed_record += records[record_index] + separator_string;
							++record_index;
						}
					}
				}

				if (!composed_record.empty())
				{
					composed_record.erase(composed_record.size() - 1);
					output_vector.push_back(composed_record);
				}
				else
				{
					output_vector.push_back(invalid_string);
				}
			}
		});

		// ProteinHypothesis output function.
		temporary_switch_object.AddFunctor(m_linked_tables_.m_protein_hypothesis_table.GetTableName(), [this, &column_name_string, &input_vector, &output_vector]
		{
			// Ensures that the vector will only contain the new strings.
			output_vector.clear();
			output_vector.reserve(input_vector.size());

			// Creates the vector that will be used to extract the record data.
			std::vector<ProteinHypothesis*> protein_hypothesis_vector;
			protein_hypothesis_vector.reserve(input_vector.size());

			for (IsotopicCluster* cluster : input_vector)
			{
				if (cluster->identification)
				{

					for (IdentificationItem* item : cluster->identification->identification_items)
					{
						for (ProteinHypothesis* hypothesis : item->protein_hypotheses_relations)
						{
							protein_hypothesis_vector.push_back(hypothesis);
						}
					}
				}
			}

			// Extracts the records.
			std::vector<std::string> records(this->m_linked_tables_.m_protein_hypothesis_table.RecordsToString(protein_hypothesis_vector, column_name_string));

			// Creates a string containing the required characters.
			std::string invalid_string(1, this->m_output_characters_.invalid_character);
			std::string separator_string(1, this->m_output_characters_.record_delimiter);

			// Adds the records to the output vector.
			size_t record_index = 0;
			for (IsotopicCluster* cluster : input_vector)
			{
				std::string composed_record("");

				if (cluster->identification)
				{
					for (IdentificationItem* item : cluster->identification->identification_items)
					{
						for (ProteinHypothesis* hypothesis : item->protein_hypotheses_relations)
						{
							composed_record += records[record_index] + separator_string;
							++record_index;
						}
					}
				}

				if (!composed_record.empty())
				{
					composed_record.erase(composed_record.size() - 1);
					output_vector.push_back(composed_record);
				}
				else
				{
					output_vector.push_back(invalid_string);
				}
			}
		});

		return temporary_switch_object;
	}

	// Returns the header for the table files.
	std::string DataExtraction::GetHeader(void)
	{
		// Converts the various characters to string for ease of use later on.
		std::string column_delimiter_as_string = std::string(1, m_output_characters_.column_delimiter);

		std::string header("");

		header += "metapeak" + column_delimiter_as_string + column_delimiter_as_string;

		for (size_t file = 0; file < m_linked_tables_.m_file_table.size(); ++file)
		{
			header += "file" + std::to_string(file) + column_delimiter_as_string;
		}

		header.erase(header.size() - 1);

		return header;
	}

	// Writes the properties defined to the defined directory.
	void DataExtraction::WritePeakTables(std::string prefix, std::string postfix, std::vector<ReferenceTableRecord>& reference_table, std::vector<std::string>& properties)
	{
		// Creates a vector with all the valid peaks.
		std::vector<Peak*> peaks;
		for (ReferenceTableRecord& record : reference_table)
		{
			for (std::vector<Peak*>& file_peaks : record.file_separated_peaks)
			{
				for (Peak* peak : file_peaks)
				{
					peaks.push_back(peak);
				}
			}
		}

		// Converts the various characters to string for ease of use later on.
		std::string column_delimiter_as_string = std::string(1, m_output_characters_.column_delimiter);
		std::string peak_delimiter_as_string = std::string(1, m_output_characters_.peak_delimiter);
		std::string invalid_as_string = std::string(1, m_output_characters_.invalid_character);

		// Validates the passed properties.
		std::unordered_set<std::string> validated_properties;
		for (std::string& property : properties)
		{
			if (property == "peak_mz" || property == "peak_rt" || property == "peak_intensity")
			{
				validated_properties.insert(property);
			}
		}

		// Outputs a table for each of the validated properties.
		for (const std::string& property : validated_properties)
		{
			m_writer_.OpenFile(prefix + property + postfix);

			if (m_writer_.IsFileOpen())
			{
				// Collects a string property for each peak.
				std::vector<std::string> peak_properties(m_linked_tables_.m_peak_table.RecordsToString(peaks, property));

				// Tracks the current peak string to output.
				size_t current_peak = 0;

				// Writes the header.
				m_writer_ << GetHeader() << "\n";

				// Loops through all the records.
				for (ReferenceTableRecord& record : reference_table)
				{
					m_writer_ << std::to_string(record.metapeak->metapeak_id) + column_delimiter_as_string;

					// Loops through all the files.
					for (std::vector<Peak*>& file_peaks : record.file_separated_peaks)
					{
						// Outputs the peaks of a file or an invalid sign if no peaks are available.
						if (!file_peaks.empty())
						{
							// Loops through all the peaks of a file.
							for (Peak* peak : file_peaks)
							{
								// Acquires and writes the peak property.
								m_writer_ << peak_properties[current_peak];
								++current_peak;

								// Checks if a delimiter needs to be added.
								if (peak->peak_id != file_peaks.back()->peak_id)
								{
									m_writer_ << peak_delimiter_as_string;
								}
							}
						}
						else
						{
							m_writer_ << invalid_as_string;
						}

						// Separates the different file columns.
						m_writer_ << column_delimiter_as_string;
					}

					m_writer_ << "\n";
				}

				// Closes the file.
				m_writer_.CloseFile();
			}
		}
	}

	// Writes the properties defined to the defined directory.
	void DataExtraction::WritePropertyTables(std::string prefix, std::string postfix, std::vector<PropertyOutputRecord>& property_table, std::vector<std::string>& properties)
	{
		// Creates a vector of all the isotopic clusters that are present within the property_table
		std::vector<IsotopicCluster*> clusters;
		for (PropertyOutputRecord& record : property_table)
		{
			for (std::vector<IsotopicCluster*> file_clusters : record.file_separated_clusters)
			{
				for (IsotopicCluster* cluster : file_clusters)
				{
					if (cluster)
					{
						clusters.push_back(cluster);
					}
				}
			}
		}

		// Creates the variables that'll be used to communicate with the to_string_switch.
		std::string column_name;
		std::vector<std::string> output_vector;

		// Initializes the to_string_switch
		Utilities::FunctorSwitch<std::string> to_string_switch(CreateToStringSwitch(column_name, clusters, output_vector));

		// Initializes a map that links property names to their table names.
		std::unordered_map<std::string, std::string> property_to_table_map(CreatePropertyToTableMap());

		// Converts the various characters to string for ease of use later on.
		std::string column_delimiter_as_string = std::string(1, m_output_characters_.column_delimiter);
		std::string peak_delimiter_as_string = std::string(1, m_output_characters_.peak_delimiter);
		std::string invalid_as_string = std::string(1, m_output_characters_.invalid_character);

		// Validates the passed properties.
		std::unordered_set<std::string> validated_properties;
		for (std::string& property : properties)
		{
			if (property_to_table_map.find(property) != property_to_table_map.end())
			{
				validated_properties.insert(property);
			}
		}

		// Loops through all the properties and writes their respective tables to the defined directory.
		for (const std::string& property : validated_properties)
		{
			// Opens the new file.
			m_writer_.OpenFile(prefix + property + postfix);

			// Only continues to write the file if the file can be opened.
			if (m_writer_.IsFileOpen())
			{
				// Acquires the properties as strings.
				column_name = property;
				to_string_switch.CallFunctor(property_to_table_map.find(property)->second);

				// Keeps track of the property strings.
				size_t property_record_index = 0;

				// Writes the header.
				m_writer_ << GetHeader() << "\n";

				for (PropertyOutputRecord& record : property_table)
				{
					// Writes the metapeak and optimal peak information.
					m_writer_ << std::to_string(record.metapeak_id) << column_delimiter_as_string;

					for (std::vector<IsotopicCluster*> file_clusters : record.file_separated_clusters)
					{
						// If the cluster vector is empty, write an invalid character, otherwise add the cluster information.
						if (file_clusters.empty())
						{
							m_writer_ << invalid_as_string;
						}
						else
						{
							for (size_t cluster = 0; cluster < file_clusters.size(); ++cluster)
							{
								// Writes the property or an invalid sign.
								if (file_clusters[cluster])
								{
									m_writer_ << output_vector[property_record_index];
									++property_record_index;
								}
								else
								{
									m_writer_ << invalid_as_string;
								}

								// Writes a delimiter when required.
								if (cluster != file_clusters.size() - 1)
								{
									m_writer_ << peak_delimiter_as_string;
								}
							}
						}

						// Writes the column delimiter for every file.
						m_writer_ << column_delimiter_as_string;
					}

					// writes the file end after each record.
					m_writer_ << "\n";
				}

				// Closes the file and notifies it as succesful.
				m_writer_.CloseFile();
			}
		}
	}

	// Writes the reference table to the defined path.
	void DataExtraction::WriteReferenceTable(std::string prefix, std::string postfix, std::vector<ReferenceTableRecord>& reference_table)
	{
		// Opens the file.
		m_writer_.OpenFile(prefix + "reference_table" + postfix);

		if (m_writer_.IsFileOpen())
		{
			// Converts the various characters to string for ease of use later on.
			std::string column_delimiter_as_string = std::string(1, m_output_characters_.column_delimiter);
			std::string peak_delimiter_as_string = std::string(1, m_output_characters_.peak_delimiter);
			std::string invalid_as_string = std::string(1, m_output_characters_.invalid_character);

			m_writer_ << GetHeader() << "\n";

			// Loops through all the records.
			for (ReferenceTableRecord& record : reference_table)
			{
				m_writer_ << std::to_string(record.metapeak->metapeak_id) + column_delimiter_as_string;

				// Loops through all the files.
				for (std::vector<Peak*>& file_peaks : record.file_separated_peaks)
				{
					// Outputs the peaks of a file or an invalid sign if no peaks are available.
					if (!file_peaks.empty())
					{
						// Loops through all the peaks of a file.
						for (Peak* peak : file_peaks)
						{
							m_writer_ << std::to_string(peak->peak_id);

							// Checks if a delimiter needs to be added.
							if (peak->peak_id != file_peaks.back()->peak_id)
							{
								m_writer_ << peak_delimiter_as_string;
							}
						}
					}
					else
					{
						m_writer_ << invalid_as_string;
					}

					// Separates the different file columns.
					m_writer_ << column_delimiter_as_string;
				}

				m_writer_ << "\n";
			}

			// Closes the file.
			m_writer_.CloseFile();
		}
	}
}
