#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Filetypes/Mzid/Mzid_File.h"
#include "Filetypes/MzXML/MzXML_File.h"
#include "Mesh/Mesh.hpp"

using namespace TAPP::Filetypes::Mzid;
using namespace TAPP::Filetypes::MzXML;

namespace TAPP::Filetypes::FileRelations
{
	struct ScanToIdentificationsRelation
	{
		Scan*							scan;
		Event*							ms_event; // This allows for easy tracking, despite the event already having a pointer towards its scan.
		SpectrumIdentificationResult*	identification;
	};

	struct PeakToEventsRelation
	{
		Peak*				peak;
		std::vector<Event*>	events;
		std::vector<Peak*>	potential_peaks;
	};

	std::vector<ScanToIdentificationsRelation> CreateScanToIdentificationRelation(MzXML_File& mzxml_file, Mzid_File& mzid_file);
	std::vector<PeakToEventsRelation> CreatePeakToEventRelation(const double mz_tolerance, const Mesh& mesh, MzXML_File& mzxml_file);
	size_t GetHitValue(const size_t hit_index, const Mesh& mesh);
}
