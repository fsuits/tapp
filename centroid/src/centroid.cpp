#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <unordered_map>

#include "LCMSFile/LCMSFile.h"
#include "Mesh/Mesh.hpp"

#include "Filetypes/MzXML/MzXML_Parser.h"
#include "Filetypes/Mzid/Mzid_Parser.h"
#include "Filetypes/FileRelations/FileRelations.h"
#include "Filetypes/TAPP/IPL.h"
#include "Filetypes/TAPP/TAPP_Output.h"

#ifndef LOAD_MIDAS
using namespace std;
using namespace TAPP;

#else
#include "MassSpectrometry/Isotope/IsotopicClusterDetector.h" // loads midas.h

using namespace TAPP;
using namespace TAPP::MassSpectrometry;

// TODO: Should probably integrate the additional information in the cluster generation.
void WriteClusters(std::string& filepath, Mesh& mesh, std::vector<Isotope::IsotopicCluster>& clusters)
{
	std::unordered_map<size_t, size_t> new_peak_ids;
	new_peak_ids.reserve(mesh.peaks.size());

	for (size_t p = 0; p < mesh.peaks.size(); ++p)
	{
		new_peak_ids.insert({ mesh.peaks[p].mID, p });
	}

	// Generates the IPL records.
	size_t cluster_id = 0;
	size_t current_cluster_group = 0;
	std::unordered_map<size_t, size_t> peak_to_clustergroup;
	std::vector<Filetypes::TAPP::IPL> ipl_records;
	for (Isotope::IsotopicCluster& cluster : clusters)
	{
		size_t overlap_cluster_group = 0;
		for (Peak* peak : cluster.clustered_peaks)
		{
			auto peak_to_cluster_iterator = peak_to_clustergroup.find(peak->mID);
			if (peak_to_cluster_iterator != peak_to_clustergroup.end())
			{
				overlap_cluster_group = peak_to_cluster_iterator->second;
				break;
			}
		}

		// If there's overlap, use overlap ID, otherwise use an auto increment id.
		if (overlap_cluster_group == 0)
		{
			++current_cluster_group;
			overlap_cluster_group = current_cluster_group;
		}

		// Annotates all the peaks with the id.
		for (Peak* peak : cluster.clustered_peaks)
		{
			peak_to_clustergroup.insert({ peak->mID, overlap_cluster_group });
		}

		// Constructs the initial IPL object.
		ipl_records.push_back(
		{
			cluster_id,
			overlap_cluster_group,
			new_peak_ids.find(cluster.event_peak->mID)->second,
			-1,
			"-1",
			cluster.identification_method,
			cluster.chargestate
		});

		if (cluster.event)
		{
			ipl_records.back().event_id = cluster.event->spectral_scan->scan_id;
		}

		if (cluster.identification)
		{
			ipl_records.back().identification_id = cluster.identification->id;
		}

		for (Peak* peak : cluster.clustered_peaks)
		{
			ipl_records.back().cluster_peaks.push_back(new_peak_ids.find(peak->mID)->second);
		}

		// Increments the cluster_id.
		++cluster_id;
	}

	

	// Writes the cluster information into an IPL file.
	std::ofstream writer(filepath);
	Filetypes::TAPP::SetGlobalPrecision(writer);
	Filetypes::TAPP::WriteIPL(writer, ipl_records, { '\n', ' ', '\"' }, true);
	writer.close();
}

// Writes various statistics regarding alignment.
void WriteStatistics(std::string filepath,
	const Mesh& mesh,
	const Filetypes::MzXML::MzXML_File& mzxml_file,
	const Filetypes::Mzid::Mzid_File& mzid_file,
	const std::vector<Isotope::IsotopicCluster>& clusters,
	const std::vector<Filetypes::FileRelations::PeakToEventsRelation>& peak_to_event,
	const std::vector<Filetypes::FileRelations::ScanToIdentificationsRelation>& scan_to_identification)
{
	size_t peak_event_links				= 0;
	size_t scan_identification_links	= 0;

	for (auto link : scan_to_identification)
	{
		if (link.identification)
		{
			++scan_identification_links;
		}
	}

	for (auto link : peak_to_event)
	{
		if (!link.events.empty())
		{
			++peak_event_links;
		}
	}

	std::fstream writer;
	writer.open(filepath);

	writer << "Peaks: " << std::to_string(mesh.peaks.size()) << "\n";
	writer << "Events: " << std::to_string(mzxml_file.events.size()) << "\n";
	writer << "Identifications: " << std::to_string(mzid_file.identification_results.size()) << "\n";
	writer << "Clusters: " << std::to_string(clusters.size()) << "\n";
	writer << "Event To Peak Links: " << std::to_string(peak_event_links) << "\n";
	writer << "Event To Identification Links: " << std::to_string(scan_identification_links);

	writer.close();
}

