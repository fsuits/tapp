// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#include <stdio.h>
#include "MetaMatch.hpp"

using namespace std;

void MetaMatch::loadIntoClass(char *fname, int FileID, int Class)
{
    Mesh::loadPeaks(mAllPeaks, fname, FileID, Class);
}

void MetaMatch::load(char *fname)
{
	std::string filepath(fname);
	if (filepath.find(".pbin", 0) != std::string::npos)
	{
		std::unordered_set<uint32_t> classes(Mesh::LoadPeaksFromPBIN(filepath, mAllPeaks));
		mClasses = new int[classes.size()];

		size_t current_class = 0;
		for (uint32_t class_id : classes)
		{
			mClasses[current_class] = class_id;
			++current_class;
		}
	}
	else
	{
		vector<int> classes;
		FILE *f = fopen(fname, "r");
		if (!f) {
			cerr << "Can't open file " << fname << endl;
			exit(-1);
		}
		char buff[1024];
		int nc = 0;
		int nf = 0;
		while (!feof(f)) {
			int nread = fscanf(f, "%s %d\n", buff, &nc);
			if (nread == 2) {
				loadIntoClass(buff, nf, nc);
				if (mNClasses < nc + 1)
					mNClasses = nc + 1;
				nf++;
				classes.push_back(nc);
			}
		}
		fclose(f);
		mNFiles = nf;
		mClasses = new int[nf];
		for (int i = 0; i<mNFiles; i++)
			mClasses[i] = classes[i];
	}
}

// allocate tiles
// sort peaks into them
// by construction, each will be sorted by class, assuming classes loaded in order
// make tile size equal to 5 times the radii
void MetaMatch::buildTiles()
{
    const double rfactor = 10.0; //for testing we changed this to 100 but original value is 10
    mTiling.mDMZ = rfactor*mMRadius;
    mTiling.mDRT = rfactor*mTRadius;
    mTiling.mNMZ = (mTiling.mMaxMZ-mTiling.mMinMZ)/mTiling.mDMZ+0.5;
    mTiling.mNRT = (mTiling.mMaxRT-mTiling.mMinRT)/mTiling.mDRT+0.5;
    mTiling.mTiles = new Tile *[mTiling.mNRT];
    for (int j=0; j<mTiling.mNRT; j++)
		mTiling.mTiles[j] = new Tile[mTiling.mNMZ];
    for (int i=0; i<mAllPeaks.size(); i++)
		addPeak(&mAllPeaks[i]);
}

// go to tile
// for each unmatched peak-
//   set centroid equal to peak location
//   find peaks in all tiles within radius
//   as each peak is added, recalc centroid
//   if centroid wanders out of range, undo all
//   make sure to avoid self
//   each master peak has count in it
//   if consituent peaks are members of other peaks with lower counts, undo them
int MetaMatch::matchPeaks()
{
    bool havehit;
    MetaPeak mp;
    int nmp = mMetaPeaks.size();

    int nMetaPeaks;
 
    nMetaPeaks = mMetaPeaks.size();

    // loop over tiles
    for (int j=1; j<mTiling.mNRT-1; j++) {
		for (int i=1; i<mTiling.mNMZ-1; i++) {
  
            vector<Peak *> *t = &mTiling.mTiles[j][i].mpPeaks;
	    
            // loop within peaks of this tile
			havehit = false;
			for (vector<Peak *>::iterator r = t->begin(); r != t->end(); r++) {
				if ((*r)->mMetaPeak)  // skip if already bound
					continue;
				mp.init(*r);  // bind
				(*r)->mMetaPeak = &mp;  // ! this mp is on the stack!  dont reference, but use to show there's a link
				mPeakBuffer[0] = *r;
				mNPeaksInBuffer = 1;
                
				// now loop over all local tiles
				bool didsomething=true;
				bool ranoff = false;
               
				while (didsomething) {
   					didsomething=false;
               
					for (int q=j-1; q<=j+1 && !ranoff; q++) {
						for (int p=i-1; p<=i+1 && !ranoff; p++) {
			                                              
							vector<Peak *> *u = &mTiling.mTiles[q][p].mpPeaks;
   							// and add peaks that are within range of current centroid
							for (vector<Peak *>::iterator s = u->begin(); s != u->end() && !ranoff; s++) {
								if (!isAvailable(*r, *s))
									continue;
                           
								if ((inRange(mp, *s))) {
									didsomething=true;
  
									addToMetaPeak(mp, *s);

									cullFarPeaks(mp);
								}                        
							} // end s iterator loop
		                    
						}  // end p loop
                    
     				} // end q loop

				}  // end while stmt
				if (mp.mNPeaks) {
					mp.fillInfo(mPeakBuffer, mNFiles, mClasses, mNoiseFloor, mNClasses);
					if (float(mp.mNFilesHit)/mp.mNFiles < this->mFraction) {
						for (int kk=0; kk<mNPeaksInBuffer; kk++)
							mPeakBuffer[kk]->mMetaPeak = nullptr;
						mNPeaksInBuffer = 0;
						mp.free();
					} else {
						mMetaPeaks.push_back(mp);
					}
				}
			} // end r iterator loop
		} // end i loop 
    } // end j loop

    return mMetaPeaks.size() != nmp;
}

