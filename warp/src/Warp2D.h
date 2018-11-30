#pragma once

#ifndef Warp2D_h_
#define Warp2D_h_
#include <map>
#include <string>
#include <iostream>
#include <vector>


class Warp2D {

	double peakOverlap(Peak& peak1,Peak& peak2);
	double similarity2D(double refTimeStart, double refTimeSeg, std::vector<Peak>& refPeaks,
						double smpTimeStart, double smpTimeSeg, std::vector<Peak>& smpPeaks);
	int cow_2D(std::vector<Peak>& refPeaks, std::vector<Peak>& smpPeaks, int nT, int mT, int t);

	int binSearch(std::vector<double>& theVector, double target, int pIdx1, int rIdx1);
	std::vector<Peak> filterPeaks(std::vector<Peak> thePeaks, int nSegments);

	int windowSize_m;
	int slack_t;
	int nTimePoints;
	int maxPeaksSegmt;
	double tRangeExpandFactor;
	bool debug;
	std::vector<double> origTime; std::vector<double> warpedTime; std::vector<double> segmtCorr;
	double totalCorr; double unWarpedCorr;
	int maxNPeaksPSegmt;

public:

	Warp2D(int maxPeaksSegmt=25, int nTimePoints=1000, int winSize_m=50, int slack_t=10);
	~Warp2D();

	void setWindowSize(int winSize_m) { windowSize_m = winSize_m; }
	void setSlack(int theSlack_t) { slack_t = theSlack_t; }
	void setNTimePoints(int nTPts) { nTimePoints = nTPts; }
	void setMaxPeaksPerSegmt(int maxPPSeg) { maxPeaksSegmt = maxPPSeg; }
	void setTimeRangeExpandFactor(double tREFac) { tRangeExpandFactor = tREFac; }

	//
	// After object set up, must call computeWarp() to compute optimal time warp,
	void computeWarp(std::vector<Peak>& referencePeaks, std::vector<Peak>& samplePeaks);

	double similarity2D(std::vector<Peak>& refPeaks,std::vector<Peak>& smpPeaks);
	double getTotalCorr() { return totalCorr; }
	double getTotalCorrUnWarped() { return unWarpedCorr; }

	// Warp any set of peaks to the computed time warp.
	std::vector<Peak> warpPeaks(std::vector<Peak>& thePeaks);

	void saveTimeMap2File(std::string& filename);

	double sampleToRefTimeInterpolate(double sampleTQ);

        void removeShift(float s) {
            for (int i=0; i<origTime.size(); i++)
                origTime[i] -= s;
        }

        void findSimilarity(std::vector<Peak>& ref, std::vector<Peak>& smp, char *prefix = "") {
            int nseg = nTimePoints/windowSize_m;
            std::vector<Peak> r = filterPeaks(ref, nseg);
            std::vector<Peak> s = filterPeaks(smp, nseg);
            double refover = similarity2D(r, r);
            double sampover = similarity2D(s, s);
            double overlap = similarity2D(r, s);
            std::cout << prefix << " similarity: ref, samp, overlap, GeometricRatio, MeanRatio: " << refover << " " << sampover << " " << overlap << " "
                 << overlap/sqrt(refover*sampover) << " " << 2*overlap/(refover+sampover) << std::endl;

         }

	/*
	*	insted of printing on stdout, insert in value hashstd::map
	*/
	
        void findSimilarity(std::vector<Peak>& ref, std::vector<Peak>& smp, std::string prefix, std::map<std::string,std::string>& soutMap) {	    
            int nseg = nTimePoints/windowSize_m;
            std::vector<Peak> r = filterPeaks(ref, nseg);
            std::vector<Peak> s = filterPeaks(smp, nseg);
            double refover = similarity2D(r, r);
            double sampover = similarity2D(s, s);
            double overlap = similarity2D(r, s);
            std::stringstream ss;
            std::map<std::string, std::string>::iterator iter;
            if(prefix.compare("UnWarped") == 0){            	
            	ss << refover;            	
            	soutMap["ref_unWarped_peaks_vol"] =  ss.str();
            	ss.str("");
				ss.clear();
				ss << sampover;
				soutMap["sam_unWarped_peaks_vol"] =  ss.str();
            	ss.str("");
				ss.clear();
				ss << overlap;
				soutMap["overlap_unWarped_ref_sam_peaks_vol"] =  ss.str();
            	ss.str("");
				ss.clear();
				ss << (overlap/sqrt(refover*sampover));
				soutMap["geometricRatio_unWarped"] =  ss.str();
            	ss.str("");
				ss.clear();
				ss << (2*overlap/(refover+sampover));
				soutMap["meanRatio_unWarped"] =  ss.str();            	            	
            	                	      	
            }else if(prefix.compare("Warped")==0){            	
            	ss << refover;            	
            	soutMap["ref_Warped_peaks_vol"] =  ss.str();
            	ss.str("");
				ss.clear();
				ss << sampover;
				soutMap["sam_Warped_peaks_vol"] =  ss.str();
            	ss.str("");
				ss.clear();
				ss << overlap;
				soutMap["overlap_Warped_ref_sam_peaks_vol"] =  ss.str();
            	ss.str("");
				ss.clear();
				ss << (overlap/sqrt(refover*sampover));
				soutMap["geometricRatio_Warped"] =  ss.str();
            	ss.str("");
				ss.clear();
				ss << (2*overlap/(refover+sampover));
				soutMap["meanRatio_Warped"] =  ss.str();
				            	          	
            }
            /*
            sout << prefix << " similarity: ref, samp, overlap, GeometricRatio, MeanRatio: " << refover << " " << sampover << " " << overlap << " "
                 << overlap/sqrt(refover*sampover) << " " << 2*overlap/(refover+sampover) << endl;
            */
         }

};

#endif // Warp2D_h_