std::map<size_t, std::vector<MassSpectrometry::Isotope::IsotopicCluster>> AnnotateClusters(std::vector<MassSpectrometry::Isotope::IsotopicCluster>& clusters)
{
	std::map<size_t, std::vector<MassSpectrometry::Isotope::IsotopicCluster>> annotated_clusters;

	size_t current_cluster_id = 0;
	std::unordered_map<size_t, size_t> peak_to_clustergroup;
	for (MassSpectrometry::Isotope::IsotopicCluster& cluster : clusters)
	{
		bool overlap = false;
		size_t overlap_cluster_id = 0;
		for (Peak* peak : cluster.clustered_peaks)
		{
			auto peak_to_cluster_iterator = peak_to_clustergroup.find(peak->mID);
			if (peak_to_cluster_iterator != peak_to_clustergroup.end())
			{
				overlap = true;
				overlap_cluster_id = peak_to_cluster_iterator->second;
				break;
			}
		}

		if (overlap)
		{
			for (Peak* peak : cluster.clustered_peaks)
			{
				peak_to_clustergroup.insert({ peak->mID, overlap_cluster_id });
			}

			auto cluster_group_iterator = annotated_clusters.insert({ overlap_cluster_id, std::vector<MassSpectrometry::Isotope::IsotopicCluster>() });
			cluster_group_iterator.first->second.push_back(cluster);
		}
		else
		{
			for (Peak* peak : cluster.clustered_peaks)
			{
				peak_to_clustergroup.insert({ peak->mID, current_cluster_id });
			}

			auto cluster_group_iterator = annotated_clusters.insert({ current_cluster_id, std::vector<MassSpectrometry::Isotope::IsotopicCluster>() });
			cluster_group_iterator.first->second.push_back(cluster);

			++current_cluster_id;
		}
	}

	return annotated_clusters;
}
#endif

