// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "MassSpectrometry/RelationalTables/LinkedTables.h"
#include "MassSpectrometry/RelationalTables/Records.h"
#include "IO/BufferedWriter.h"
#include "Utilities/FunctorSwitch.hpp"

namespace TAPP::MassSpectrometry::RelationalTables
{
	/*** Typedefs *************************************************************/

	typedef std::function<IsotopicCluster*(IsotopicCluster*, IsotopicCluster*)> ClusterSelectionMethod;
	typedef std::function<Peak*(Peak*, Peak*)> PeakSelectionMethod;

	/*** Structs **************************************************************/

	struct OutputCharacters
	{
		char column_delimiter;
		char peak_delimiter;
		char record_delimiter;
		char value_delimiter;
		char invalid_character;
	};

	struct ReferenceTableRecord
	{
		Metapeak*									metapeak;
		std::vector<std::vector<Peak*>>				file_separated_peaks;
	};

	struct PropertyOutputRecord
	{
		size_t										metapeak_id;
		std::vector<std::vector<IsotopicCluster*>>	file_separated_clusters;
	};

	/*	DataExtraction
		<summary>This class can be used to extract and output data from a linked_tables object. It is not thread safe.</summary>
	*/
	class DataExtraction
	{
		public:
			DataExtraction(LinkedTables& linked_tables, OutputCharacters& output_characters, size_t maximum_buffer_size = 50000000);

			std::vector<PropertyOutputRecord> CreatePropertyTableBlueprint(std::vector<ReferenceTableRecord>& reference_table, std::vector<IsotopicCluster*>& approved_clusters, ClusterSelectionMethod cluster_compare);
			std::vector<ReferenceTableRecord> CreateReferenceTable(std::vector<IsotopicCluster*>& approved_clusters, PeakSelectionMethod peak_compare);

			std::unordered_map<std::string, std::string> CreatePropertyToTableMap(void);
			Utilities::FunctorSwitch<std::string> CreateToStringSwitch(std::string& column_name_string, std::vector<IsotopicCluster*>& input_vector, std::vector<std::string>& output_vector);

			std::string GetHeader(void);

			void WritePeakTables(std::string prefix, std::string postfix, std::vector<ReferenceTableRecord>& reference_table, std::vector<std::string>& properties);
			void WritePropertyTables(std::string prefix, std::string postfix, std::vector<PropertyOutputRecord>& property_table, std::vector<std::string>& properties);
			void WriteReferenceTable(std::string prefix, std::string postfix, std::vector<ReferenceTableRecord>& reference_table);

		private:
			LinkedTables&		m_linked_tables_;
			OutputCharacters	m_output_characters_;
			IO::BufferedWriter	m_writer_;
	};
}
