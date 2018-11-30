#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "MassSpectrometry/RelationalTables/Records.h"
#include "Filetypes/Mzid/Mzid_Parser.h"
#include "Filetypes/MzXML/MzXML_Parser.h"
#include "Filetypes/TAPP/IPL.h"
#include "Filetypes/TAPP/PID.h"
#include "Collections/Table.hpp"

namespace TAPP::MassSpectrometry::RelationalTables
{
	class LinkedTables
	{
		public:
			/*** Public Variables *******************************************************/

			/*** MZXML ***/

			TAPP::Collections::Table<Event>					m_event_table;
			TAPP::Collections::Table<Scan>					m_scan_table;

			/*** MZID ***/

			TAPP::Collections::Table<FastaSequence>			m_fasta_sequence_table;
			TAPP::Collections::Table<FragmentIon>			m_fragment_ion_table;
			TAPP::Collections::Table<IdentificationItem>	m_identification_item_table;
			TAPP::Collections::Table<IdentificationResult>	m_identification_result_table;
			TAPP::Collections::Table<Peptide>				m_peptide_table;
			TAPP::Collections::Table<PeptideEvidence>		m_peptide_evidence_table;
			TAPP::Collections::Table<PeptideModification>	m_peptide_modification_table;
			TAPP::Collections::Table<ProteinGroup>			m_protein_group_table;
			TAPP::Collections::Table<ProteinHypothesis>		m_protein_hypothesis_table;

			/*** TAPP ***/

			TAPP::Collections::Table<IsotopicCluster>		m_cluster_table;
			TAPP::Collections::Table<File>					m_file_table;
			TAPP::Collections::Table<Metapeak>				m_metapeak_table;
			TAPP::Collections::Table<Peak>					m_peak_table;

			/*** Constructors / Destructor **********************************************/

			LinkedTables(void);
			LinkedTables
			(
				const std::vector<TAPP::Filetypes::TAPP::PID>& pid_entries,
				const TAPP::Filetypes::Mzid::SpectrumIdentificationItem* item_example,
				const TAPP::Filetypes::Mzid::ProteinGroup* group_example,
				const char delimiter_char = ';',
				const char invalid_char = '-'
			);
			LinkedTables(const LinkedTables& other) = delete;
			LinkedTables(LinkedTables&& other);


			~LinkedTables(void);

			LinkedTables& operator=(const LinkedTables& other) = delete;
			LinkedTables& operator=(LinkedTables&& other);

			/*** Public Functions *******************************************************/

			void InsertIdentificationTables(const size_t file_id, const std::vector<Filetypes::TAPP::IPL>& ipl_entries, const Mzid_File& mzid_file);
			void InsertScanTable(const size_t file_id, const std::vector<Filetypes::TAPP::IPL>& ipl_entries, const MzXML_File& mzxml_file);

		private:
			/*** Private Variables ******************************************************/

			char															m_delimiter_;
			char															m_invalid_;

			// Used to store various strings in order to reduce memory requirements.

			std::unordered_map<size_t, std::string>							m_cv_references_;
			std::unordered_map<size_t, std::string>							m_modification_names_;
			std::unordered_set<std::string>									m_cv_param_names_;

			std::vector<std::pair<std::string, std::vector<std::string>>>	m_property_vectors_;

			/*** Private Functions ******************************************************/

			// void InitializeTables_(void);

			void InsertPID_(const std::vector<Filetypes::TAPP::PID>& pid_entries);
	};
}
