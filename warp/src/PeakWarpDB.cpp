
//#include "External/GawDex/stdafx.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "Mesh/Mesh.hpp"
#include "PeakWarpDB.h"
#include "Mesh/StringTokenizer.h"

PeakWarpDB::PeakWarpDB(int theSampleIdx) {
	sampleIdx = theSampleIdx;
}

PeakWarpDB::~PeakWarpDB() {
	map<double, MZTree*>::iterator rtTreePos;
	for(rtTreePos=rtTree.begin(); rtTreePos!=rtTree.end(); ++rtTreePos) {
		MZTree* mzTree = rtTreePos->second;
		MZTree::iterator mzTreePos;
		for(mzTreePos=mzTree->begin(); mzTreePos!=mzTree->end(); ++mzTreePos) {
			PeakWarpDBEntry* peakDBEntry = mzTreePos->second;
			delete peakDBEntry;
		}
		delete mzTree;
	}
}

void PeakWarpDB::addPeak(Peak& thePeak) {
	map<double, MZTree*>::iterator rtTreePos;
	rtTreePos = rtTree.find(thePeak.mY);
	if (rtTreePos!=rtTree.end()) {
		MZTree* mzTree = rtTreePos->second;
		MZTree::iterator mzTreePos;
		mzTreePos = mzTree->find(thePeak.mX);
		if (mzTreePos==mzTree->end()) {
			PeakWarpDBEntry* peakDBEntry = new PeakWarpDBEntry(thePeak);
			mzTree->insert(make_pair(thePeak.mX, peakDBEntry));
		} else { // Peak already recorded at this position. Duplicate ?
			cout << "PeakWarpDB> Duplicate peak ? Ignoring ..." << endl;
		}
	} else {
		// RT value not seen, add list at RT.
		MZTree* mzTree = new MZTree();
		rtTree.insert(make_pair(thePeak.mY, mzTree));
		//
		PeakWarpDBEntry* peakDBEntry = new PeakWarpDBEntry(thePeak);
		mzTree->insert(make_pair(thePeak.mX, peakDBEntry));
	}
}

PeakWarpDBEntry* PeakWarpDB::getPeakDBEntry(Peak& thePeak) {
	map<double, MZTree*>::iterator rtTreePos;
	rtTreePos = rtTree.find(thePeak.mY);
	if (rtTreePos==rtTree.end()) { // RT value not seen.
		return nullptr;
	} else {
		MZTree* mzTree = rtTreePos->second;
		MZTree::iterator mzTreePos;
		mzTreePos = mzTree->find(thePeak.mX);
		if (mzTreePos==mzTree->end()) { // MZ value not seen.
			return nullptr;
		} else {
			return mzTreePos->second;
		}
	}
}

double PeakWarpDB::getPeakMinRT() {
	map<double, MZTree*>::iterator rtTreePos; rtTreePos = rtTree.begin();
	if (rtTreePos!=rtTree.end())
		return rtTreePos->first;
	else throw new PeakWarpDBEx("Empty Peak DB.");
}

double PeakWarpDB::getPeakMaxRT() {
	map<double, MZTree*>::iterator rtTreePos; rtTreePos = rtTree.end(); rtTreePos--;
	if (rtTreePos!=rtTree.end())
		return rtTreePos->first;
	else throw new PeakWarpDBEx("Empty Peak DB.");
}

vector<Peak> PeakWarpDB::getPeaksAtRTBand(double rtCenter, double rtWidth) {
	vector<Peak> thePeaks;

	double startRT = rtCenter-rtWidth;	
	PeakWarpDBIterator peakIterator(this); peakIterator.init(startRT);

	while(!peakIterator.done()) { Peak nextPeakd  = peakIterator.nextP();
					Peak& nextPeak = nextPeakd;

		if (nextPeak.mY<=(rtCenter+rtWidth)) {
			thePeaks.push_back(nextPeak);
		} else 
			break;
	}
	return thePeaks;
}

void PeakWarpDBIterator::init() {
	rtTreePos = peakWarpDB->rtTree.begin();
	if (rtTreePos == peakWarpDB->rtTree.end()) { // Empty DB.
		curntList = nullptr;
		curntPeak = nullptr;
	} else {
		curntList = rtTreePos->second;
		mzTreePos = curntList->begin();
		curntPeak = mzTreePos->second;
	}
}

void PeakWarpDBIterator::init(double startRT) {
	rtTreePos = peakWarpDB->rtTree.lower_bound(startRT);
	// If not exactly the required RT, go to next higher value.
	if (rtTreePos != peakWarpDB->rtTree.end() && rtTreePos->first<startRT) {
		rtTreePos++;
	}

	if (rtTreePos!=peakWarpDB->rtTree.end()) {
		curntList = rtTreePos->second;
		mzTreePos = curntList->begin();
		curntPeak = mzTreePos->second;
	} else { // Empty DB.
		curntList = nullptr;
		curntPeak = nullptr;
	}
}

bool PeakWarpDBIterator::done() {
	return (curntPeak==nullptr);
}

Peak PeakWarpDBIterator::nextP() {
	return (nextE()->peak);
}

PeakWarpDBEntry* PeakWarpDBIterator::nextE() {
	PeakWarpDBEntry *result = curntPeak;
	if (curntList != nullptr) { 
		mzTreePos++;
		if (mzTreePos == curntList->end()) { // Reached end of MZ list, get next list.
			rtTreePos++;
			if (rtTreePos != peakWarpDB->rtTree.end()) {
				curntList = rtTreePos->second;
				mzTreePos = curntList->begin();
				curntPeak = mzTreePos->second;
			} else {
				curntList = nullptr;
				curntPeak = nullptr;
			}
		} else {
			curntPeak = mzTreePos->second;
		}
	}
	return (result);
}