int MetaMatch::MatchPeaksRefactored(void)
{
	bool havehit;
	MetaPeak mp;
	int nmp = mMetaPeaks.size();

	int nMetaPeaks;

	nMetaPeaks = mMetaPeaks.size();

	// Iterates over each tile.
	for (int y_axis = 1; y_axis < mTiling.mNRT - 1; y_axis++)
	{
		for (int x_axis = 1; x_axis < mTiling.mNMZ - 1; x_axis++)
		{
			std::vector<Peak*>& current_tile = mTiling.mTiles[y_axis][x_axis].mpPeaks;

			// loop within peaks of this tile
			havehit = false;
			for (Peak* current_peak : current_tile)
			{
				if (!current_peak->mMetaPeak)
				{
					mp.init(current_peak);  // bind
					current_peak->mMetaPeak = &mp;  // ! this mp is on the stack!  dont reference, but use to show there's a link
					mPeakBuffer[0] = current_peak;
					mNPeaksInBuffer = 1;

					// now loop over all local tiles
					bool didsomething = true;
					bool ranoff = false;
					while (didsomething)
					{
						didsomething = false;

						// Iterates over each local tile.
						for (int local_y_axis = y_axis - 1; local_y_axis <= y_axis + 1 && !ranoff; local_y_axis++)
						{
							for (int local_x_axis = x_axis - 1; local_x_axis <= x_axis + 1 && !ranoff; local_x_axis++)
							{
								vector<Peak*>& local_tile = mTiling.mTiles[local_y_axis][local_x_axis].mpPeaks;

								// Adds peaks that are within range of current centroid
								for (Peak* local_peak : local_tile)
								{
									if (isAvailable(current_peak, local_peak) && inRange(mp, local_peak))
									{
										didsomething = true;
										addToMetaPeak(mp, local_peak);
										cullFarPeaks(mp);
									}
								}
							}
						}
					}

					if (mp.mNPeaks)
					{
						mp.fillInfo(mPeakBuffer, mNFiles, mClasses, mNoiseFloor, mNClasses);
						if (float(mp.mNFilesHit) / mp.mNFiles < this->mFraction)
						{
							for (int kk = 0; kk < mNPeaksInBuffer; kk++)
							{
								mPeakBuffer[kk]->mMetaPeak = nullptr;
							}
							mNPeaksInBuffer = 0;
							mp.free();
						}
						else
						{
							mMetaPeaks.push_back(mp);
						}
					}
				}
			}
		}
	}

	return mMetaPeaks.size() != nmp;
}

void MetaMatch::glomOrphans()
{
    int nmp = mMetaPeaks.size();

    for (int i=0; i<nmp; i++) {
		MetaPeak *mp = &mMetaPeaks[i];
		int ti, tj;
		getTileIndices(&mp->mPeak, ti, tj);
		for (int ii=ti-1; ii<ti+1; ii++) {
			if (ii<0)
				continue;
			if (ii >= mTiling.mNMZ)
				continue;
			for (int jj=tj-1; jj<tj+1; jj++) {
				if (jj<0)
					continue;
				if (jj >= mTiling.mNRT)
					continue;
				vector<Peak *> *t = &mTiling.mTiles[jj][ii].mpPeaks;
				// loop within peaks of this tile
				for (vector<Peak *>::iterator r = t->begin(); r != t->end(); r++) {
					if ((*r)->mMetaPeak)  // skip if already bound
						continue;
					if (inRange(*mp, *r)) {
						mp->append(*r, mPeakBuffer, mNFiles, mClasses, mNoiseFloor, mNClasses);
					}
				}
			}
		}
    }
}

