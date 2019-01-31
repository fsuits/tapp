// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.


#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "Mesh/Mesh.hpp"
#include "Warp2D.h"

using namespace std;

// warp2d refname sampname outname
int main(int argc, char **argv) {

	int WinSize = 50;
	int Slack = 10;
	int MaxPeaksSegment = 50;
	int NTimePoints = 2000;
	string ref_file_name;
	string sam_file_name;
	string output_file_name;
	char wpksname[1024];
	char tmapname[1024];
	char qualityFname[1024];
        float SampShift = 0;
        bool OverlapOnly = false;
        bool HaveTMap = false;

	// output outstem.wpks and outstem.tmap
	if (argc < 3) {
	    cout << "Warp2D <-mzwidth 0.2> <-rtwidth 0.2> <-maxmzwidth 0.5> <-maxrtwidth 0.5> refpeaks samppeaks <outstem> <WinSize=50> <Slack=10> <MaxPeaksSgmt=50> <NTimePoints=2000> <sampshift=0>" << endl;
            cout << endl;
            cout << "If no outstem provided, only output overlap of the two peak lists" << endl;
            cout << "If <outstem> ends in .tmap, use it as a tmap file and just output overlap info before and after warping" << endl;
            cout << "Input peak files must have 3, 4, 5, 6, or more columns, and must be space-delimited.  First line is skipped as header" << endl;
            cout << "Options are [MZ RT H] [N MZ RT H] [MZ RT H MZSIGMA RTSIGMA] [N MZ RT H MZSIGMA RTSIGMA]" << endl;
            cout << "All warping is from the sample to the reference, as captured in the .tmap file" << endl;
            cout << endl;
            cout << "If the file kill.txt is found in the local directory, it is used to mask off mass regions for alignment" << endl;
            cout << "kill.txt simply lists mz values for contamination, and any peak within +/- 0.5 of those masses will be excluded from the overlap signal" << endl;
            cout << endl;
            cout << "maxwidths provide culling of peaks that appear unusually large in either dimension.  They are not used in warping." << endl;
            cout << endl;
            cout << "Sample invocations:" << endl;
            cout << "Warp2D ref.pks samp.pks ref_samp -> warps samp to ref and outputs ref_samp.wpks and ref_samp.tmap" << endl;
            cout << "Warp2D ref.pks samp.pks -> just calculates overlap of the two sets of peaks" << endl;
            cout << "Warp2D -mzwidth 0.1 -rtwidth 0.4 ref.pks samp.pks ref_samp 40 8 60 3000 -30 -> warps samp to ref with fully specified parameters, including shift of -30" << endl;
	    exit(0);
	}

        int narg = 1;
        double mzwidth = 0.2;
        double rtwidth = 0.2;
        double maxmzwidth = 0.5;
        double maxrtwidth = 0.5;

        while (*argv[narg] == '-') {
            if (!strcmp(argv[narg], "-mzwidth")) {
                narg++;
                mzwidth = atof(argv[narg]);
                narg++;
            } else if (!strcmp(argv[narg], "-rtwidth")) {
                narg++;
                rtwidth = atof(argv[narg]);
                narg++;
            } else if (!strcmp(argv[narg], "-maxmzwidth")) {
                narg++;
                maxmzwidth = atof(argv[narg]);
                narg++;
            } else if (!strcmp(argv[narg], "-maxrtwidth")) {
                narg++;
                maxrtwidth = atof(argv[narg]);
                narg++;
            }
        }
        
	ref_file_name=argv[narg];
	vector<Peak> pr = Mesh::loadPeaks(argv[narg++], mzwidth, rtwidth);
	sam_file_name=argv[narg];
	vector<Peak> ps = Mesh::loadPeaks(argv[narg++], mzwidth, rtwidth);	
        int nleft = argc-narg;

        if (nleft > 0 && !strstr(argv[narg], ".tmap")) {
        	output_file_name = argv[narg];
	    sprintf(wpksname, "%s.wpks", argv[narg]);
	    sprintf(tmapname, "%s.tmap", argv[narg]);
	    sprintf(qualityFname, "%s.quality", argv[narg]);
            narg++;
        } else {
            OverlapOnly = true;
            if (nleft > 0) {
                HaveTMap = true;
                strcpy(tmapname, argv[narg]);
                narg++;
            }
        }

	if (argc > narg)
	    WinSize = atoi(argv[narg++]);

	if (argc > narg)
	    Slack = atoi(argv[narg++]);

	if (argc > narg)
	    MaxPeaksSegment = atoi(argv[narg++]);

	if (argc > narg)
	    NTimePoints = atoi(argv[narg++]);

        if (argc > narg)
            SampShift = atof(argv[narg++]);

        if (SampShift != 0) {
            for (int i=0; i<ps.size(); i++)
                ps[i].mY += SampShift;
        }

        // need to have peaks in descending order before cull
        sort(pr.begin(), pr.end());
        sort(ps.begin(), ps.end());

	vector<Peak> prc = Mesh::cullPeaks(pr, "kill.txt", 0.5, 2000000, maxmzwidth, maxrtwidth);  // suits changed widths to 0.5 8/1/08
	vector<Peak> psc = Mesh::cullPeaks(ps, "kill.txt", 0.5, 2000000, maxmzwidth, maxrtwidth);

	vector<Peak> prc_copy = prc; // to be safe copy the vector peaks list in
	vector<Peak> psc_copy = psc;
	
	Warp2D w(MaxPeaksSegment, NTimePoints, WinSize, Slack);

        if (OverlapOnly) {
            char *pre = "";
            if (HaveTMap)
                cout << "Calculating overlap using supplied .tmap file" << endl;
            else
                cout << "Calculating overlap directly without .tmap file" << endl;

            if (HaveTMap) {
                cout << "Before Warp: " << endl;
                pre = "UnWarped";
            }
            w.findSimilarity(prc, psc, pre);
            if (HaveTMap) {
                cout << endl << "After Warp:" << endl;
                TimeMap tm;
                tm.load(tmapname);
                for (vector<Peak>::iterator p = psc.begin(); p != psc.end(); p++)
                    p->mY = tm.lookUp(p->mY);
                w.findSimilarity(prc, psc, "Warped");
            }
            exit(0);
        }

	string fname(tmapname);

	w.computeWarp(prc, psc);

	for (int i=0; i<ps.size(); i++) {
	    ps[i].mY = w.sampleToRefTimeInterpolate(ps[i].mY);
	}

    w.removeShift(SampShift);
	w.saveTimeMap2File(fname);
	fstream f(wpksname, std::ios_base::out);
	Mesh::DumpPeaks(ps, f);
	f.close();
	
	/*
	*	call to calculate Quality when warping is done
	*/
	if (!OverlapOnly) {	//Just to be sure its not a call to calculate Quality
		//file header line
		const int HEADERS_SIZE =25;
		string headerLine[HEADERS_SIZE] = {"ref_file_name", "sam_file_name", "output_file_name", "mzwidth", "rtwidth", "maxmzwidth", "maxrtwidth",
        		"winSize", "slack", "maxPeaksSgmt", "sampShift", "ref_NTimePoints", "sam_NTimePoints", "ref_nPeaksTotal", "sam_nPeaksTotal",
        		 "ref_unWarped_peaks_vol", "sam_unWarped_peaks_vol", "overlap_unWarped_ref_sam_peaks_vol", "geometricRatio_unWarped", "meanRatio_unWarped",
        		 "ref_Warped_peaks_vol", "sam_Warped_peaks_vol", "overlap_Warped_ref_sam_peaks_vol", "geometricRatio_Warped", "meanRatio_Warped"
        		 };        
		map <string, string> val_hashMap; // hashMap
		
		for(int i=0; i< HEADERS_SIZE; i++) { // init with default keys i.e defined header line
			val_hashMap[headerLine[i]] = "";			
		}		
		stringstream ss;
		//insert key values
		val_hashMap["ref_file_name"]=(string)ref_file_name;
		val_hashMap["sam_file_name"]=(string)sam_file_name;
		val_hashMap["output_file_name"]=(string)output_file_name;
		ss.str("");
		ss.clear();
		ss << mzwidth;
		val_hashMap["mzwidth"] = ss.str();
		ss.str("");
		ss.clear();
		ss << rtwidth;
        val_hashMap["rtwidth"] = ss.str();
        ss.str("");
		ss.clear();
		ss << maxmzwidth;
        val_hashMap["maxmzwidth"] = ss.str();
        ss.str("");
		ss.clear();
		ss << maxrtwidth;
        val_hashMap["maxrtwidth"] = ss.str();
        ss.str("");
		ss.clear();
		ss << WinSize;
        val_hashMap["winSize"] = ss.str();
        ss.str("");
		ss.clear();
		ss << Slack;
        val_hashMap["slack"] = ss.str();
        ss.str("");
		ss.clear();
		ss << MaxPeaksSegment;
        val_hashMap["maxPeaksSgmt"] = ss.str();
        ss.str("");
		ss.clear();
		ss << SampShift;
        val_hashMap["sampShift"] = ss.str();
        ss.str("");
		ss.clear();		
        ss << NTimePoints;
        val_hashMap["ref_NTimePoints"] =ss.str();
        val_hashMap["sam_NTimePoints"] = ss.str();
        ss.str("");
		ss.clear();		
        ss << prc_copy.size();
        val_hashMap["ref_nPeaksTotal"] =ss.str();
        ss.str("");
		ss.clear();		
        ss << psc_copy.size();                
        val_hashMap["sam_nPeaksTotal"] =ss.str();
	    w.findSimilarity(prc_copy, psc_copy, "UnWarped", val_hashMap);
		TimeMap tm;
        tm.load(tmapname);
        for (vector<Peak>::iterator p = psc_copy.begin(); p != psc_copy.end(); p++)
        p->mY = tm.lookUp(p->mY);
        
        w.findSimilarity(prc_copy, psc_copy, "Warped", val_hashMap);
        
        fstream fout(qualityFname, std::ios_base::out);				        
		for(int i=0; i< HEADERS_SIZE; i++) { //print header to file
			(i+1) < HEADERS_SIZE ? fout << headerLine[i] << ", " : fout << headerLine[i];			
		}
		fout << endl;		
		for(int i=0; i< HEADERS_SIZE; i++) { //print value to file in same order as header 
			(i+1) < HEADERS_SIZE ? fout << val_hashMap[headerLine[i]] << ", " : fout << val_hashMap[headerLine[i]];			
		}
		fout << endl;		
        fout.close();
	}

	return 0;
}
