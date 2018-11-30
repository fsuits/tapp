#include <iostream>
#include <fstream>
#include <sstream>

#include "Mesh/Mesh.hpp"
#include "PeakMatchDB.h"
#include "Mesh/StringTokenizer.h"

PeakMatchDB::PeakMatchDB(int theSampleIdx, double mzBandHalfWidth, double maximumMatchRTDiff) {
	sampleIdx = theSampleIdx;
	mzBandHalfWidth_ = mzBandHalfWidth;
	maximumMatchRTDiff_ = maximumMatchRTDiff;
}

PeakMatchDB::~PeakMatchDB() {
	map<double, RTTree*>::iterator mzTreePos;
	for(mzTreePos=mzTree.begin(); mzTreePos!=mzTree.end(); ++mzTreePos) {
		RTTree* rtTree = mzTreePos->second;
		RTTree::iterator rtTreePos;
		for(rtTreePos=rtTree->begin(); rtTreePos!=rtTree->end(); ++rtTreePos) {
			PeakDBEntry* peakDBEntry = rtTreePos->second;
			delete peakDBEntry;
		}
		delete rtTree;
	}
}

void PeakMatchDB::addPeak(Peak& thePeak) {
	map<double, RTTree*>::iterator mzTreePos;
	mzTreePos = mzTree.find(thePeak.mX);
	if (mzTreePos==mzTree.end()) {
		// M/Z value not seen, add list at M/Z.
		RTTree *rtTree = new RTTree();
		mzTree.insert(make_pair(thePeak.mX, rtTree));
		//
		PeakDBEntry* peakDBEntry = new PeakDBEntry(thePeak);
		rtTree->insert(make_pair(thePeak.mY, peakDBEntry));
	} else {
		RTTree *rtTree = mzTreePos->second;
		RTTree::iterator rtTreePos = rtTree->find(thePeak.mY);
		if (rtTreePos==rtTree->end()) {
			PeakDBEntry* peakDBEntry = new PeakDBEntry(thePeak);
			rtTree->insert(make_pair(thePeak.mY, peakDBEntry));
		} else { // Peak already recorded at this position. Duplicate ?
			cout << "PeakMatchDB> Duplicate peak ? Ignoring ..." << endl;
		}
	}
}

void PeakMatchDBIterator::init() {
	mzTreePos = peakMatchDB->mzTree.begin();
	if (mzTreePos==peakMatchDB->mzTree.end()) { // Empty DB.
		curntList = nullptr;
		curntPeak = nullptr;
	} else {
		curntList = mzTreePos->second;
		rtTreePos = curntList->begin();
		curntPeak = rtTreePos->second;
	}
}

void PeakMatchDBIterator::init(double startMZ) {
	mzTreePos = peakMatchDB->mzTree.lower_bound(startMZ);
	// If not exactly the required MZ, go to next higher value.
	if (mzTreePos!=peakMatchDB->mzTree.end()&&mzTreePos->first<startMZ) {
		mzTreePos++;
	}
	if (mzTreePos==peakMatchDB->mzTree.end()) { // Empty DB.
		curntList = nullptr;
		curntPeak = nullptr;
	} else {
		curntList = mzTreePos->second;
		rtTreePos = curntList->begin();
		curntPeak = rtTreePos->second;
	}
}

bool PeakMatchDBIterator::done() {
	return (curntPeak==nullptr);
}

Peak PeakMatchDBIterator::nextP() {
	return (nextE()->peak);
}

PeakDBEntry* PeakMatchDBIterator::nextE() {
	PeakDBEntry *result = curntPeak;
	if (curntList!=nullptr) {
		rtTreePos++;
		if (rtTreePos==curntList->end()) { // Reached end of RT list, get next list.
			mzTreePos++;
			if (mzTreePos==peakMatchDB->mzTree.end()) {
				curntList = nullptr;
				curntPeak = nullptr;
			} else {
				curntList = mzTreePos->second;
				rtTreePos = curntList->begin();
				curntPeak = rtTreePos->second;
			}
		} else {
			curntPeak = rtTreePos->second;
		}
	}
	return (result);
}

PeakDBEntry* PeakMatchDB::getPeakDBEntry(Peak& thePeak) {
	map<double, RTTree*>::iterator mzTreePos = mzTree.find(thePeak.mX);
	if (mzTreePos==mzTree.end()) { // M/Z value not seen.
		return nullptr;
	} else {
		RTTree* rtTree = mzTreePos->second;
		RTTree::iterator rtTreePos = rtTree->find(thePeak.mY);
		if (rtTreePos==rtTree->end()) { // R/T value not seen.
			return nullptr;
		} else {
			return rtTreePos->second;
		}
	}
}

vector<Peak> PeakMatchDB::getPeaksAtMZBand(float mzCenter, float mzWidth) {
	vector<Peak> thePeaks;
	double startMZ = mzCenter-mzWidth;
	PeakMatchDBIterator peakIterator(this);
	peakIterator.init(startMZ);
	while(!peakIterator.done()) { Peak nextPeakd = peakIterator.nextP();
		Peak nextPeak= nextPeakd;
		//
		if (nextPeak.mX<=(mzCenter+mzWidth)) thePeaks.push_back(nextPeak);
		else break;
	}
	return thePeaks;
}

void PeakMatchDB::recordMatch(Peak& sourcePeak, Peak& targetPeak, int targetSmpIdx) {
	PeakDBEntry* peakDBEntry = this->getPeakDBEntry(sourcePeak);
	if (peakDBEntry==nullptr) { // Bad source peak, fail.
		throw new PeakMatchDBEx("recordMatch()> Can't find source peak in this DB.");
	} else {
		peakDBEntry->recordMatch(targetPeak, targetSmpIdx);
	}
}

