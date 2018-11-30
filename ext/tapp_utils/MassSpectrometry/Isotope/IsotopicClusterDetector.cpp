#include "MassSpectrometry/Isotope/IsotopicClusterDetector.h"  // loads midas.h

#include <cmath>
#include <unordered_map>

#include "MassSpectrometry/Isotope/Averagine.h"
#include "MassSpectrometry/CommonSortings.h"

using namespace std;
using namespace TAPP::Filetypes::FileRelations;

namespace TAPP::MassSpectrometry::Isotope
{
	/*** Constructors ***********************************************************/

	IsotopicClusterDetector::IsotopicClusterDetector(double error_tolerance, double mz_sigma_range, double rt_sigma_range, unsigned char max_chargestate, unsigned char midas_mode)
		: m_error_tolerance_(error_tolerance), m_midas_mode_(midas_mode), m_mz_sigma_range_(mz_sigma_range), m_rt_sigma_range_(rt_sigma_range), m_max_chargestate_(max_chargestate)
	{
	}

	/*** Public Functions *******************************************************/

	// Get Ms/Ms identified peaks
	// Calculate other peaks nearby through charge state and sigma error allowance
	// Apply isotopic distribution through midas
	vector<IsotopicCluster> IsotopicClusterDetector::DetectClusters(vector<PeakToEventsRelation>& peak_list,
		vector<ScanToIdentificationsRelation>& scan_list, bool detect_structure)
	{
		// Disables any MIDAS output.
		std::cout.setstate(std::ios_base::failbit);

		m_averagine_table_ = TAPP::MassSpectrometry::Isotope::CreateAveragineTable();
		CreateOrderedList_(peak_list);

		// Initializes the initial vector which will be concatenated with the additional information.
		vector<IsotopicCluster> clusters;

		// Retrieves the event and identification based clusters.
		if (!scan_list.empty())
		{
			vector<IsotopicCluster> event_based_clusters(DetectEventBasedClusters_(peak_list, scan_list));
			clusters.reserve(event_based_clusters.size());
			clusters.insert(clusters.end(), event_based_clusters.begin(), event_based_clusters.end());
		}

		if (detect_structure)
		{
			vector<IsotopicCluster> structure_based_clusters(DetectStructureBasedClusters_(peak_list, clusters));
			clusters.reserve(clusters.size() + structure_based_clusters.size());
			clusters.insert(clusters.end(), structure_based_clusters.begin(), structure_based_clusters.end());
		}

		m_averagine_table_.clear();
		m_rt_ordered_peaks_.clear();

		// Enables output once again.
		std::cout.clear();

		return clusters;
	}

	/*** Private Functions ******************************************************/

	// Creates and orders a mz vector and rt map.
	void IsotopicClusterDetector::CreateOrderedList_(vector<PeakToEventsRelation>& peak_list)
	{
		// Creates a sorted mz ordered peak list.
		m_rt_ordered_peaks_.reserve(peak_list.size());
		for (PeakToEventsRelation& peak : peak_list)
		{
			m_rt_ordered_peaks_.push_back(&peak);
		}
		//sort(m_mz_ordered_peaks_.begin(), m_mz_ordered_peaks_.end(), AscendingMassToCharge);

		sort(m_rt_ordered_peaks_.begin(), m_rt_ordered_peaks_.end(), [](PeakToEventsRelation* a, PeakToEventsRelation* b)
		{
			return a->peak->mY < b->peak->mY;
		});

		// Maps the first rt value occurance to its index in the rt ordered vector.
		size_t size = m_rt_ordered_peaks_.size();
		for (size_t index = 0; index < size; ++index)
		{
			m_rt_map_.insert({ floor(m_rt_ordered_peaks_[index]->peak->mY), index });
		}
	}

