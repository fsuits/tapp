#ifndef METAMATCH_HPP
#define METAMATCH_HPP

#define DBGX 435.89
#define DBGY 87.226

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include "LCMSFile/Attribute.h"
#include "Filetypes/TAPP/MPKS.h"
#include "Filetypes/TAPP/PID.h"
#include "Filetypes/TAPP/TAPP_Output.h"
#include "LCMSFile/LCMSFile.h"

#if 0

load peaks from many files
use Default.hdr for setting tile sizes and max radius in each dir
Also set class and global peak membership fraction

file1.pks 0
file2.pks 0
file3.pks 1
:
fileN.pks k-1

Each peak loaded into appropriate tile

Load all peaks and fill tiles

sort each tile into classes and find class iterators using stl

For each peak in tile that isnt already part of a metapeak, 


Build list of peaks for each class
Build master list of peaks for whole set
Determine if any peaks overlap
Each peak should include pointers to consituent peaks



#endif

class Tile
{
public:
    // peaks sorted by class
	std::vector<Peak *> mpPeaks;
};

class Tiling {
public:
	double mMinMZ, mMaxMZ;
	double mMinRT, mMaxRT;
	double mDMZ, mDRT;
	int mNMZ, mNRT;
	Tile **mTiles;

	inline int xindex(float x) {
		return (x - mMinMZ) / (mMaxMZ - mMinMZ)*(mNMZ - 1) + 0.5;
	}

	inline int yindex(float y) {
		return (y - mMinRT) / (mMaxRT - mMinRT)*(mNRT - 1) + 0.5;
	}

	inline int xindexfloor(double x) {
		return (x - mMinMZ) / (mMaxMZ - mMinMZ)*(mNMZ - 1);
	}

	inline int yindexfloor(double y) {
		return (y - mMinRT) / (mMaxRT - mMinRT)*(mNRT - 1);
	}
};

class MetaPeak
{
public:
    // array of pointers to peaks, some of which are null
    Peak **mPeaks;
    int mNPeaks;
    int mNMembers;
    Peak mPeak;
    double mXSum;
    double mYSum;
    double mXWeightedSigma;
    double mYWeightedSigma;
    double mHeightSum;
    double *mFileVolume;
    double *mFileHeight;
    double *mClassHeight;
    double *mClassVolume;
    double *mClassFoldRatio;  // individ rel to mean of others - log
    double mExtremeFoldRatio;  // most disparate ratio - based on abs log
    int *mClassHits;
    int *mFileHits;
    int mExtremeClass;  // stand-out class
    int mNFiles;
    int *mClasses;
    int mNClasses;
    double mVFoldRatio;
    double mHFoldRatio;
    double mNoiseFloor;
    int mNFilesHit;
    int mNClassesHit;
    int *mClassFileHits;
    int mID;

    // mp is assumed to be empty
    // this initializes stats and centroid based on given peak
    void init(Peak *p) {
		mNPeaks = 1;
		mNFiles = 0;
		if (p) {
			mXSum = p->mX*p->mHeight;
			mYSum = p->mY*p->mHeight;
			mHeightSum = p->mHeight;
			mPeak.mX = p->mX;
			mPeak.mY = p->mY;
		} else {
			mNPeaks = 0;
			mXSum = 0;
			mYSum = 0;
			mHeightSum = 0;
			mPeak.mX = 0;
			mPeak.mY = 0;
		}
		mFileVolume = nullptr;
		mFileHeight = nullptr;
		mClasses = nullptr;
		mNFilesHit = 0;
		mXWeightedSigma = 0;
		mYWeightedSigma = 0;
    }

    MetaPeak() {
		init(nullptr);
    }

    // 0 means failure
    int loadBasicInfo(FILE *f) {
		char buff[4096];

		if (!fgets(buff, 4096, f))
			return 0;

		sscanf(buff, "%lf %lf %d %lf %lf %lf %lf %lf %d", &mPeak.mX, &mPeak.mY, &mNPeaks, &mPeak.mXSig, &mPeak.mYSig,
			&mPeak.mHeight, &mPeak.mVolume, &mExtremeFoldRatio, &mExtremeClass);

		return 1;
    }