void MetaMatch::getPeakStats(int &nmpks, int &norph) {
    nmpks = mMetaPeaks.size();
    norph = 0;
    for (int i=0; i<mAllPeaks.size(); i++) {
		if (mAllPeaks[i].mMetaPeak == nullptr)
			norph++;
	}
}

void MetaMatch::dumpPeaks(char *fstem, bool statsonly)
{
    static bool first = true;
    double sumnfiles = 0;

    char buff[1024];

    // dump header
    if (!statsonly)
	{
        sprintf(buff, "%s.mpks", fstem);

		std::vector<TAPP::Filetypes::TAPP::MPKS> mpks_records;
		mpks_records.reserve(mMetaPeaks.size());
		for (int i = 0; i<mMetaPeaks.size(); i++)
		{
			sumnfiles += mMetaPeaks[i].mNFilesHit;
			if (!statsonly && mMetaPeaks[i].mNPeaks)
			{
				mpks_records.push_back(
				{
					(size_t)mMetaPeaks[i].mID,
					mMetaPeaks[i].mPeak.mX,
					mMetaPeaks[i].mPeak.mY,
					(size_t)mMetaPeaks[i].mNPeaks,
					mMetaPeaks[i].mPeak.mXSig,
					mMetaPeaks[i].mPeak.mYSig,
					mMetaPeaks[i].mXWeightedSigma,
					mMetaPeaks[i].mYWeightedSigma,
					mMetaPeaks[i].mPeak.mHeight,
					mMetaPeaks[i].mPeak.mVolume,
					mMetaPeaks[i].mExtremeFoldRatio,
					(double)mMetaPeaks[i].mExtremeClass,
				});

				for (int c = 0; c < mNClasses; ++c)
				{
					mpks_records.back().class_values.push_back(mMetaPeaks[i].mClassHeight[c]);
				}

				for (int f = 0; f < mNFiles; f++)
				{
					mpks_records.back().file_values.push_back(mMetaPeaks[i].mFileHeight[f]);
				}
			}
		}

		fstream f(buff, std::ios_base::out);
		TAPP::Filetypes::TAPP::SetGlobalPrecision(f);
		TAPP::Filetypes::TAPP::WriteMPKS(f, mpks_records, { '\n', ' ', '"' }, true);
		f.close();
    }

    if (!statsonly)
	{
		sprintf(buff, "%s.pid", fstem);

        fstream g(buff, std::ios_base::out);
		TAPP::Filetypes::TAPP::SetGlobalPrecision(g);

		bool showheader = true;
		for (int i = 0; i<mMetaPeaks.size(); i++)
		{
			std::vector<TAPP::Filetypes::TAPP::PID> pid_records(mMetaPeaks[i].GetPID());
			TAPP::Filetypes::TAPP::WritePID(g, pid_records, { '\n', ' ', '"' }, showheader);
			showheader = false;
		}

		g.close();
    }

    int norphans = 0;

    for (int i=0; i<mAllPeaks.size(); i++) {
		if (mAllPeaks[i].mMetaPeak == nullptr)
			norphans++;
    }

    double xsum = 0;
    double ysum = 0;
    double nsum = 0;
    for (int i=0; i<mMetaPeaks.size(); i++) {
		xsum += mMetaPeaks[i].mPeak.mXSig;
		ysum += mMetaPeaks[i].mPeak.mYSig;
		nsum += mMetaPeaks[i].mNPeaks;
    }

    sprintf(buff, "%s.stats", fstem);
    ios_base::openmode mode = std::ios_base::out;
    if (!first)
        mode |= std::ios_base::app;
    fstream h(buff, mode);

    if (first)
		h << "MRadius TRadius Fraction NMetaPeaks MeanXSigma MeanYSigma MeanNPeaks MeanNFilesHit NPeaks NOrphans" << endl;

    h << mMRadius;
    h << " " << mTRadius;
    h << " " << mFraction;
    h << " " << mMetaPeaks.size();
    h << " " << xsum/mMetaPeaks.size();
    h << " " << ysum/mMetaPeaks.size();
    h << " " << float(nsum)/mMetaPeaks.size();
    h << " " << sumnfiles/mMetaPeaks.size();
    h << " " << mAllPeaks.size();
    h << " " << norphans << endl;

    first = false;

    h.close();

    // dump orphans
    if (!statsonly)
	{
		sprintf(buff, "%s.orph", fstem);
		std::vector<TAPP::Filetypes::TAPP::PID> pid_records;
		for (int i = 0; i < this->mAllPeaks.size(); ++i)
		{
			if (!mAllPeaks[i].mMetaPeak)
			{
				pid_records.push_back(
				{
					static_cast<int64_t>(-1),
					(unsigned char)mAllPeaks[i].mFile,
					(unsigned char)mAllPeaks[i].mClass,
					static_cast<uint64_t>(mAllPeaks[i].mID),
					mAllPeaks[i].mX,
					mAllPeaks[i].mY,
					mAllPeaks[i].mHeight
				});
			}
		}

        fstream f(buff, std::ios_base::out);
		TAPP::Filetypes::TAPP::SetGlobalPrecision(f);
		TAPP::Filetypes::TAPP::WritePID(f, pid_records, { '\n', ' ', '"' }, true);
		f.close();		
    }
}

