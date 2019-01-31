// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.


#ifndef PeakMatchDB_h_
#define PeakMatchDB_h_


#include <string>
#include <vector>
#include <map>

class DoubleMatrix;
class Mesh;
class Peak;

using namespace std;

class PeakMatchDBEx {
public:
	PeakMatchDBEx(string msg) {errMessage=msg;}
	string errMessage;
};

class PeakDBEntry {
public:
	PeakDBEntry(Peak& thePeak) {
		peak = thePeak; keep = true;
	}
	void recordMatch(Peak& targetPeak, int targetSmpIdx) {
		matchedPeaksMap.insert(make_pair(targetSmpIdx, targetPeak));
	}
	bool isMatchedTo(int targetSmpIdx) {
		map<int, Peak>::iterator pos = matchedPeaksMap.find(targetSmpIdx);
		if (pos!=matchedPeaksMap.end()) {
			return true;
		} else return false;
	}
	int nMatches() { return (int)matchedPeaksMap.size(); }

	Peak peak; // Full peak info, see Peak class.
	bool keep;
	map<int, Peak> matchedPeaksMap;
};

typedef map<double, PeakDBEntry*> RTTree;

//
// Organizes peaks first by M/Z then by RT.
//
class PeakMatchDB {
	friend class PeakMatchDBIterator;

	//stree mzTree;
	map<double, RTTree*> mzTree;
	int sampleIdx;
	double mzBandHalfWidth_; // Half width of the M/Z band to consider around each peak.
	double maximumMatchRTDiff_; // The maximum amount of RT two matched peaks can differ by.

public:
	PeakMatchDB(int theSampleIdx, double mzBandHalfWidth =0.5, double maximumMatchRTDiff =5);
	~PeakMatchDB();

	void addPeak(Peak& thePeak);
	PeakDBEntry* getPeakDBEntry(Peak& thePeak);

	vector<Peak> getPeaksAtMZBand(float mzCenter, float mzWidth);

	void recordMatch(Peak& sourcePeak, Peak& targetPeak, int targetSmpIdx);

};

class PeakMatchDBIterator {
	PeakMatchDB *peakMatchDB;
	//
	map<double, RTTree*>::iterator mzTreePos; RTTree *curntList;
	RTTree::iterator rtTreePos; PeakDBEntry *curntPeak;
public:
	PeakMatchDBIterator(PeakMatchDB* thePeakMatchDB) { 
		peakMatchDB = thePeakMatchDB; init();
	}

	void init();
	void init(double startMZ);
	Peak nextP();
	PeakDBEntry* nextE();
	bool done(); // Check done() status before calling next().
};

class DynProgPeakMatching {

	double mzBandHalfWidth_; // Half width of the M/Z band to consider around each peak.

	double maximumMatchRTDiff_; // The maximum amount of RT two matched peaks can differ by.

	int minimumMatchCountTh0_; // The minimum number of matches for a peak to be added to the Master list.

	float peakHTh0; // Peak height and volume thresholds for peak to be considered for matching.
	float peakVTh0; // Peaks below either thresholds are ignored.

public:
	DynProgPeakMatching(double mzBandHalfWidth = 0.5, double maximumMatchRTDiff = 5, 
		int minimumMatchCountTh0 = 6) {
		mzBandHalfWidth_ = mzBandHalfWidth;
		maximumMatchRTDiff_ = maximumMatchRTDiff;
		minimumMatchCountTh0_ = minimumMatchCountTh0;
		peakHTh0 = 0; // Default, accept all peaks.
		peakVTh0 = 0;
	}

	void setPeakHeightTh0(float pHTh0) { peakHTh0 = pHTh0; }
	void setPeakVolumeTh0(float pVTh0) { peakVTh0 = pVTh0; }

};

#endif 