	std::vector<TAPP::Filetypes::TAPP::PID> GetPID(void)
	{
		std::vector<TAPP::Filetypes::TAPP::PID> pid_records;
		for (int file = 0; file < mNFiles; ++file)
		{
			for (int peak = 0; peak < this->mNPeaks; ++peak)
			{
				Peak* p = this->mPeaks[peak];
				if (p->mFile == file)
				{
					pid_records.push_back(
					{
						static_cast<int64_t>(mID),
						(unsigned char)p->mFile,
						(unsigned char)p->mClass,
						static_cast<uint64_t>(p->mID),
						p->mX,
						p->mY,
						p->mHeight
					});
				}
			}
		}
		return pid_records;
	}

    void dump(std::ostream &sout, bool showheader=true)
	{
		// cout << mPeak.mX << " " << mPeak.mY << endl;
		if (showheader)
		{
			sout << "mpid mz rt Height FileID PeakID Class" << std::endl;
		}
		for (int nf=0; nf<mNFiles; nf++)
		{
			for (int i=0; i<this->mNPeaks; i++)
			{
				Peak *p = this->mPeaks[i];
				if (p->mFile == nf)
				{
					// cout << id << " " << p->mFile << " " <<  p->mX << " " << p->mY << " " << p->mHeight << endl;
					sout << mID << " " << p->mX << " " << p->mY << " " << p->mHeight << " " << p->mFile << " " << p->mID << " " << p->mClass << std::endl;
				}
			}
		}
    }

    void dump() {
		dump(std::cout);
    }

    // add peak to this metapeak
    // only change stats and reference - don't change allocation within peak
    void addPeak(Peak *p) {
		mXSum += p->mX*p->mHeight;
		mYSum += p->mY*p->mHeight;
		mHeightSum += p->mHeight;
		mNPeaks++;
		p->mMetaPeak = this;
		mPeak.mX = mXSum/mHeightSum;
		mPeak.mY = mYSum/mHeightSum;
    }

    // add a peak to a metapeak
    // this involves placing all the current peaks in the allpeaks buffer,
    // adding the new peak
    // and filling the metapeak from scratch
    void append(Peak *p, Peak **allpeaks, int nfiles, int *classes, float noisefloor, int nclasses) {
		for (int i=0; i<mNPeaks; i++) {
			allpeaks[i] = mPeaks[i];
		}
		allpeaks[mNPeaks] = p;
		p->mMetaPeak = this;
		int npks = mNPeaks+1;
		free();
		mNPeaks = npks;
		fillInfo(allpeaks, nfiles, classes, noisefloor, nclasses);
    }

    // dereference the metapeak pointed to by these peaks
    void removePeaks() {
		for (int i=0; i<mNPeaks; i++)
			mPeaks[i]->mMetaPeak = nullptr;
		mNPeaks = 0;
    }

    void findFoldRatio();

    void free() {
		if (mPeaks) {
			delete [] mPeaks;
			mPeaks = nullptr;
		}
		if (mFileVolume) {
			delete [] mFileVolume;
			mFileVolume = nullptr;
		}
		if (mFileHeight) {
			delete [] mFileHeight;
			mFileHeight = nullptr;
		}
		if (mClassHeight) {
			delete [] mClassHeight;
			mClassHeight = nullptr;
		}
		if (mClassVolume) {
			delete [] mClassVolume;
			mClassVolume = nullptr;
		}
		if (mClassFoldRatio) {
			delete [] mClassFoldRatio;
			mClassFoldRatio = nullptr;
		}
		if (mClassHits) {
			delete [] mClassHits;
			mClassHits = nullptr;
		}
		if (mFileHits) {
			delete [] mFileHits;
			mFileHits = nullptr;
		}
		if (mClassFileHits) {
			delete [] mClassFileHits;
			mClassFileHits = nullptr;
		}

		if (mClasses)
			mClasses = nullptr;
    }