	vector<Peak*> IsotopicClusterDetector::CalculateIsotopicClusterPeaks_(PeakToEventsRelation& base_peak, unsigned char chargestate,
		DistributionTable& distribution_table)
	{
		// Detects base peak position within the isotopic distribution table.
		int isotopic_shift = DetermineIsotopicDistributionShift_(base_peak, chargestate, distribution_table);

		// Defines the Isotopic Cluster
		std::vector<Peak*> cluster;

		// Moves into the descending direction.
		if (isotopic_shift > 0)
		{
			Peak* reference_peak	= base_peak.peak;
			int current_isotope		= isotopic_shift - 1;

			while (current_isotope != -1 && reference_peak != nullptr)
			{
				// Calculates the mz estimated value and acquires the potential peaks for that region.
				double expected_mz = reference_peak->mX - (1.0 / chargestate);
				double expected_intensity = reference_peak->mHeight * (distribution_table[current_isotope].prob /
					distribution_table[isotopic_shift].prob);

				vector<PeakToEventsRelation*> potential_peaks = GetPeaks_(
					expected_mz,
					reference_peak->mY,
					reference_peak->mXSig,
					reference_peak->mYSig);

				// Loops through the available peaks, determining the best match.
				Peak* best_match = SelectIdealPeak_(reference_peak, potential_peaks, expected_mz, expected_intensity);
				if (best_match != nullptr)
				{
					cluster.push_back(best_match);
					reference_peak = best_match;
				}
				else
				{
					break;
				}
					
				--current_isotope;
			}
		}

		// Moves into the ascending direction.
		if (isotopic_shift < distribution_table.size() - 1)
		{
			Peak* reference_peak = base_peak.peak;
			int current_isotope = isotopic_shift + 1;
			while (current_isotope != distribution_table.size() && reference_peak != nullptr)
			{
				// Calculates the mz estimated value and acquires the potential peaks for that region.
				double expected_mz = reference_peak->mX + (1.0 / chargestate);
				double expected_intensity = reference_peak->mHeight * (distribution_table[current_isotope].prob /
					distribution_table[isotopic_shift].prob);

				vector<PeakToEventsRelation*> potential_peaks = GetPeaks_(
					expected_mz,
					reference_peak->mY,
					reference_peak->mXSig,
					reference_peak->mYSig);

				// Loops through the available peaks, determining the best match.
				Peak* best_match = SelectIdealPeak_(reference_peak, potential_peaks, expected_mz, expected_intensity);
				if (best_match != nullptr)
				{
					cluster.push_back(best_match);
					reference_peak = best_match;
				}
				else
				{
					break;
				}

				++current_isotope;
			}
		}

		// Adds the base peak into the list.
		cluster.push_back(base_peak.peak);

		// Sorts the cluster based on mz, giving it the correct order.
		sort(cluster.begin(), cluster.end(), AscendingMassToCharge);
		return cluster;
	}