int main(int argc, char* argv[])
{
	const std::string MESSAGE_PREFIX("Centroid: ");

    if (argc < 2)
	{
		//cout << MESSAGE_PREFIX << argv[0] << " file.mesh <-npeaks N> <-hdr Header.hdr> < file.pks" << endl;
		cout << MESSAGE_PREFIX << argv[0] << " file.mesh <-npeaks N> <-hdr Header.hdr> -output file.pks" << endl <<
			"-mzid The mzid file that will be used for the detection of isotopic clusters." << endl <<
			"-mzxml The mzxml file that will be used for the detection of isotopic clusters." << endl <<
			"-error The error tolerance from 0.0 to 1.0 in regards to the intensity of an isotopic peak." << endl <<
			"-mz_sigma The m/z tolerance when selecting isotopic peaks. M/z area is (x - sigma * tolerance) to (x + sigma * tolerance)" << endl <<
			"-rt_sigma The RT tolerance when selecting isotopic peaks. RT area is (x - sigma * tolerance) to (x + sigma * tolerance)" << endl <<
			"-precursor_window The m/z tolerance in daltons that'll be used when a peak cannot be found at the exact location within the HIT list." << endl <<
			"-structure Extract clusters that are based entirely on their structure within the MzXML file." << endl;
        exit(0);
    }

	FSLittleEndian::get();

    char *fname = argv[1];
    char HeadName[1000];

    int narg = 2;

    int npeaks = 100000;

    int havenpks = 0;
    int haveheadname = 0;

	std::string		mzxml_filepath, mzid_filepath, output_name;
	bool			detect_structure_based_clusters = false;
	unsigned int	isotopic_clustering_mz_sigma_tolerance = 1;
	unsigned int	isotopic_clustering_rt_sigma_tolerance = 1;
	double			isotopic_clustering_error_tolerance = 0.1;
	double			precursor_mz_selection_window = 0.0;

    while (narg < argc) {
        if (!strcmp(argv[narg], "-npeaks")) {
            narg++;
            if (narg < argc) {
                npeaks = atoi(argv[narg]);
                havenpks = 1;
            } else {
                cerr << "-npeaks is missing a value on command line.  Terminating" << endl;
                exit(-1);
            }
        } else if (!strcmp(argv[narg], "-hdr")) {
            narg++;
            if (narg < argc) {
                strcpy(HeadName, argv[narg]);
                haveheadname = 1;
            } else {
                cerr << "-hdr is missing a value on command line.  Terminating" << endl;
                exit(-1);
            }
        } 
		else if (std::string(argv[narg]) == "-mzxml")
		{
			++narg;
			if (narg < argc)
			{
				mzxml_filepath = argv[narg];
			}
		}
		else if (std::string(argv[narg]) == "-mzid")
		{
			++narg;
			if (narg < argc)
			{
				mzid_filepath = argv[narg];
			}
		}
		else if (std::string(argv[narg]) == "-output")
		{
			++narg;
			if (narg < argc)
			{
				output_name = argv[narg];
			}
		}
		else if (std::string(argv[narg]) == "-error")
		{
			++narg;
			if (narg < argc)
			{
				isotopic_clustering_error_tolerance = stof(argv[narg]);
			}
		}
		else if (std::string(argv[narg]) == "-mz_sigma")
		{
			++narg;
			if (narg < argc)
			{
				isotopic_clustering_mz_sigma_tolerance = atoi(argv[narg]);
			}
		}
		else if (std::string(argv[narg]) == "-rt_sigma")
		{
			++narg;
			if (narg < argc)
			{
				isotopic_clustering_rt_sigma_tolerance = atoi(argv[narg]);
			}
		}
		else if (std::string(argv[narg]) == "-precursor_window")
		{
			++narg;
			if (narg < argc)
			{
				precursor_mz_selection_window = std::stod(argv[narg]);
			}
		}
		else if (std::string(argv[narg]) == "-structure")
		{
			detect_structure_based_clusters = true;
		}
		else {
            cerr << "Argument " << argv[narg] << " not recognized. Terminating" << endl;
            exit(-1);
        }
        narg++;
    }

	// Checks input variables. Only output_name for now.
	if (output_name.empty())
	{
		std::cout << "Centroid: -output is missing." << std::endl;
		exit(-1);
	}

    LCMSFile mLCMS;

    mLCMS.setAttributes("centroid");

    if (!haveheadname) {
        strcpy(HeadName, fname);

        int n = strlen(HeadName);
        while (n>0 && HeadName[n] != '.')
            n--;
        if (HeadName[n] == '.') {
            strcpy(&HeadName[n], ".hdr");
        }
    }
    if (mLCMS.loadAttributes(HeadName)) {
        cerr << "Header file " << HeadName << " not found.  Trying Default.hdr" << endl;
        strcpy(HeadName, "Default.hdr");
        if (mLCMS.loadAttributes(HeadName)) {
            cerr << "Default.hdr not found - using standard defaults." << endl;
        }
    }

    // mLCMS.loadAttributes("Default.hdr");
    mLCMS.setConversionAttributes();

    char cmdstr[2048];
    strcpy(cmdstr, argv[0]);
    for (int i=1; i<argc; i++) {
        strcat(cmdstr, " ");
        strcat(cmdstr, argv[i]);
    }
    mLCMS.mAttributes.addUnique("ConversionCommandLineCentroid", cmdstr);


    if (mLCMS.mMesh.mConversion.mNPeaksToFind > 0 && !havenpks)  // allow command line to override hdr value
        npeaks = mLCMS.mMesh.mConversion.mNPeaksToFind;

	mLCMS.mMesh.loadFromFile(argv[1]);

    mLCMS.mMesh.FindPeaks(npeaks, mLCMS.mMesh.mConversion.mPeakThreshold, mLCMS.mMesh.mConversion.mPeakHeightMin);

	std::ofstream writer(output_name + ".pks");
	if (writer.is_open())
	{
		mLCMS.mMesh.DumpPeaks(writer);
		writer.close();
	}

	if (detect_structure_based_clusters || !mzxml_filepath.empty())
	{
		// Initializes the empty variables. The actual contents of these will define the operations the program will use.
		Filetypes::MzXML::MzXML_File	mzxml_file;
		Filetypes::Mzid::Mzid_File		mzid_file;

		if (!mzxml_filepath.empty())
		{
			std::cout << MESSAGE_PREFIX << "Parsing MzXML file." << std::endl;
			Filetypes::MzXML::MzXML_Parser mz_parser(50240000);
			mzxml_file = mz_parser.ParseFile(mzxml_filepath);

			if (!mzid_filepath.empty())
			{
				std::cout << MESSAGE_PREFIX << "Parsing Mzid file." << std::endl;
				Filetypes::Mzid::Mzid_Parser parser(true, 50240000);
				mzid_file = parser.ParseFile(mzid_filepath);
			}
		}

		std::cout << MESSAGE_PREFIX << "Creating relational tables." << std::endl;
		std::vector<Filetypes::FileRelations::PeakToEventsRelation>	peak_to_event					= Filetypes::FileRelations::CreatePeakToEventRelation(precursor_mz_selection_window, mLCMS.mMesh, mzxml_file);
		std::vector<Filetypes::FileRelations::ScanToIdentificationsRelation> scan_to_identification = Filetypes::FileRelations::CreateScanToIdentificationRelation(mzxml_file, mzid_file);

#ifdef LOAD_MIDAS
		std::cout << MESSAGE_PREFIX << "Calculating isotopic clusters." << std::endl;
		MassSpectrometry::Isotope::IsotopicClusterDetector detector(isotopic_clustering_error_tolerance, isotopic_clustering_mz_sigma_tolerance, isotopic_clustering_rt_sigma_tolerance);
		std::vector<MassSpectrometry::Isotope::IsotopicCluster> detected_clusters = detector.DetectClusters(peak_to_event, scan_to_identification, detect_structure_based_clusters);

		// Outputs the cluster information.
		std::string output_filename(output_name + ".ipl");
		WriteClusters(output_filename, mLCMS.mMesh, detected_clusters);

		// Outputs statistical information regarding alignment of the files.
		WriteStatistics(output_name + ".stat", mLCMS.mMesh, mzxml_file, mzid_file, detected_clusters, peak_to_event, scan_to_identification);
#endif
	}

    return 0;
}