    // *classes is a pointer to the class corresponding to each file.  So, there are mNFiles int's in classes
    // AllPeaks is assumed to be the MetaMatch PeakBuffer
    // The number of peaks in there is set both by MetaMatch.mNPeaksInBuffer, and mp.mNPeaks
    // This assumes mNPeaks has been set properly
    void fillInfo(Peak **AllPeaks, int nfiles, int *classes, double noisefloor, int nclasses) {
		// allocate space for this metapeak, given number of support peaks and classes
		mPeaks = new Peak *[mNPeaks];
		mNFiles = nfiles;
		mFileVolume = new double[mNFiles];
		mFileHeight = new double[mNFiles];
		mNClasses = nclasses;
		mClassHeight = new double[mNClasses];
		mClassVolume = new double[mNClasses];
		mClassFoldRatio = new double[mNClasses];
		mClassHits = new int[mNClasses];  // any peak adds here
		mFileHits = new int[mNFiles];
		mClassFileHits = new int[mNClasses];  // any file adds here
		mExtremeFoldRatio = 0;
		mExtremeClass = 0;
		mNoiseFloor = noisefloor;

		for (int i=0; i<mNFiles; i++)
			mFileVolume[i] = mFileHeight[i] = mFileHits[i] = 0;

		for (int i=0; i<mNClasses; i++)
			mClassHeight[i] = mClassVolume[i] = mClassFoldRatio[i] = mClassHits[i] = mClassFileHits[i] = 0;

		mClasses = classes;

		double xsq = 0;
		double ysq = 0;
		double hsum = 0;
		double xwsq = 0;
		double ywsq = 0;
		mPeak.mHeight = 0;
		mPeak.mVolume = 0;
		// loop over all peaks and calc stats per file
		// the centroid (mPeak.mX,Y) is assumed to be up to date as the metapeak grows
		for (int v=0; v<mNPeaks; v++) {
			Peak *p = AllPeaks[v];
			mPeaks[v] = p;
			double dx = mPeak.mX - p->mX;
			double dy = mPeak.mY - p->mY;
			xsq += dx*dx;
			ysq += dy*dy;
			hsum += p->mHeight;
			xwsq += p->mHeight*dx*dx;
			ywsq += p->mHeight*dy*dy;
			// mPeak.mHeight += p->mHeight;
			if (mPeak.mHeight < p->mHeight)
				mPeak.mHeight = p->mHeight;
			mPeak.mVolume += p->mVolume;
			mFileVolume[p->mFile] += p->mVolume;
			int c = mClasses[p->mFile];
			mClassHits[c]++;
			mFileHits[p->mFile]++;
			// mFileHeight[p->mFile] += p->mHeight;
			if (p->mHeight > mFileHeight[p->mFile])  // for height, use largest peak for each file - don't add
				mFileHeight[p->mFile] = p->mHeight;
    		mClassVolume[c] += p->mVolume;
		}
		// This is a geometric
		mPeak.mXSig = sqrt(xsq/mNPeaks);
		mPeak.mYSig = sqrt(ysq/mNPeaks);
		mXWeightedSigma = sqrt(xwsq/hsum);
		mYWeightedSigma = sqrt(ywsq/hsum);
		// mPeak.mHeight /= mNPeaks;

		mNFilesHit = 0;
		for (int i=0; i<mNFiles; i++)
			if (mFileVolume[i] != 0)
			mNFilesHit++;

		mNClassesHit = 0;
		for (int i=0; i<mNClasses; i++)
			if (mClassHits[i] > 0)
			mNClassesHit++;

		for (int i=0; i<mNClasses; i++) {
			if (mClassHits[i] > 0) {
			mClassVolume[i] /= mClassHits[i];
			}
		}

		// now loop over files and find class average of heights
		// each file height is MAX of peaks in that file
		// each class height is MEAN of files contributing

		for (int i=0; i<mNFiles; i++) {
			int c = mClasses[i];
			if (mFileVolume[i] != 0) {
				mClassFileHits[c]++;
				mClassHeight[c] += mFileHeight[i];
			}
		}

		// class height is mean of files
		double maxht = 0;
		double newvol = 0;
		for (int i=0; i<mNClasses; i++) {
			if (mClassFileHits[i] > 0)
				mClassHeight[i] /= mClassFileHits[i];
			else
				mClassHeight[i] = mNoiseFloor;
			if (maxht < mClassHeight[i]) {
				maxht = mClassHeight[i];
				newvol = mClassVolume[i];
			}
		}

		// assign peak height as max height in class
		mPeak.mHeight = maxht;
		// assign peak volume as 
		mPeak.mVolume = newvol;

		findFoldRatio();
    }

