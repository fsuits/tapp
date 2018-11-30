#include "Filetypes/FileRelations/FileRelations.h"

#include <algorithm>
#include <cmath>
#include <inttypes.h>
#include <math.h>

using namespace TAPP::Filetypes::Mzid;
using namespace TAPP::Filetypes::MzXML;

namespace TAPP::Filetypes::FileRelations
{
	// Links scan events to identifications.
	// If no mzid file is provided, the vector will only contain the available information from the mzxml file.
	std::vector<ScanToIdentificationsRelation> CreateScanToIdentificationRelation(MzXML_File& mzxml_file, Mzid_File& mzid_file)
	{
		// Creates a Scan id to vector map and a Scan to Identification map.
		std::unordered_map<size_t, ScanToIdentificationsRelation*> scan_map;
		std::vector<ScanToIdentificationsRelation> scan_to_identification;
		scan_map.reserve(mzxml_file.scans.size());
		scan_to_identification.reserve(mzxml_file.scans.size());

		// Fills the maps with the available scan information.
		for (std::pair<const size_t, MzXML::Scan>& scan : mzxml_file.scans)
		{
			scan_to_identification.push_back({ &scan.second, nullptr, nullptr });
			scan_map.insert({ scan.first, &scan_to_identification.back() });
		}

		// Adds the Event relationships within the struct.
		for (std::pair<const size_t, MzXML::Event>& event_entry : mzxml_file.events)
		{
			scan_map.find(event_entry.first)->second->ms_event = &event_entry.second;
		}

		// Acquires a scan annotated map for the identifications and creates the identification relationship if an mzid file is available.
		if (!mzid_file.filename.empty())
		{
			std::string mzid_filename = mzxml_file.filename.substr(0, mzxml_file.filename.find_last_of('.')) + ".mgf";
			Mzid_File::ScanAnnotatedIdentificationResults scan_annotated_identification_results(mzid_file.GetScanAnnotatedIdentificationResults(mzid_filename));

			if (scan_annotated_identification_results[0].second->scan_id != (size_t)-1)
			{
				for (auto result : scan_annotated_identification_results)
				{
					scan_map.find(result.first)->second->identification = result.second;
				}
			}
			else
			{
				// For now, do nothing. But can be utilized to create a log.
			}
		}

		return scan_to_identification;
	}

	// Links peaks to scan events.
	// If no mzxml file is provided, the vector will only contain the available information from the mesh object.
	std::vector<PeakToEventsRelation> CreatePeakToEventRelation(const double precursor_mz_window, const Mesh& mesh, MzXML::MzXML_File& mzxml_file)
	{
		std::vector<PeakToEventsRelation> peak_to_scans;
		peak_to_scans.reserve(mesh.peaks.size());

		// Creates the initial map of peaks.
		for (Peak* peak : mesh.indexed_peaks)
		{
			peak_to_scans.push_back({ peak });
		}

		if (!mzxml_file.filename.empty())
		{
			// Defines a region of interest.
			float time_units	= mesh.mConversion.mRTReduction;
			double mz_min		= mesh.mConversion.mMinMZ;
			double mz_max		= mesh.mConversion.mMaxMZ;
			double rt_min		= mesh.mConversion.mMinRT * time_units;
			double rt_max		= mesh.mConversion.mMaxRT * time_units;

			size_t out_of_scope = 0;
			for (std::pair<const size_t, MzXML::Event>& event : mzxml_file.events)
			{
				MzXML::Event& event_ref = event.second;

				// Checks if the event is within the scope of the TAPP parameters.
				if (event_ref.precursor_mz >= mz_min &&
					event_ref.precursor_mz <= mz_max &&
					event_ref.spectral_scan->retention_time >= rt_min &&
					event_ref.spectral_scan->retention_time <= rt_max)
				{
					int hit_index = mesh.Index(mesh.mConversion.IndexToMeshX(event_ref.precursor_mz),
						mesh.mConversion.IndexToMeshY(event_ref.spectral_scan->retention_time / time_units));

					int peak_index = GetHitValue(hit_index, mesh);
					if (peak_index != 0)
					{
						peak_to_scans[peak_index].events.push_back(&event.second);
					}
					// If the MZ Tolerance has been defined, look for the nearest peak.
					else if (precursor_mz_window != 0)
					{
						// Calculates the range to analyse.
						int current_hit_index = mesh.Index(mesh.mConversion.IndexToMeshX(event_ref.precursor_mz - precursor_mz_window),
							mesh.mConversion.IndexToMeshY(event_ref.spectral_scan->retention_time / time_units));
						int max_hit_index = mesh.Index(mesh.mConversion.IndexToMeshX(event_ref.precursor_mz + precursor_mz_window),
							mesh.mConversion.IndexToMeshY(event_ref.spectral_scan->retention_time / time_units));

						// Maps the peaks within range and selects the one with the highest intensity.
						std::unordered_set<size_t> peak_index_map;
						int current_highest_intensity_peak = -1;

						while (current_hit_index <= max_hit_index)
						{
							// Adds a peak to the set, value must be anything other than 0.
							int potential_peak_index = GetHitValue(current_hit_index, mesh);
							if (potential_peak_index != 0)
							{
								peak_index_map.insert(potential_peak_index);

								// Checks if the newly added peak has an higher intensity than the established peak.
								if (current_highest_intensity_peak == -1)
								{
									current_highest_intensity_peak = potential_peak_index;
								}
								else if (peak_to_scans[potential_peak_index].peak->mHeight < peak_to_scans[current_highest_intensity_peak].peak->mHeight)
								{
									current_highest_intensity_peak = potential_peak_index;
								}
							}
							++current_hit_index;
						}

						if (current_highest_intensity_peak != -1)
						{
							// Converts the mapping to a vector.
							std::vector<Peak*> peak_index_vector;
							peak_index_vector.reserve(peak_index_map.size());
							for (size_t peak : peak_index_map)
							{
								peak_index_vector.push_back(mesh.indexed_peaks[peak]);
							}

							// Adds the information to the struct.
							peak_to_scans[current_highest_intensity_peak].events.push_back(&event.second);
							peak_to_scans[current_highest_intensity_peak].potential_peaks = peak_index_vector;
						}
					}
				}
				else
				{
					++out_of_scope;
				}
			}
		}

		return peak_to_scans;
	}

	size_t GetHitValue(const size_t hit_index, const Mesh& mesh)
	{
		int peak_index = 0;

		if (!((hit_index < 0 || hit_index >= (mesh.mConversion.mNMZ - 1) * (mesh.mConversion.mNRT - 1))) && // Not outside of the range.
			mesh.hit[hit_index] != 0) // No hit value.
		{
			// Calculates the index for the mesh.indexed_peaks vector.
			peak_index = sqrt(int64_t(mesh.hit[hit_index]) * int64_t(mesh.hit[hit_index])) - 1;

			// If not present within the peak list.
			if (peak_index >= mesh.peaks.size())
			{
				peak_index = 0;
				//peak_to_scans[peak_index].events.push_back(&event.second);
			}
		}

		return peak_index;
	}
}