	vector<IsotopicCluster> IsotopicClusterDetector::DetectEventBasedClusters_(vector<PeakToEventsRelation>& peak_list,
		vector<ScanToIdentificationsRelation>& scan_list)
	{
		// Creates a new vector which is ordered by intensity.
		vector<PeakToEventsRelation*> intensity_ordered_peaks;
		intensity_ordered_peaks.reserve(peak_list.size());
		for (PeakToEventsRelation& peak : peak_list)
		{
			if (!peak.events.empty())
			{
				intensity_ordered_peaks.push_back(&peak);
			}
		}

		// Orders peaks based on intensity.
		sort(intensity_ordered_peaks.begin(), intensity_ordered_peaks.end(), [](PeakToEventsRelation* a, PeakToEventsRelation* b)
		{
			return a->peak->mHeight > b->peak->mHeight;
		});

		// Maps the identifications to an id.
		unordered_map<size_t, SpectrumIdentificationResult*> identification_map; // scan id - identification
		identification_map.reserve(scan_list.size());
		for (ScanToIdentificationsRelation& scan : scan_list)
		{
			if (scan.identification != nullptr)
			{
				identification_map.insert({ scan.scan->scan_id, scan.identification });
			}
		}

		// Loops through the peaks and attempts to gather clusters for each.
		vector<IsotopicCluster> clusters;
		for (PeakToEventsRelation* peak : intensity_ordered_peaks)
		{
			// Sorts the events based on charge state.
			sort(peak->events.begin(), peak->events.end(), [](const Event* a, const Event* b)
			{
				return a->precursor_charge_state > b->precursor_charge_state;
			});

			// Loops through all the events for this peak.
			unsigned char chargestate = 0;
			for (Event* event : peak->events)
			{
				IsotopicCluster cluster; // Holds the cluster information.

				// Acquires isotopic distribution based on Averagine or Identification.
				string sequence;
				auto identification = identification_map.find(event->spectral_scan->scan_id);
				if (identification != identification_map.end() && identification->second->items[0]->passed) // Identification found.
				{
					sequence = identification->second->items[0]->peptide_reference->sequence;
					cluster.identification = identification->second;
					cluster.identification_method = 'i';
				}
				else// if (event->precursor_charge_state != chargestate)
				{
					sequence = Averagine(m_averagine_table_, event->precursor_mz * event->precursor_charge_state);
					cluster.identification = nullptr;
					cluster.identification_method = 'a';
				}

				if (!sequence.empty())
				{
					chargestate = event->precursor_charge_state;

					// Acquires the isotopic distribution.
					MIDAs midas(chargestate, 1, 1, 1e-150, m_midas_mode_);
					midas.Initialize_Elemental_Composition(sequence, "", "H", "OH", 1);
					DistributionTable isotopic_distribution(midas.Fine_Grained_Isotopic_Distribution());

					cluster.event_peak		= peak->peak;
					cluster.chargestate		= chargestate;
					cluster.clustered_peaks = CalculateIsotopicClusterPeaks_(*peak, chargestate, isotopic_distribution);
					cluster.event			= event;
					clusters.push_back(cluster);
				}
			}
		}

		return clusters;
	}

// implemented function to detect isotope cluster that does not have an MS/MS event. Need testing.
	vector<IsotopicCluster> IsotopicClusterDetector::DetectStructureBasedClusters_(vector<PeakToEventsRelation>& peak_list, vector<IsotopicCluster>& detected_clusters)
	{
		// Creates a set of peaks that need to be filtered.
		unordered_set<size_t> detected_peaks;
		for (IsotopicCluster& cluster : detected_clusters)
		{
			for (Peak* peak : cluster.clustered_peaks)
			{
				detected_peaks.insert(peak->mID);
			}
		}

		// Creates a vector of pointers to peaks that haven't been clustered yet.
		vector<PeakToEventsRelation*> filtered_peak_list;
		filtered_peak_list.reserve(peak_list.size());
		for (PeakToEventsRelation& peak : peak_list)
		{
			if (detected_peaks.find(peak.peak->mID) == detected_peaks.end())
			{
				filtered_peak_list.push_back(&peak);
			}
		}
		filtered_peak_list.shrink_to_fit();

		// Sorts the peaks based on intensity.
		sort(filtered_peak_list.begin(), filtered_peak_list.end(), [](PeakToEventsRelation* a, PeakToEventsRelation* b)
		{
			return a->peak->mHeight > b->peak->mHeight;
		});

		// Loops through all the charge states, trying to determine which cluster configurations are viable.
		std::vector<IsotopicCluster>	clusters;
		std::unordered_set<size_t>		clustered_peak_ids;
		for (PeakToEventsRelation* peak : filtered_peak_list)
		{
			// If the peak has already been clustered or has events, ignore it.
			if (clustered_peak_ids.find(peak->peak->mID) == clustered_peak_ids.end() && peak->events.empty())
			{
				size_t clusters_added = 0;
				for (unsigned char chargestate = 1; chargestate <= m_max_chargestate_; ++chargestate)
				{
					// Acquires the isotopic distribution.
					MIDAs midas(chargestate, 1, 1, 1e-150, m_midas_mode_);
					midas.Initialize_Elemental_Composition(Averagine(m_averagine_table_, peak->peak->mX * chargestate), "", "H", "OH", 1);
					DistributionTable isotopic_distribution(midas.Fine_Grained_Isotopic_Distribution());

					IsotopicCluster cluster(
					{
						peak->peak,
						chargestate,
						CalculateIsotopicClusterPeaks_(*peak, chargestate, isotopic_distribution),
						nullptr,
						nullptr,
						'g'
					});

					if (cluster.clustered_peaks.size() > 1)
					{
						clusters.push_back(cluster);
						++clusters_added;
					}
				}

				// Adds the clustered peak ids to the set.
				for (size_t cluster = clusters.size() - clusters_added; cluster < clusters.size(); ++cluster)
				{
					for (Peak* peak : clusters[cluster].clustered_peaks)
					{
						clustered_peak_ids.insert(peak->mID);
					}
				}
			}
		}

		return clusters;
	}

