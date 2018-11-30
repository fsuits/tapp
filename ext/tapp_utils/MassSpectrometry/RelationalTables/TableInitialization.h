#pragma once

#include <string>
#include <vector>

#include "MassSpectrometry/RelationalTables/Records.h"
#include "Filetypes/Mzid/Mzid_File.h"
#include "Filetypes/MzXML/MzXML_File.h"
#include "Collections/Table.hpp"

using namespace TAPP::Filetypes::Mzid;
using namespace TAPP::Filetypes::MzXML;

namespace TAPP::MassSpectrometry::RelationalTables
{
	/*** MZXML ***/

	TAPP::Collections::Table<Event> InitializeEventTable(const std::string table_name);
	TAPP::Collections::Table<Scan> InitializeScanTable(const std::string table_name);

	/*** MZID ***/

	TAPP::Collections::Table<FastaSequence> InitializeFastaSequenceTable(const std::string table_name);
	TAPP::Collections::Table<FragmentIon> InitializeFragmentIonTable(const std::string table_name, const char delimiter = ',');
	TAPP::Collections::Table<IdentificationItem> InitializeIdentificationItemTable(const std::string table_name, const TAPP::Filetypes::Mzid::SpectrumIdentificationItem* item, const char delimiter = ',');
	TAPP::Collections::Table<IdentificationResult> InitializeIdentificationResultTable(const std::string table_name, const char delimiter = ',', const char invalid = '-');
	TAPP::Collections::Table<Peptide> InitializePeptideTable(const std::string table_name, const char delimiter = ',');
	TAPP::Collections::Table<PeptideEvidence> InitializePeptideEvidenceTable(const std::string table_name);
	TAPP::Collections::Table<PeptideModification> InitializePeptideModificationTable(const std::string table_name);
	TAPP::Collections::Table<ProteinGroup> InitializeProteinGroupTable(const std::string table_name, const TAPP::Filetypes::Mzid::ProteinGroup* protein_group, const char delimiter = ',');
	TAPP::Collections::Table<ProteinHypothesis> InitializeProteinHypothesisTable(const std::string table_name, const char delimiter = ',');

	/*** TAPP ***/

	TAPP::Collections::Table<File> InitializeFileTable(const std::string table_name);
	TAPP::Collections::Table<IsotopicCluster> InitializeIsotopicClusterTable(const std::string table_name, const char delimiter = ',', const char invalid = '-');
	TAPP::Collections::Table<Metapeak> InitializeMetapeakTable(const std::string table_name, const char delimiter = ',');
	TAPP::Collections::Table<Peak> InitializePeakTable(const std::string table_name);
}
