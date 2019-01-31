// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.


#ifndef PeakWarpDB_h_
#define PeakWarpDB_h_

#include <string>
#include <vector>
#include <map>

class DoubleMatrix;
class Mesh;
class Peak;

using namespace std;

class PeakWarpDBEx {
public:
	PeakWarpDBEx(string msg) {errMessage=msg;}
	string errMessage;
};

class PeakWarpDBEntry {
public:
	PeakWarpDBEntry(Peak& thePeak) {
		peak = thePeak; keep = true;
	}
	Peak peak;
	bool keep;
};

typedef map<double, PeakWarpDBEntry*> MZTree;

//
// Organizes peaks first by RT then by M/Z.
//
class PeakWarpDB {
	friend class PeakWarpDBIterator;

	//stree rtTree;
	map<double, MZTree*> rtTree;
	int sampleIdx;

public:
	PeakWarpDB(int theSampleIdx);
	~PeakWarpDB();

	void addPeak(Peak& thePeak);
	PeakWarpDBEntry* getPeakDBEntry(Peak& thePeak);

	double getPeakMinRT();
	double getPeakMaxRT();
	vector<Peak> getPeaksAtRTBand(double rtCenter, double rtWidth);
};

class PeakWarpDBIterator {
	PeakWarpDB *peakWarpDB;
	//
	map<double, MZTree*>::iterator rtTreePos;
	MZTree *curntList;
	MZTree::iterator mzTreePos;
	PeakWarpDBEntry *curntPeak;
public:
	PeakWarpDBIterator(PeakWarpDB* thePeakWarpDB) { 
		peakWarpDB = thePeakWarpDB; init();
	}

	void init();
	void init(double startRT);
	Peak nextP();
	PeakWarpDBEntry* nextE();
	bool done(); // Check done() status before calling next().
};

#endif 
//PeakWarpDB_h_