void MetaMatch::dumpNear(float x, float y, float e, float f)
{
    cout << "MetaPeaks near " << x << " " << y << " within " << e << " " << f << endl;
    int n = 0;
    for (int i=0; i<mMetaPeaks.size(); i++ ) {
		MetaPeak *mp = &mMetaPeaks[i];
		Peak *p = &mp->mPeak;
		if (fabs(p->mX-x)>e || fabs(p->mY-y)>f)
			continue;
		if (mp->mNPeaks)
			cout << 1;
		else
			cout << 0;
		cout << " " << n << " " << i << " " << p->mX << " " << p->mY << " " << p->mHeight << " " << p->mXSig << " " << p->mYSig << " " << mp->mExtremeFoldRatio << endl;
		for (int j=0; j<mp->mNPeaks; j++) {
			Peak *q = mp->mPeaks[j];
			cout << "    " << n+2 << " " << j << " " << q->mFile << " " << q->mX << " " << q->mY << " " << q->mHeight << " " << q->mID << " " << 0 << " " << q->mClass << endl;
		}
		n++;
    }
    for (int j=0; j<mAllPeaks.size(); j++) {
		if (mAllPeaks[j].mMetaPeak)
			continue;
		Peak *q = &mAllPeaks[j];
		if (fabs(q->mX-x)>e || fabs(q->mY-y)>f)
			continue;
		cout << "    " << n+2 << " " << j << " " << q->mFile << " " << q->mX << " " << q->mY << " " << q->mHeight << " " << q->mID << " " << 0 << " " << q->mClass << endl;
    }
}

void MetaMatch::dumpTiles(int a, int b)
{
    for (int q=b-1; q<=b+1; q++) {
		for (int p=a-1; p<=a+1; p++) {
			vector<Peak *> *u = &mTiling.mTiles[q][p].mpPeaks;
			for (vector<Peak *>::iterator s = u->begin(); s != u->end(); s++) {
				cout << p << " " << q << " " << (*s)->mX << " " << (*s)->mY << " " << (*s)->mHeight << " " << (*s)->mVolume << endl;
			}
		}
    }
}

// have stats on each class
// find extreme
void MetaPeak::findFoldRatio()
{
    mExtremeFoldRatio = 0;
    mExtremeClass = 0;
    if (mNClasses == 1) {
		return;
    }
    if (mNClasses == 2) {
		mExtremeFoldRatio = log(mClassHeight[0]/mClassHeight[1]);
		return;
    }
    for (int i=0; i<mNClasses; i++) {
		double othersum = 0;
		for (int j=0; j<mNClasses; j++) {
			if (j == i)
				continue;
			othersum += mClassHeight[j];
		}

		mClassFoldRatio[i] = log(mClassHeight[i]/othersum*(mNClasses-1));
		if (fabs(mExtremeFoldRatio) < fabs(mClassFoldRatio[i])) {
			mExtremeFoldRatio = mClassFoldRatio[i];
			mExtremeClass = i;
		}
    }
}
