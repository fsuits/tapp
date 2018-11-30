#pragma once

#include <string>
#include <unordered_map>

#include "Filetypes/Mzid/Mzid_File.h"
#include "Filetypes/MzXML/MzXML_File.h"

// TODO: Add descriptions for each struct and function.

/*	Records

	All records have been setup with two way relationships through pointers.
*/

using namespace TAPP::Filetypes::Mzid;
using namespace TAPP::Filetypes::MzXML;

namespace TAPP::MassSpectrometry::RelationalTables
{
	/*** Forward Declarations ***************************************************/

	/*** MZID ***/
	struct FastaSequence;
	struct FragmentIon;
	struct IdentificationItem;
	struct IdentificationResult;
	struct Peptide;
	struct PeptideEvidence;
	struct ProteinGroup;
	struct ProteinHypothesis;
	struct PeptideModification;

	/*** MZXML ***/
	struct Event;
	struct Scan;

	/*** TAPP ***/
	struct File;
	struct IsotopicCluster;
	struct Metapeak;
	struct Peak;

	/*** Structs ****************************************************************/

	/*** MZID ***/

	struct FastaSequence
	{
		std::string									description;
		std::string									id;

		std::vector<PeptideEvidence*>				evidence_relations;
		std::vector<ProteinHypothesis*>				hypothesis_relations;
	};

	struct FragmentIon
	{
		std::string									indices;
		std::string									name;
		unsigned char								charge;
		std::vector<double>							mz_values;
		std::vector<double>							intensity_values;
		std::vector<double>							error_values;

		IdentificationItem*							identification_item_relation;
	};

	struct IdentificationItem
	{
		std::string									id;
		unsigned char								chargestate;
		double										calculated_mz;
		double										experimental_mz;
		bool										passed;
		unsigned short								rank;
		Peptide*									peptide_reference;
		std::vector<CV_Parameter>					cv_parameters;
		std::vector<FragmentIon*>					fragmentations;
		std::vector<PeptideEvidence*>				peptide_evidence;

		IdentificationResult*						identification_result_relation;
		std::vector<ProteinHypothesis*>				protein_hypotheses_relations;
	};

	struct IdentificationResult
	{
		std::string									id;
		size_t										index;
		double										retention_time;
		Scan*										scan;
		std::vector<IdentificationItem*>			identification_items;

		std::vector<IsotopicCluster*>				isotopic_cluster_relations;
	};

	struct Peptide
	{
		std::string									id;
		std::string									sequence;
		std::vector<PeptideModification*>			modifications;

		std::vector<IdentificationItem*>			identification_item_relations;
		std::vector<PeptideEvidence*>				peptide_evidence_relations;
	};

	struct PeptideEvidence
	{
		std::string									id;
		unsigned short								start;
		unsigned short								end;
		char										pre;
		char										post;
		bool										is_decoy;
		Peptide*									petide_reference;
		FastaSequence*								sequence_reference;

		std::vector<IdentificationItem*>			identification_item_relations;
		std::vector<ProteinHypothesis*>				protein_hypothesis_relations;
	};

	struct PeptideModification
	{
		unsigned short								accession;
		unsigned short								location;
		double										monoisotopic_mass_delta;
		char										residues;
		std::string*								cv_reference;
		std::string*								name;

		Peptide*									peptide_relation;
	};

	struct ProteinGroup
	{
		std::string									id;
		std::vector<CV_Parameter>				cv_parameters;
		std::vector<ProteinHypothesis*>				hypotheses;
	};

	struct ProteinHypothesis
	{
		std::string									id;
		bool										passed_threshold;
		PeptideEvidence*							evidence;
		FastaSequence*								sequence;
		std::vector<IdentificationItem*>			identification_items;

		ProteinGroup*								protein_group_relation;
	};

	/*** MZXML ***/

	struct Event
	{
		Scan*										spectral_scan;
		Scan*										precursor_scan;
		unsigned char								precursor_charge_state;
		double										precursor_intensity;
		double										precursor_mz;

		IsotopicCluster*							cluster_relation;
	};

	struct Scan
	{
		size_t										scan_id;
		size_t										amount_of_peaks;
		unsigned char								ms_level;
		char										polarity;
		double										min_mz;
		double										max_mz;
		double										event_peak_mz;
		double										event_peak_intensity;
		double										retention_time;
		double										total_ion_current;

		File*										file_relation;
		IdentificationResult*						identification_result_relation;
		std::vector<Event*>							event_relations;
	};

	/*** TAPP ***/

	struct File
	{
		unsigned char								file_id;
		unsigned char								identification_class;

		std::vector<IdentificationResult*>			identification_result_relations;
		std::vector<IsotopicCluster*>				isotopic_cluster_relations;
		std::vector<Peak*>							peak_relations;
		std::vector<Scan*>							scan_relations;
	};

	struct IsotopicCluster
	{
		size_t										id;
		size_t										group;
		char										identification_method;
		unsigned char								chargestate;
		Peak*										event_peak; // The initial peak used to calculate the cluster.
		Event*										event; // The event on which the identification is based.
		IdentificationResult*						identification; // The identification of the event.

		std::vector<Peak*>							clustered_peaks; // All peaks from the cluster.

		File*										file_relation;
	};

	struct Metapeak
	{
		size_t										metapeak_id;
		std::vector<Peak*>							peaks;
	};

	/*	Peak
	<summary>Represents a peak.</summary>
	*/
	struct Peak
	{
		size_t										peak_id;
		double										mz;
		double										rt;
		double										intensity;
		File*										file_relation;

		Metapeak*									metapeak_relation;
		std::vector<IsotopicCluster*>				cluster_relations;
	};


}