    bool operator<(const MetaPeak& p2) const {
        return (mPeak.mHeight > p2.mPeak.mHeight);  // forces descending sort
    }

};

class MetaMatch
{
public:
    LCMSFile mLCMSFile;
    int mNClasses; // including master?
    int mNFiles;
    double mMRadius;
    double mTRadius;
    char mOutStem[512];
    double mFraction;
    double mNoiseFloor;
    int *mClasses;
	Tiling mTiling;

    enum {MAXBUFFERPEAKS=4096};
    Peak *mPeakBuffer[MAXBUFFERPEAKS];
    int mNPeaksInBuffer;

    std::vector<MetaPeak> mMetaPeaks;

    enum {MASTER=-1};

    // long list of all peaks
	std::vector<Peak> mAllPeaks;

    // load peaks from file into master lists
    void loadIntoClass(char *fname, int FileID, int Class);

    // load all files
    void load(char *fname);

    void getPeakStats(int &nmpks, int &norph);

    // using Default.hdr, layout tiles and sort peaks into them
    void buildTiles();

    void glomOrphans();

    bool outsideTile(int i, int j, MetaPeak &mp) {
		float x = mp.mPeak.mX;
		float y = mp.mPeak.mY;
		float xmin = mLCMSFile.mMesh.mConversion.mMinMZ+i*mLCMSFile.mMesh.mConversion.mDMZ;
		if (x < xmin)
			return true;
		if (x > xmin+mLCMSFile.mMesh.mConversion.mDMZ)
			return true;
		float ymin = mLCMSFile.mMesh.mConversion.mMinRT+j*mLCMSFile.mMesh.mConversion.mDRT;
		if (y < ymin)
			return true;
		if (y > ymin+mLCMSFile.mMesh.mConversion.mDRT)
			return true;
		return false;
    }

    void getTileIndices(Peak *p, int &i, int &j) {
		i = mTiling.xindex(p->mX);
		j = mTiling.yindex(p->mY);
    }

    // add a peak to a tile
    inline void addPeak(Peak *p) {
		int i = mTiling.xindexfloor(p->mX);
		int j = mTiling.yindexfloor(p->mY);
		if (i >=0 && i<mTiling.mNMZ && j >=0 && j<mTiling.mNRT)
			mTiling.mTiles[j][i].mpPeaks.push_back(p);
    }

    inline bool isAvailable(Peak *r, Peak *p)
    {
		if (p->mMetaPeak != nullptr)
			return false;
		if (r->mID == p->mID && r->mFile == p->mFile)
			return false;
		return true;
    }

	inline bool isAvailableRefactored(Peak* current, Peak* neighbor)
	{
		return current->mMetaPeak && !(current->mID == neighbor->mID && current->mFile == neighbor->mFile);
	}

    inline bool inRange(MetaPeak &m, Peak *p)
    {
	bool checking=false;
	double dx = fabs(p->mX-m.mPeak.mX);
	if (dx > mMRadius) {
		if (checking)
			std::cout << " false" << std::endl;
		return false;
	}
	double dy = fabs(p->mY-m.mPeak.mY);
	if (dy > mTRadius) {
	    if (checking)
			std::cout << " false" << std::endl;
	    return false;
	}
	if (checking)
		std::cout << " true" << std::endl;
	return true;
    }

    // add peak to metapeak
    // and place buffer in list
    // and point peak at metapeak
    inline void addToMetaPeak(MetaPeak &m, Peak *p)
    {
		m.addPeak(p);
		mPeakBuffer[mNPeaksInBuffer++] = p;
		p->mMetaPeak = &m;
    }