	vector<PeakToEventsRelation*> IsotopicClusterDetector::GetPeaks_(double mz, double rt, double mz_sigma, double rt_sigma)
	{
		// Calculates the region of interest.
		double rt_min = rt - (rt_sigma * m_rt_sigma_range_);
		double rt_max = rt + (rt_sigma * m_rt_sigma_range_);
		double mz_min = mz - (mz_sigma * m_mz_sigma_range_);
		double mz_max = mz + (mz_sigma * m_mz_sigma_range_);
		
		// Creates the vector.
		vector<PeakToEventsRelation*> peaks;

		// Calculates the position of the minimum rt value peak.
		size_t size = m_rt_ordered_peaks_.size();
		size_t rt_index = 0;

		auto floored_value = m_rt_map_.find(floor(rt_min));
		if (floored_value != m_rt_map_.end())
		{
			rt_index = floored_value->second;
		}
		else
		{
			while (rt_index < size && m_rt_ordered_peaks_[rt_index]->peak->mY <= rt_min)
			{
				++rt_index;
			}
		}

		// Loops through the remaining peaks, in order to detect candidates for the selected region.
		while (rt_index < size && m_rt_ordered_peaks_[rt_index]->peak->mY <= rt_max)
		{
			double peak_mz = m_rt_ordered_peaks_[rt_index]->peak->mX;
			if (peak_mz >= mz_min && peak_mz <= mz_max)
			{
				peaks.push_back(m_rt_ordered_peaks_[rt_index]);
			}
			++rt_index;
		}

		return peaks;
	}
	
	// Matches the isotopic distribution with the peaks within the cluster and returns the shift relative to the cluster.
	int IsotopicClusterDetector::DetermineIsotopicDistributionShift_(PeakToEventsRelation& base_peak, unsigned char chargestate, DistributionTable& isotopic_distribution)
	{
		// Attempts to locate peaks in the ascending direction.
		vector<PeakToEventsRelation*> peaks(GetPeaks_(
			base_peak.peak->mX + (1.0 / chargestate),
			base_peak.peak->mY,
			base_peak.peak->mXSig,
			base_peak.peak->mYSig));

		// If no peaks have been found in the ascending direction, attempt finding peaks in the descending direction.
		if (peaks.empty())
		{
			vector<PeakToEventsRelation*> descending_peaks(GetPeaks_(
				base_peak.peak->mX - (1.0 / chargestate),
				base_peak.peak->mY,
				base_peak.peak->mXSig,
				base_peak.peak->mYSig));
		}

		// Attempts to determine the offset between the peak list and isotopic distribution.
		char isotope_shift = -1;
		if (!peaks.empty())
		{
			// Calculates which isotope distribution element is the event peak.
			for (size_t iso = 0; iso < isotopic_distribution.size() - 1; ++iso)
			{
				double expected_intensity = base_peak.peak->mHeight * (isotopic_distribution[iso + 1].prob / isotopic_distribution[iso].prob);

				for (PeakToEventsRelation* peak : peaks)
				{
					if (peak->peak->mHeight >= expected_intensity * (1 - m_error_tolerance_) && peak->peak->mHeight <= expected_intensity * (1 + m_error_tolerance_))
					{
						isotope_shift = iso;
						break;
					}
				}

				if (isotope_shift != -1)
				{
					break;
				}
			}
		}

		return isotope_shift;
	}

	Peak* IsotopicClusterDetector::SelectIdealPeak_(const Peak* reference_peak, const vector<PeakToEventsRelation*> potential_peaks, const double expected_mz, const double expected_intensity)
	{
		// Loops through the available peaks, determining the best match.
		Peak* best_match = nullptr;
		double best_difference = 10;
		for (PeakToEventsRelation* potential_peak : potential_peaks)
		{
			Peak* peak = potential_peak->peak;
			// If the peak is within the expected range.
			if (peak->mHeight >= expected_intensity * (1 - m_error_tolerance_) &&
				peak->mHeight <= expected_intensity * (1 + m_error_tolerance_))
			{
				double current_difference = sqrt(pow((1 / expected_intensity)	* (expected_intensity - peak->mHeight), 2)) +
											sqrt(pow((1 / expected_mz)			* (expected_mz - peak->mX), 2)) +
											sqrt(pow((1 / reference_peak->mY)	* (reference_peak->mY - peak->mY), 2));
				// double intensity_difference = sqrt(pow((1 / expected_intensity) * (expected_intensity - peak->mHeight), 2));
				// double mz_difference = sqrt(pow((1 / mz_estimate) * (mz_estimate - peak->mX), 2));
				// double rt_difference = sqrt(pow((1 / event_peak->mY) * (event_peak->mY - peak->mY), 2));

				if (current_difference < best_difference && peak->mID != reference_peak->mID)
				{
					best_match		= peak;
					best_difference = current_difference;
				}
			}
		}

		return best_match;
	}
}
