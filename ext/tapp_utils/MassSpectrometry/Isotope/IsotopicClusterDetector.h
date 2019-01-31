// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#pragma once

#include <map>
#include <vector>

#include "Filetypes/FileRelations/FileRelations.h"

#include "Mesh/Mesh.hpp"

#ifdef LOAD_MIDAS
#include "External/Midas/MIDAs.h"
#endif

using namespace TAPP::Filetypes::FileRelations;

// TODO: DetectEventBasedClusters_ assumes that there's only one identification item per result.

namespace TAPP::MassSpectrometry::Isotope
{
	struct IsotopicCluster
	{
		Peak*							event_peak; // Initial peak used to calculate the cluster.
		unsigned char					chargestate;
		std::vector<Peak*>				clustered_peaks;
		Event*					event;
		SpectrumIdentificationResult*	identification;
		char							identification_method;
	};

	class IsotopicClusterDetector
	{
		public:
			IsotopicClusterDetector(double error_tolerance = 0.1, double mz_sigma_range = 1, double rt_sigma_range = 1, unsigned char max_chargestate = 0, unsigned char midas_mode = 1);

			std::vector<IsotopicCluster> DetectClusters(std::vector<PeakToEventsRelation>& peak_list,
			std::vector<TAPP::Filetypes::FileRelations::ScanToIdentificationsRelation>& scan_list, bool detect_structure = false);

		private:
			typedef std::vector<Isotopic_Distribution> DistributionTable; // std::vector<Isotopic_Distribution>

			double			m_error_tolerance_;
			unsigned char	m_midas_mode_;
			double			m_mz_sigma_range_;
			double			m_rt_sigma_range_;
			unsigned char	m_max_chargestate_;

			// Variables used during cluster calculation. These are cleaned after each cluster analysis.
			std::vector<std::pair<char, double>>								m_averagine_table_; // Table is only created once per run for efficiency.
			std::vector<TAPP::Filetypes::FileRelations::PeakToEventsRelation*>	m_rt_ordered_peaks_;
			std::unordered_map<size_t, size_t>									m_rt_map_;

			std::vector<Peak*> CalculateIsotopicClusterPeaks_(TAPP::Filetypes::FileRelations::PeakToEventsRelation& base_peak, unsigned char chargestate,
				DistributionTable& distribution_table);

			void CreateOrderedList_(std::vector<TAPP::Filetypes::FileRelations::PeakToEventsRelation>& peak_list);

			std::vector<IsotopicCluster> DetectEventBasedClusters_(std::vector<TAPP::Filetypes::FileRelations::PeakToEventsRelation>& peak_list,
				std::vector<ScanToIdentificationsRelation>& scan_list);

			std::vector<IsotopicCluster> DetectStructureBasedClusters_(std::vector<PeakToEventsRelation>& peak_list,
				std::vector<IsotopicCluster>& detected_clusters);

			int DetermineIsotopicDistributionShift_(PeakToEventsRelation& peak, unsigned char chargestate, DistributionTable& isotopic_distribution);

			std::vector<PeakToEventsRelation*> GetPeaks_(double mz, double rt, double mz_sigma, double rt_sigma);
			
			Peak* SelectIdealPeak_(const Peak* reference_peak, const std::vector<PeakToEventsRelation*> potential_peaks, const double expected_mz, const double expected_intensity);
	};
}