    // go through all metapeaks
    // dereference all peaks that point to weak metapeaks
    // this metapeak is assumed to be filled and allocated
    void FilterPeaks() {
		for (int i=0; i<mMetaPeaks.size(); i++) {
			Peak *p = &mMetaPeaks[i].mPeak;
			if (float(mMetaPeaks[i].mNFilesHit)/mNFiles < mFraction) {
				mMetaPeaks[i].removePeaks();
				mMetaPeaks[i].free();
			}
		}
    }


    // remove any peaks far from centroid and recalc stats
    // this operates on the list of files in PeakBuffer
    // this assumes mp isn't filled yet
    void cullFarPeaks(MetaPeak &mp)
    {
		int nculled = 0;
		// zap any peaks that are outside range of current centroid
		for (int i=0; i<mNPeaksInBuffer; i++) {
			if (!inRange(mp, mPeakBuffer[i])) {
				Peak *p = mPeakBuffer[i];

				mp.mXSum -= p->mX*p->mHeight;
				mp.mYSum -= p->mY*p->mHeight;
				mp.mHeightSum -= p->mHeight;
				mp.mPeak.mX = mp.mXSum/mp.mHeightSum;
				mp.mPeak.mY = mp.mYSum/mp.mHeightSum;
				mPeakBuffer[i]->mMetaPeak = nullptr;
				mPeakBuffer[i] = nullptr;
				nculled++;
			}
		}
		// if any zapped, collapse PeakBuffer so it only has peaks in range
		if (nculled) {
			int a = 0;
			int b = 1;
			for (int i=0; i<mNPeaksInBuffer; i++) {
			if (mPeakBuffer[i] != nullptr) {
				a++;
			}
			if (a != b) {
				mPeakBuffer[a] = mPeakBuffer[b];
			}
			b++;
			}
			mNPeaksInBuffer -= nculled;

			// recalc stats - particularly centroid
			mp.init(mPeakBuffer[0]);
			for (int i=1; i<mNPeaksInBuffer; i++)
			mp.addPeak(mPeakBuffer[i]);
		}
    }

    // find peaks within each class and across classes
    // add peaks to class and master list
    // each peak should have pointers to constituent peaks and accumulated stats
    // class peak will have array of pointers to class members
    // master peak will have longer array
    // a peak can only be in one class peak and one master peak

    // for each peak not already matched:
    // loop:
    //   find all unmatched peaks nearby
    //   find centroid
    //   find unmatched peaks nearby
    // until npeaks does not increase
    // if npeaks big enough, mark as new peak and build stats
    // class peaks are either matched or separate
    // need tolerance for class matching and 

    int matchPeaks();
	int MatchPeaksRefactored(void);

    void dumpPeaks(char *fstem, bool statsonly=false);

    void dumpNear(float x, float y, float e, float f);

    void initDefault()
    {
		mNClasses = 0;
		mNoiseFloor = 1.0;
		mLCMSFile.loadAttributes("Default.hdr");
		mLCMSFile.setAttributes();
    }

    MetaMatch() {
		initDefault();
    }

    MetaMatch(char *fname) {
		initDefault();
		load(fname);
    }

    void dumpTiles(int a, int b);

    void findFoldRatio() {
		for (int i=0; i<mMetaPeaks.size(); i++) {
			mMetaPeaks[i].findFoldRatio();
		}
    }

    void reset() {
		for (int i=0; i<mAllPeaks.size(); i++)
			mAllPeaks[i].mMetaPeak = nullptr;
		for (int i=0; i<mMetaPeaks.size(); i++)
			mMetaPeaks[i].free();
		mMetaPeaks.clear();
		delete [] mTiling.mTiles;
    }

    void initClusterParams(double mrad, double trad, double frac, double noise) {
		mMRadius = mrad;
		mTRadius = trad;
		mFraction = frac;
		mNoiseFloor = noise;
    }

    void setOutStem(char *stem) {
        strcpy(mOutStem, stem); 
    }
};

#endif
