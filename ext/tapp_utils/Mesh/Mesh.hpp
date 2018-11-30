#pragma once
#ifndef MESH_HPP
#define MESH_HPP

#include <string.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <unordered_set>
#include <vector>

#include "Mesh/Encoding.h"
#include "Mesh/ConversionSpec.h"
#include "DoubleMatrix.h"

#include "Filetypes/TAPP/PKS.h"

#include "Filetypes/TAPP/TAPP_Output.h"

#include "MiniPeak.h"

class Point2D {
public:
    double x;
    double y;
};

class Data2D {
public:
    Point2D p;
    double v;
};

class IndexEntry {
public:
    float mRT;
    std::streampos mOffset;
    int mScanNum;
    int mNPeaks;

    bool operator==(const IndexEntry &ie2) const { return (mRT == ie2.mRT); }

    bool operator<(const IndexEntry &ie2) const { return (mRT < ie2.mRT); }

    bool operator==(const float &rt) const { return (mRT == rt); }

    bool operator<(const float &rt) const { return (mRT < rt); }
};

class IndexFile {
public:
    char mFName[1024];

    std::vector<IndexEntry> mEntries;

    float mRTMin;
    float mRTMax;

    inline bool isEmpty() const { return mEntries.size() == 0; }

    void load(const char *fname) {
        strcpy(mFName, fname);
        FILE *f = fopen(mFName, "r");
        if (!f) return;  // leave mEntries empty
        IndexEntry ie;
        while (!feof(f)) {
            fscanf(f, "%lld %f %d %d", &ie.mOffset, &ie.mRT, &ie.mScanNum, &ie.mNPeaks);
            mEntries.push_back(ie);
        }
        fclose(f);
        mRTMin = mEntries.front().mRT;
        mRTMax = mEntries.back().mRT;

        std::cout << "Loaded index file, " << mEntries.size()
                  << " entries with rt range: " << mEntries.front().mRT << " "
                  << mEntries.back().mRT << std::endl;
    }

    // return first line with rt equal to or greater than query
    inline std::streampos getOffset(const float rt) {
        if (mEntries.size() == 0) 
			return -1;
        if (rt <= mRTMin) 
			return mEntries.front().mOffset;
        if (rt >= mRTMax) 
			return mEntries.back().mOffset;
        std::vector<IndexEntry>::iterator val;
        val = lower_bound(mEntries.begin(), mEntries.end(), rt);
        return val->mOffset;
    }
};

class TimeMap {
    struct TimeMapEntry {
        double mIn;
        double mOut;
    };

    std::vector<TimeMapEntry> mMap;

    double mMin;
    double mMax;
    int mCount;

public:
    TimeMap() { mCount = 0; }

    void load(char *fname) {
        FILE *f = fopen(fname, "r");
        while (!feof(f)) {
            TimeMapEntry tme;
            int nread = fscanf(f, "%lf %lf\n", &tme.mIn, &tme.mOut);
            if (nread == 2) 
				mMap.push_back(tme);
        }
        mMin = mMap[0].mIn;
        mCount = mMap.size();
        mMax = mMap[mCount - 1].mIn;
    }

    inline double lookUp(const double t) const {
        if (t <= mMin) 
			return mMap[0].mOut - (mMin - t);
        if (t >= mMax) 
			return mMap[mCount - 1].mOut + (t - mMax);

        // go until just past left index
        for (int i = 0; i < mCount - 2; i++) {
            if (t <= mMap[i + 1].mIn)
                return mMap[i].mOut + (t - mMap[i].mIn) /
                                          (mMap[i + 1].mIn - mMap[i].mIn) *
                                          (mMap[i + 1].mOut - mMap[i].mOut);
        }

        return mMap[mCount - 1].mOut + t - mMax;
    }

    int getSize() const { return mCount; }

    void clear() {
        mMap.clear();
        mCount = 0;
    }
};

class Bound {
public:
    int mMax;

    Bound() { 
		mMax = 0;
	}

    void SetMax(const int n) { 
		mMax = n; 
	}

    Bound(const int n) { 
		mMax = n;
	}

    inline void Clamp(int &n) const {
        if (n < 0) {
            n = 0;
            return;
        }
        if (n >= mMax) {
            n = mMax - 1;
            return;
        }
        return;
    }
};

class MetaPeak;

class Peak {
public:
    int mID;
    double mX;
    double mY;
    double mXFullCentroid;
    double mYFullCentroid;
    double mXPeak;
    double mYPeak;
    int mI;
    int mJ;
    double mXSig;
    double mYSig;
    double mHeight;
    double mVolume;
    double mBorderBkgnd;
    double mSNVolume;
    double mSNHeight;
    double mSNCentroid;
    double mVCentroid;
    int mCount;
    int mNRefs;
    int mClass;
    int mFile;
    MetaPeak *mMetaPeak;

    Peak() {
        mID = -1;
        mX = 0;
        mY = 0;
        mI = 0;
        mJ = 0;
        mHeight = 0;
        mXSig = 0;
        mYSig = 0;
        mCount = 0;
        mVolume = 0;
        mXPeak = 0;
        mYPeak = 0;
        mBorderBkgnd = 0;
        mNRefs = 0;
        mClass = 0;
        mFile = 0;
        mMetaPeak = nullptr;
    }

    Peak(int i, int j, double h, double x = 0, double y = 0) {
        mID = -1;
        mX = x;
        mY = y;
        mI = i;
        mJ = j;
        mXSig = 0;
        mYSig = 0;
        mCount = 0;
        mHeight = h;
        mVolume = 0;
        mBorderBkgnd = 0;
        mNRefs = 0;
        mClass = 0;
        mFile = 0;
        mMetaPeak = nullptr;
    }

    bool operator==(const Peak &p2) const {
        return ((mHeight == p2.mHeight) && (mVolume == p2.mVolume));
    }

    // this behaves as p1 > p2
    bool operator<(const Peak &p2) const {
        return ((mHeight > p2.mHeight) ||
                ((mHeight == p2.mHeight) &&
                 (mVolume > p2.mVolume)));  // forces descending sort
    }

    void addRef() { mNRefs++; }

    int getNRefs() const { return mNRefs; }

    double Disparity(const Peak &p) const {
        double dx = (p.mX - mX) / (mXSig * 2 + 0.5);
        double dy = (p.mY - mY) / (mYSig * 2 + 1);
        double r = sqrt(dx * dx + dy * dy);
        if (r > 1) return 2;
        double f = fabs(p.mHeight - mHeight) /
                   (p.mHeight + mHeight);  // 0 if identical, 1 if one absent
        return f * r;
    }
};

inline bool SigSort(const Peak &p1, const Peak &p2) {
    return (p1.mHeight > p2.mHeight);
}

/*
Accepts two peaks and returns which one is considered the largest.

Used in combination with a paired index, which can be used to track
the peak as it moves through various sortings and std::vectors.
*/
inline bool IndexedSigSort(const std::pair<int, Peak> &p1,
                           const std::pair<int, Peak> &p2) {
    return (p1.second.mHeight > p2.second.mHeight);
}

// #define DBGPK 1

// mesh is regular array with data at each point
class Mesh {
public:
    enum { NOUTOFCORE = 40 };
    char fname[1024];  // this is source name when READING - so could be xml or
                       // mesh
    char namestem[1024];
    char meshname[1024];  // this is the MESH name - always mesh
    char histoname[1024];
    char datname[1024];
    char headername[1024];
    char indexname[1024];
    FILE *file;  // this is mesh file whether read or write
    FILE *delta;
    double dsum;
    int nshiftedout;
    double y1outofcore;
    std::unique_ptr<float[]> v;  // TODO: Adapt pointers to unique_ptr's
    std::unique_ptr<int[]> hit;
    float *weight;
    unsigned short *count;  // does this put a limit on npeaks?
    double splatfactor;
    double normfactor;
    double vmin;
    double vmax;
    double vmean;
    int donormalize;
    std::vector<Peak> peaks;
    std::vector<Peak *> indexed_peaks;
    Bound xbound;
    Bound ybound;
    double unitweight;
    std::vector<double> align;
    TimeMap timemap;
    float rawxmin, rawxmax, rawymin, rawymax, rawmeandx, rawmeandy;
    ConversionSpecs mConversion;
    enum { NHISTO = 1000 };
    int histo[NHISTO];
    const float minhisto = 1;
    const float maxhisto = 1E9;

#ifdef DBGPK
    float tx;
    float ty;
    float eps;
#endif

    int getHistoBin(const float v) const {
        if (v <= minhisto) return 0;
        int bin = (log(v) - log(minhisto)) / (log(maxhisto) - log(minhisto)) * NHISTO;
        if (bin < 0) 
			return 0;
        if (bin >= NHISTO)
			return NHISTO - 1;
        return bin;
    }

    float getHistoVal(const int bin) const {
        float v = exp(float(bin) / NHISTO * (log(maxhisto) - log(minhisto)) +
                      log(minhisto));
        if (v < minhisto) 
			return minhisto;
        if (v > maxhisto) 
			return maxhisto;
        return v;
    }

    void clearHisto() {
        for (int i = 0; i < NHISTO; i++) 
			histo[i] = 0;
    }

    void addHisto(const float v) { 
		histo[getHistoBin(v)]++;
	}

    Mesh() { 
		clearHisto(); 
	}

    // main constructor for loading line mode ascii data
    // init names and allocate memory but do not load
    // nx, ny are determined from ranges and deltas
    Mesh(double x1, double y1, double x2, double y2, double Dx, double Dy,
         const char *Fname) {
        initSimple(x1, y1, x2, y2, Dx, Dy, Fname);
    }

    inline double ycoordoutofcore(const int j) const {
        return y1outofcore + j * mConversion.mDRT;
    }

    // This now uses y-fastest
    inline int Index(const int i, const int j) const {
        return j * mConversion.mNMZ + i;
    }

    inline int UpLine() const { return -mConversion.mNMZ; }

    inline int DownLine() const { return +mConversion.mNMZ; }

    void init(double x1, double y1, double x2, double y2, double Dx, double Dy,
              const std::string &input_path, const std::string &output_path,
              bool ForRead) {
        mConversion.mMinMZ = x1;
        mConversion.mMinRT = y1;
        mConversion.mMaxMZ = x2;
        mConversion.mMaxRT = y2;
        mConversion.mDMZ = Dx;
        mConversion.mDRT = Dy;
        // mConversion.mWarpedMesh = 0;
        normfactor = 1;
        donormalize = 0;
        unitweight = 0;
        // warped = 0;
        mConversion.mNMZ =
            (mConversion.mMaxMZ - mConversion.mMinMZ) / Dx + 0.5 + 1;
        mConversion.mNRT =
            (mConversion.mMaxRT - mConversion.mMinRT) / Dy + 0.5 + 1;
        initNames(input_path, output_path);
        clearHisto();
        init(ForRead);
    }

    // for warped mesh, need to map everything to warped grid
    void initUsingConversion(const std::string &input_path,
                             const std::string &output_path, bool ForRead) {
        init(mConversion.WorldToMeshX(mConversion.mMinMZ), mConversion.mMinRT,
             mConversion.WorldToMeshX(mConversion.mMaxMZ), mConversion.mMaxRT,
             mConversion.mMeanDMZ, mConversion.mMeanDRT, input_path,
             output_path, ForRead);
    }

    void initSimple(double x1, double y1, double x2, double y2, double Dx,
                    double Dy, const char *Fname, const char *mfile = "") {
        char mfname[1024];
        mConversion.mMinMZ = x1;
        mConversion.mMinRT = y1;
        mConversion.mMaxMZ = x2;
        mConversion.mMaxRT = y2;
        mConversion.mDMZ = Dx;
        mConversion.mDRT = Dy;
        mConversion.mNMZ =
            (mConversion.mMaxMZ - mConversion.mMinMZ) / Dx + 0.5 + 1;
        mConversion.mNRT =
            (mConversion.mMaxRT - mConversion.mMinRT) / Dy + 0.5 + 1;
        mConversion.mWarpedMesh = 0;

        strcpy(fname, Fname);
        strcpy(mfname, mfile);
        init(mfname);
    }

    void loadAlign(const char *name) {
        FILE *f = fopen(name, "r");
        while (!feof(f)) {
            double v;
            if (1 == fscanf(f, "%lf", &v)) align.push_back(v);
        }
        fclose(f);
    }

    // here Header refers to the .mesh file
    void loadHeader(char *name) {
        strcpy(fname, name);
        strcpy(fname, name);
        strcpy(meshname, name);
        strcpy(namestem, name);
        char *p = strrchr(namestem, '.');
        *p = '\0';

        mConversion.Load(meshname, datname);
    }

    void copyValues(Mesh *m) {
        mConversion.mNMZ = m->mConversion.mNMZ;
        mConversion.mNRT = m->mConversion.mNRT;
        mConversion.mDMZ = m->mConversion.mDMZ;
        mConversion.mDRT = m->mConversion.mDRT;
        mConversion.mSigmaMZ = m->mConversion.mSigmaMZ;
        mConversion.mSigmaRT = m->mConversion.mSigmaRT;
        mConversion.mMinMZ = m->mConversion.mMinMZ;
        mConversion.mMinRT = m->mConversion.mMinRT;
        mConversion.mMaxMZ = m->mConversion.mMaxMZ;
        mConversion.mMaxRT = m->mConversion.mMaxRT;
        normfactor = m->normfactor;
        donormalize = m->donormalize;
        unitweight = m->unitweight;
    }

    // sigmas are for splatting but also for peak centroiding
    // determine total weight of a unit height splat in center of mesh
    void setSigmas() {
        splatfactor = 1.0;  // maintain peak height

        Data2D p;
        p.v = 1.0;
        p.p.x = mConversion.IndexToWorldX(
            mConversion.mNMZ /
            2);  // grab middle of x range in world coordinate
        p.p.y = NOUTOFCORE / 2 * mConversion.mDRT;
        y1outofcore = 0;
        // splat single point into center of mesh
        Splat(p);
        unitweight = 0;
        // run through and find total weight, zeroing as you go
        for (int j = 0; j < NOUTOFCORE; j++) {
            int index = j * mConversion.mNMZ;
            for (int i = 0; i < mConversion.mNMZ; i++) {
                unitweight += v[index];
                v[index] = 0;
                index++;
            }
        }
    }

    // mname refers to the input or output name of the MESH
    // sourcename refers to the original data source, which may be xml
    // note that all generated file names are LOCAL
    // but sourcename becomes fname - which is unchanged
    // generate the others
    void initNames(const std::string &input_path,
                   const std::string &output_path) {
        strcpy(fname, input_path.c_str());
        strcpy(namestem, output_path.c_str());

        sprintf(histoname, "%s.histo", namestem);
        sprintf(meshname, "%s.mesh", namestem);
        sprintf(datname, "%s.dat", namestem);
        sprintf(headername, "%s.hdr", namestem);
        sprintf(indexname, "%s.inx", namestem);
    }

    // mname refers to the input or output name of the MESH
    // sourcename refers to the original data source, which may be xml
    // note that all generated file names are LOCAL
    // but sourcename becomes fname - which is unchanged
    // generate the others
    void initNames(const char *mname, const char *sourcename = "") {
        strcpy(fname, sourcename);
        strcpy(namestem, mname);

        // output locally even when input is remote
        if (const char *p = strrchr(mname, '\\'))
            strcpy(namestem, p + 1);
        else if (const char *p = strrchr(mname, '/'))
            strcpy(namestem, p + 1);
        int n = strlen(namestem) - 1;
        for (int i = n; i >= 0; i--) {
            if (namestem[i] == '.') {
                namestem[i] = '\0';
                break;
            }
        }

        for (int i = 0; i < strlen(namestem); i++)
            if (isspace(namestem[i])) namestem[i] = '_';

        sprintf(histoname, "%s.histo", namestem);
        sprintf(meshname, "%s.mesh", namestem);
        sprintf(datname, "%s.dat", namestem);
        sprintf(headername, "%s.hdr", namestem);

        strcpy(indexname, sourcename);
        n = strlen(indexname) - 1;
        if (n > 0) {
            for (int i = n; i >= 0; i--) {
                if (indexname[i] == '.') {
                    indexname[i] = '\0';
                    break;
                }
            }
            strcat(indexname, ".inx");
        } else {
            strcpy(indexname, "noindex.inx");
        }
    }

    void allocate() {
        v.reset(FSUtil::ArrayAllocation<float>(
            mConversion.mNMZ * mConversion.mNRT, "v in mesh allocate"));
        FSUtil::CheckAlloc(weight, mConversion.mNMZ * mConversion.mNRT,
                           "weight in mesh allocate");
        FSUtil::CheckAlloc(count, mConversion.mNMZ * mConversion.mNRT,
                           "count in mesh allocate");
        memset(weight, 0, mConversion.mNMZ * mConversion.mNRT * sizeof(float));
        memset(count, 0,
               mConversion.mNMZ * mConversion.mNRT * sizeof(unsigned short));

        hit.reset(FSUtil::ArrayAllocation<int>(
            mConversion.mNMZ * mConversion.mNRT, "hit in mesh allocate"));
        memset(v.get(), 0, mConversion.mNMZ * mConversion.mNRT * sizeof(float));
        memset(hit.get(), 0, mConversion.mNMZ * mConversion.mNRT * sizeof(int));

        dsum = 0;
        donormalize = 0;
        xbound.SetMax(mConversion.mNMZ);
        ybound.SetMax(mConversion.mNRT);
    }

    void deallocate() {
        delete[] weight;
        delete[] count;
    }

    // assumes mesh has been predefined in dimensions
    void init(char *mfname) {
        v.reset(FSUtil::ArrayAllocation<float>(
            mConversion.mNMZ * mConversion.mNRT, "v in mesh fname init"));
        FSUtil::CheckAlloc(weight, mConversion.mNMZ * mConversion.mNRT,
                           "weight in mesh fname init");
        FSUtil::CheckAlloc(count, mConversion.mNMZ * mConversion.mNRT,
                           "count in mesh fname init");
        hit.reset(FSUtil::ArrayAllocation<int>(
            mConversion.mNMZ * mConversion.mNRT, "hit in mesh fname init"));

        for (int i = 0; i < mConversion.mNMZ * mConversion.mNRT; i++) {
            v[i] = 0;
            weight[i] = 0;
            count[i] = 0;
            hit[i] = 0;
        }
        dsum = 0;
        file = fopen(fname, "r");
        if (!file) {
            std::cerr << "Error opening file: " << fname << std::endl;
            exit(-1);
        }
        initNames(mfname, "");
        donormalize = 0;
        xbound.SetMax(mConversion.mNMZ);
        ybound.SetMax(mConversion.mNRT);
        peaks.reserve(5000000);
        clearHisto();
    }

    void initOutOfCore() {
        v.reset(FSUtil::ArrayAllocation<float>(mConversion.mNMZ * NOUTOFCORE,
                                               "v in mesh bool init"));
        FSUtil::CheckAlloc(weight, mConversion.mNMZ * NOUTOFCORE,
                           "weight in mesh bool init");
        FSUtil::CheckAlloc(count, mConversion.mNMZ * NOUTOFCORE,
                           "count in mesh bool init");
        hit.reset(FSUtil::ArrayAllocation<int>(mConversion.mNMZ * NOUTOFCORE,
                                               "hit in mesh bool init"));

        for (int i = 0; i < mConversion.mNMZ * NOUTOFCORE; i++) {
            v[i] = 0;
            weight[i] = 0;
            count[i] = 0;
            hit[i] = 0;
        }

        file = fopen(datname, "wb");
        if (!file) {
            std::cerr << "Error opening file: " << meshname << std::endl;
            exit(-1);
        }
        clearHisto();
        donormalize = 0;
        xbound.SetMax(mConversion.mNMZ);
        ybound.SetMax(mConversion.mNRT);
    }

    void init(bool ForRead = true) {
        if (!ForRead) {
            initOutOfCore();
            return;
        }
        v.reset(FSUtil::ArrayAllocation<float>(
            mConversion.mNMZ * mConversion.mNRT, "v in mesh bool init"));
        FSUtil::CheckAlloc(weight, mConversion.mNMZ * mConversion.mNRT,
                           "weight in mesh bool init");
        FSUtil::CheckAlloc(count, mConversion.mNMZ * mConversion.mNRT,
                           "count in mesh bool init");
        hit.reset(FSUtil::ArrayAllocation<int>(
            mConversion.mNMZ * mConversion.mNRT, "hit in mesh bool init"));
        clearHisto();

        for (int i = 0; i < mConversion.mNMZ * mConversion.mNRT; i++) {
            v[i] = 0;
            weight[i] = 0;
            count[i] = 0;
            hit[i] = 0;
        }

        dsum = 0;
        if (ForRead)
            file = fopen(meshname, "r");
        else
            file = fopen(meshname, "w");
        if (!file) {
            std::cerr << "Error opening file: " << meshname << std::endl;
            exit(-1);
        }
        donormalize = 0;
        xbound.SetMax(mConversion.mNMZ);
        ybound.SetMax(mConversion.mNRT);
    }

    // splat data to single mesh point
    // d has mesh coordinates
    // i and j reference the shifted mesh
    // localxsig is in mesh units
    inline void SplatToMesh(const int i, const int j, const Data2D &d,
                            const double localxsig) {
        if (i < 0 || i >= mConversion.mNMZ || j < 0 || j >= NOUTOFCORE) return;

        int index = Index(i, j);

        // need to find gaussian weightings based on sigma distance from this
        // grid point to the data point
        double a, b;
        a = (d.p.x - mConversion.IndexToMeshX(i)) / localxsig;
        b = (d.p.y - ycoordoutofcore(j)) / mConversion.mSigmaRT;
        double rsq = a * a + b * b;
        double w = splatfactor * exp(-0.5 * rsq);

        if (w > 0) {
            v[index] += w * d.v;
            weight[index] += w;
            count[index]++;
        }
    }

    inline double GetSignal(double x, double y, double wx, double wy) {
        int i1 = (x - wx - mConversion.mMinMZ) / mConversion.mDMZ + 0.5;
        xbound.Clamp(i1);
        int j1 = (y - wy - mConversion.mMinRT) / mConversion.mDRT + 0.5;
        ybound.Clamp(j1);
        int i2 = (x + wx - mConversion.mMinMZ) / mConversion.mDMZ + 0.5;
        xbound.Clamp(i2);
        int j2 = (y + wy - mConversion.mMinRT) / mConversion.mDRT + 0.5;
        ybound.Clamp(j2);

        double sum = 0;
        for (int i = i1; i <= i2; i++)
            for (int j = j1; j <= j2; j++) sum += v[Index(i, j)];

        return sum;
    }

    // load regular mesh into mesh object
    // should allow both ascii and mesh data
    void loadFromFile(const char *Fname) {
        if (strstr(Fname, ".hdr")) {
            return;
        }

        FILE *f = fopen(Fname, "r");
        if (!f) {
            std::cout << "Error - cannot open file " << Fname << std::endl;
            exit(-1);
        }

        strcpy(fname, Fname);
        strcpy(meshname, Fname);
        strcpy(namestem, Fname);

        char *p = strrchr(namestem, '.');
        *p = '\0';

        mConversion.Load(meshname, datname);

        splatfactor = 1.0;

        v.reset(
            FSUtil::ArrayAllocation<float>(mConversion.mNMZ * mConversion.mNRT,
                                           "allocating v floats in mesh init"));
        hit.reset(
            FSUtil::ArrayAllocation<int>(mConversion.mNMZ * mConversion.mNRT,
                                         "allocating hit floats in mesh init"));

        for (int i = 0; i < mConversion.mNMZ * mConversion.mNRT; i++) {
            v[i] = 0;
            hit[i] = 0;
        }

        f = fopen(datname, "rb");

        for (int i = 0; i < mConversion.mNMZ * mConversion.mNRT; i++) {
            float vv;
            fread(&vv, sizeof(float), 1, f);
            Swap::MakeFloat32(vv, mConversion.mMeshLittleEndian);
            v[i] = vv;
        }

        fclose(f);
        xbound.SetMax(mConversion.mNMZ);
        ybound.SetMax(mConversion.mNRT);

#ifdef DBGPK
        tx = 736.004;
        ty = 31.774;
        eps = 3.0;
#endif
    }

    // this is the expansion needed to rescale sigma from index space to world
    // space
    inline double ExpansionAtIndex(const double x) const {
        double xorig = mConversion.IndexToWorldX(x);
        double xorig2 = mConversion.IndexToWorldX(x + 1);  // check this
        return fabs(xorig2 - xorig);
    }

    // this assumes the expansion factor is one at ConversionMassAtSigma
    inline double WorldSigmaFromIndex(const double x) const {
        return ExpansionAtIndex(x) * mConversion.mSigmaMZ;
    }

    // Used by loadXML
    // splat from mesh coordinates into the grid
    inline void Splat(const Data2D &d) {
        // d.x, y are in mesh, rt space coming in
        double localxsig = SigmaAtMeshInMeshUnits(d.p.x);

        int wx = localxsig / mConversion.mDMZ * 2;
        int wy = mConversion.mSigmaRT / mConversion.mDRT * 2;
        int i = mConversion.MeshToIndexX(d.p.x) + 0.5;
        int j = (d.p.y - y1outofcore) / mConversion.mDRT +
                0.5;  // j references into the shifted mesh
        i -= wx;
        j -= wy;

        for (int b = j; b <= j + 2 * wy; b++)
            for (int a = i; a <= i + 2 * wx; a++)
                SplatToMesh(a, b, d, localxsig);
    }

    // check every time we load a new rt
    // it is about to begin splatting at this value of rt
    // if it is a line beyond the middle of the outofcore mesh then shift out
    // until it is in the middle
    void shiftMesh(const float rt) {
        y1outofcore = mConversion.mMinRT + nshiftedout * mConversion.mDRT;

        // should shift out until incoming rt lands in middle of mesh
        // as you shift out the central rt will increase until it is greater
        // than the incoming rt
        double yshiftlimit =
            y1outofcore + (NOUTOFCORE / 2 + 1) * mConversion.mDRT;
        int shiftcount = 0;
        while (rt > yshiftlimit && nshiftedout < mConversion.mNRT) {
            shiftcount++;

            // don't output initial empty lines
            if (nshiftedout >= 0)
                fwrite(v.get(), sizeof(float), mConversion.mNMZ, file);

            int ll = mConversion.mNMZ * sizeof(float);
            int ls = mConversion.mNMZ * sizeof(unsigned short);

            // go from low row to high copying upper row to lower
            for (int i = 0; i < NOUTOFCORE - 1; i++) {
                memcpy(v.get() + i * mConversion.mNMZ,
                       v.get() + (i + 1) * mConversion.mNMZ, ll);
                memcpy(weight + i * mConversion.mNMZ,
                       weight + (i + 1) * mConversion.mNMZ, ll);
                memcpy(count + i * mConversion.mNMZ,
                       count + (i + 1) * mConversion.mNMZ, ls);
            }
            {
                int i = mConversion.mNMZ * (NOUTOFCORE - 1);
                for (int j = 0; j < mConversion.mNMZ; j++) {
                    v[i + j] = 0;
                    weight[i + j] = 0;
                    count[i + j] = 0;
                }
            }
            nshiftedout++;
            y1outofcore = mConversion.mMinRT + nshiftedout * mConversion.mDRT;
            yshiftlimit = y1outofcore + (NOUTOFCORE / 2 + 1) * mConversion.mDRT;
        }
    }

    // standalone routine called to build index file
    void buildIndex(const char *fstem) {
        enum { BUFFSIZE = 10000000 };
        char *buff;

        // names are created here so the index build is standalone
        char xmlname[1024];

        sprintf(xmlname, "%s.mzXML", fstem);
        sprintf(indexname, "%s.inx", fstem);

        std::cout << "Building index file " << indexname << std::endl;

        std::fstream xml(xmlname);

        if (!xml.is_open()) {
            std::cerr << "File " << xmlname << " not opened.  Terminating."
                      << std::endl;
            exit(-1);
        }

        FILE *inx = fopen(indexname, "w");
        if (!inx) {
            std::cerr << "Cound not open index file " << indexname
                      << " .  Terminating." << std::endl;
            exit(-1);
        }

        std::streampos fptr;

        FSUtil::CheckAlloc(buff, BUFFSIZE, "buff in mesh loadXML");

        *buff = 0;

        while (!strstr(buff, "<msRun ")) {
            xml.read(buff, BUFFSIZE);
        }

        int scancount = 0;
        char *p;

        p = strstr(buff, "scanCount=");
        p += strlen("scanCount=");

        int nread = sscanf(p, "\"%d\"", &scancount);
        if (nread != 1) {
            std::cerr << "Error reading msRun, scancount" << std::endl;
            exit(-1);
        }

        if (scancount < 2) {
            std::cerr << "Quitting due to anomalous scancount" << std::endl;
            exit(-1);
        }

        int nscan = 0;
        while (!xml.eof() && nscan < scancount) {
            while (!strstr(buff, "<scan num") && !xml.eof()) {
                fptr = xml.tellg();
                xml.read(buff, BUFFSIZE);
            }
            if (xml.eof()) {
                break;
            }
            int peaksCount = 0;
            int scannum = 0;
            int mslevel = 0;
            float rt;
            p = strstr(buff, "<scan num");
            if (p == nullptr) {
                std::cout << "Error looking for <scan num" << std::endl;
                break;
            }
            p = strstr(buff, "=");
            p++;
            nread = sscanf(p, "\"%d\"", &scannum);
            if (nread != 1) {
                std::cerr << "Error reading scannum at nscan " << nscan
                          << std::endl;
                exit(-1);
            }
            if (!strstr(buff, "msLevel")) {
                while (!strstr(buff, "msLevel") && !xml.eof())
                    xml.read(buff, BUFFSIZE);
                if (xml.eof()) {
                    break;
                }
            }
            p = strstr(buff, "msLevel");
            if (p == nullptr) {
                std::cout << "Error looking for msLevel" << std::endl;
            }
            p = strstr(p, "=");
            p++;
            nread = sscanf(p, "\"%i\"", &mslevel);

            if (mslevel == 1) {
                if (!strstr(buff, "peaksCount"))
                    while (!strstr(buff, "peaksCount") && !xml.eof())
                        xml.read(buff, BUFFSIZE);
                if (xml.eof()) {
                    break;
                }
                p = strstr(buff, "peaksCount");
                if (p == nullptr) {
                    std::cout << "Error looking for peakscount" << std::endl;
                    break;
                }
                p = strstr(p, "=");
                p++;
                nread = sscanf(p, "\"%d\"", &peaksCount);
                if (nread != 1) {
                    std::cerr << "Error reading peaksCount at nscan " << nscan
                              << std::endl;
                    exit(-1);
                }
                if (!strstr(buff, "retentionTime"))
                    while (!strstr(buff, "retentionTime") && !xml.eof())
                        xml.read(buff, BUFFSIZE);
                if (xml.eof()) {
                    std::cerr << "Error reading retentionTime at nscan "
                              << nscan << std::endl;
                    exit(-1);
                }

                p = strstr(buff, "retentionTime");
                p = strstr(p, "=");
                p++;
                p = strstr(p, "PT");
                p += 2;
                nread = sscanf(p, "%fS", &rt);
                if (nread != 1) {
                    std::cerr << "Error reading retentionTime at nscan "
                              << nscan << std::endl;
                    exit(-1);
                }

                // now have fptr and retention time
                // originally fptr was long long and used with ftelli64 - but
                // now using fstream and tellg I hope it is still long long
                //    or else it will fail with large files.
                fprintf(inx, "%lld %f %d %d\n", fptr, rt, scannum, peaksCount);
                std::cout << fptr << " " << rt << " " << scannum << " "
                          << peaksCount << std::endl;
            }
        }

        xml.close();
        fclose(inx);

        std::cout << "Index file complete" << std::endl;

        delete[] buff;
        buff = NULL;
    }

    inline double SigmaAtMass(const double x) const {
        double sig = 0;
        switch (mConversion.mMassSpecType) {
            case ConversionSpecs::ORBITRAP:
                sig =
                    mConversion.mSigmaMZ * pow(x / mConversion.mMZAtSigma, 1.5);
                break;
            case ConversionSpecs::FTICR:
                sig = mConversion.mSigmaMZ * pow(x / mConversion.mMZAtSigma, 2);
            case ConversionSpecs::QTOF:
            case ConversionSpecs::TOF:
                sig = mConversion.mSigmaMZ * (x / mConversion.mMZAtSigma);
                break;
            case ConversionSpecs::QUAD:
            case ConversionSpecs::IONTRAP:
                sig = mConversion.mSigmaMZ;
                break;
            default:
                std::cerr << "Mass Spec Type not recognized" << std::endl;
                exit(-1);
        }
        return sig;
    }

    inline double SigmaAtMeshInMeshUnits(const double x) const {
        double wx = mConversion.MeshToWorldX(x);
        double s = SigmaAtMass(wx);
        return mConversion.WorldToMeshX(wx + s) - mConversion.WorldToMeshX(wx);
    }

    void loadXML(const char *xname, bool dump) {
#ifdef DBG_GRID
        std::cerr << "In loadXML" << std::endl;
#endif
        enum { BUFFSIZE = 10000000 };
        char *buff;
        Data2D d;
        vmin = 1.0E10;
        vmax = -1.0E10;
        int count = 0;
        int precision;
        nshiftedout = -NOUTOFCORE / 2;
        y1outofcore = mConversion.mMinRT + nshiftedout * mConversion.mDRT;

        int dxcount = 0;
        int dycount = 0;
        double dxsum = 0;
        double dysum = 0;
        float prevx = -1;
        float prevy = -1;
        float Dx, Dy;

        rawxmin = 1E6;
        rawxmax = -1E6;
        rawymin = rawxmin;
        rawymax = rawxmax;

        std::ifstream xml(xname);
        if (!xml.is_open()) {
            std::cerr << "File " << xname << " not opened.  Terminating."
                      << std::endl;
            exit(-1);
        }

        FILE *dumpfile = NULL;
        if (dump) {
            dumpfile = fopen("meshdump.txt", "w");
            if (!dumpfile) {
                std::cerr << "Error opening meshdump.txt" << std::endl;
                exit(-1);
            }
        }

        FSUtil::CheckAlloc(buff, BUFFSIZE, "buff in mesh loadXML");

        bool usealign = timemap.getSize() > 0;

        char ticname[512];
        sprintf(ticname, "%s.tic", namestem);
        FILE *tic = fopen(ticname, "w");

#ifdef DBG_GRID
        std::cerr << "Seeking <msRun" << std::endl;
#endif
        strcpy(buff, "");
        while (!strstr(buff, "<msRun ")) {
            xml.getline(buff, BUFFSIZE);
        }
#ifdef DBG_GRID
        std::cerr << "Found <msRun" << std::endl;
#endif
        int scancount = 0;
        char *p;
        p = strstr(buff, "scanCount=");
        p += strlen("scanCount=");
        int nread = sscanf(p, "\"%d\"", &scancount);
        if (nread != 1) {
            std::cerr << "Error reading msRun, scancount" << std::endl;
            exit(-1);
        }

        double timeconversion = 1.0;
        timeconversion = mConversion.mRTReduction;

        if (scancount < 2) {
            std::cerr << "Quitting due to anomalous scancount" << std::endl;
            exit(-1);
        }

        IndexFile inx;
        inx.load(indexname);
        if (!inx.isEmpty()) {
            std::streampos offset =
                inx.getOffset(mConversion.mMinRT * timeconversion);
            if (offset >= 0) 
				xml.seekg(offset);
        }

        int nscan = 0;
        double ic;
        while (!xml.eof() && nscan < scancount) {
            while (!strstr(buff, "<scan num") && !xml.eof())
                xml.getline(buff, BUFFSIZE);
            if (xml.eof()) {
                std::cerr << "Error finding scan num in xml file at nscan = "
                          << nscan << " expecting nscans = " << scancount
                          << std::endl;
                exit(-1);
            }
            int peaksCount = 0;
            int scannum = 0;
            int mslevel = 0;
            float rt;
            p = strstr(buff, "<scan num");
            p = strstr(buff, "=");
            p++;
            nread = sscanf(p, "\"%d\"", &scannum);
            if (nread != 1) {
                std::cerr << "Error reading scannum at nscan " << nscan
                          << std::endl;
                exit(-1);
            }
            if (!strstr(buff, "msLevel")) {
                while (!strstr(buff, "msLevel") && !xml.eof())
                    xml.getline(buff, BUFFSIZE);
                if (xml.eof()) {
                    std::cerr << "Error finding msLevel at nscan " << nscan
                              << std::endl;
                    exit(-1);
                }
            }
            p = strstr(buff, "msLevel");
            p = strstr(p, "=");
            p++;
            nread = sscanf(p, "\"%i\"", &mslevel);

            if (mslevel == 1) {
                if (!strstr(buff, "peaksCount"))
                    while (!strstr(buff, "peaksCount") && !xml.eof())
                        xml.getline(buff, BUFFSIZE);
                if (xml.eof()) {
                    std::cerr << "Error reading peaksCount at nscan " << nscan
                              << std::endl;
                    exit(-1);
                }
                p = strstr(buff, "peaksCount");
                p = strstr(p, "=");
                p++;
                nread = sscanf(p, "\"%d\"", &peaksCount);
                if (nread != 1) {
                    std::cerr << "Error reading peaksCount at nscan " << nscan
                              << std::endl;
                    exit(-1);
                }
                if (!strstr(buff, "retentionTime"))
                    while (!strstr(buff, "retentionTime") && !xml.eof())
                        xml.getline(buff, BUFFSIZE);
                if (xml.eof()) {
                    std::cerr << "Error reading retentionTime at nscan "
                              << nscan << std::endl;
                    exit(-1);
                }
                p = strstr(buff, "retentionTime");
                p = strstr(p, "=");
                p++;
                p = strstr(p, "PT");
                p += 2;
                nread = sscanf(p, "%fS", &rt);
                if (nread != 1) {
                    std::cerr << "Error reading retentionTime at nscan "
                              << nscan << std::endl;
                    exit(-1);
                }

                nscan++;
                rt /= timeconversion;

                // rt not in region yet
                if (rt < mConversion.mMinRT) {
                    xml.getline(buff, BUFFSIZE);
                    continue;
                }

                // rt out of region so quit
                if (rt > mConversion.mMaxRT) {
                    std::cout << "high rt " << rt << " " << mConversion.mMaxRT
                              << " shifting remainder of mesh" << std::endl;
                    shiftMesh(1E6);

                    break;
                }
                while (!strstr(buff, "<peaks ") && !xml.eof())
                    xml.getline(buff, BUFFSIZE);
                while (!strstr(buff, "precision") && !xml.eof())
                    xml.getline(buff, BUFFSIZE);
                if (xml.eof()) {
                    std::cerr << "Error reading peaks at nscan " << nscan
                              << std::endl;
                    exit(-1);
                }
                p = strstr(buff, "precision");
                p = strstr(p, "=");
                p++;
                nread = sscanf(p, "\"%i\"", &precision);
                if (!strstr(buff, "m/z-int"))
                    while (!strstr(buff, "m/z-int") && !xml.eof())
                        xml.getline(buff, BUFFSIZE);
                if (xml.eof()) {
                    std::cerr << "Error reading m/z-int at nscan " << nscan
                              << std::endl;
                    exit(-1);
                }

                char *ptr = (char *)strstr(buff, ">");
                ptr++;
                d.p.y = rt;
                if (d.p.y < rawymin) rawymin = d.p.y;
                if (d.p.y > rawymax) rawymax = d.p.y;
                if (prevy > 0) {
                    Dy = d.p.y - prevy;
                    dysum += fabs(Dy);
                    dycount++;
                }
                prevy = d.p.y;
                if (usealign) d.p.y = timemap.lookUp(d.p.y);
                int bit = 0;
                ic = 0;
                prevx = -1;

                int littleendian = 1;
                if (this->mConversion.mInvertXMLEndian)
                    littleendian = 1 - littleendian;

                float localmin = 1E10;
                float localmax = -localmin;
                double localsum = 0;
                int localcount = 0;
                double localav = 0;

                shiftMesh(rt);

                std::cout << "rt, npeaks: " << rt << " " << peaksCount
                          << std::endl;

                // now splat all points in the mass range into the mesh
                for (int i = 0; i < peaksCount; i++) {
                    double m;
                    double intens;
                    Encoding::getFloatFloat(
                        ptr, bit, m, intens, precision,
                        littleendian);  // forcing non littleendian
                    m = mConversion.WorldToMeshX(m);  // mesh units
                    if (m < mConversion.mMinMZ || m > mConversion.mMaxMZ) {
                        continue;
                    }
                    d.p.x = m;
                    d.v = intens;
                    if (d.v > 0) {
                        Splat(d);
                        addHisto(intens);
                        if (dump && d.p.x >= mConversion.mMinMZ &&
                            d.p.x <= mConversion.mMaxMZ) {
                            fprintf(dumpfile, "%f %f %f\n", d.p.x, d.p.y, d.v);
                        }
                        if (true) {
                            localsum += intens;
                            localcount++;
                            localav = localsum / localcount;
                            if (localmin > intens) localmin = intens;
                            if (localmax < intens) localmax = intens;
                        }
                    }
                    if (d.v < 0) {
                        std::cout << "negative intensity: m, i = " << m << " "
                                  << intens << std::endl;
                    }

                    dsum += d.v;
                    ic += d.v;
                    count++;
                    if (vmin > d.v) vmin = d.v;
                    if (vmax < d.v) vmax = d.v;
                    if (d.p.x < rawxmin) rawxmin = d.p.x;
                    if (d.p.x > rawxmax) rawxmax = d.p.x;
                    if (prevx >= 0) {
                        Dx = d.p.x - prevx;
                        dxsum += fabs(Dx);
                        dxcount++;
                    }
                    prevx = d.p.x;
                }
                fprintf(tic, "%lf %lf\n", d.p.y, ic);
                if (nscan % 200 == 0)
                    std::cout << nscan << " " << d.p.y << " " << ic
                              << std::endl;
            } else {
                nscan++;
            }
        }

        fclose(tic);
        xml.close();
        if (dump && dumpfile) fclose(dumpfile);
        vmean = dsum / count;

        rawmeandx = dxsum / dxcount;
        rawmeandy = dysum / dycount;

        fclose(file);

        delete[] buff;
        buff = NULL;
    }

    // normalize mesh points based on accumulated weights
    // This will boost sparse areas, and reduce dense areas
    void weightMesh() {
        for (int i = 0; i < mConversion.mNMZ * mConversion.mNRT; i++) {
            if (count[i] > 0 && weight[i] > 1E-20)
                v[i] /= weight[i];
            else
                v[i] = 0;
        }
    }

    void dumpHisto() {
        FILE *f = fopen(histoname, "w");
        fprintf(f, "# %d %f %f\n", NHISTO, minhisto, maxhisto);

        for (int i = 0; i < NHISTO; i++)
            fprintf(f, "%f %d\n", getHistoVal(i), histo[i]);

        fclose(f);
    }

    void normalize() {
        double sum = 0;
        for (int i = 0; i < mConversion.mNMZ * mConversion.mNRT; i++)
            sum += v[i];
        normfactor = dsum / sum;
        if (donormalize) {
            for (int i = 0; i < mConversion.mNMZ * mConversion.mNRT; i++)
                v[i] *= normfactor;
        }
    }

    void normalize(float factor) {
        for (int i = 0; i < mConversion.mNMZ * mConversion.mNRT; i++)
            v[i] *= factor;
        vmax *= factor;
        vmin *= factor;
        vmean *= factor;
    }

    // just check if local max
    inline int IsPeak(const int i, const int j) {
        int p = Index(i, j);
        double z = v[p];
        if (z == 0) return 0;
        if (v[p - 1] > z) return 0;
        if (v[p + 1] > z) return 0;
        if (v[p + UpLine()] > z) return 0;
        if (v[p + DownLine()] > z) return 0;
        if (v[p - 1 + UpLine()] > z) return 0;
        if (v[p - 1 + DownLine()] > z) return 0;
        if (v[p + 1 + UpLine()] > z) return 0;
        if (v[p + 1 + DownLine()] > z) return 0;
        return 1;
    }

    // This should return 1 if there are no downward paths from here
    // So, return 0 if there is a downward path available
    // A value near zero is assumed to be a local minimum
    inline int GoingUp(const int i, const int j, const double thresh,
                       const int id) {
        int n = Index(i, j);
        double vv = v[n];

        // this is an assumed background limit
        // if you're this low, you must be at edge and upward tending
        if (vv < 4) return 1;

        // if there is an unclaimed, downward path out, then not going up
        if ((hit[n + 1] != id) && (vv >= v[n + 1])) return 0;
        if ((hit[n - 1] != id) && (vv >= v[n - 1])) return 0;
        if ((hit[n + UpLine()] != id) && (vv >= v[n + UpLine()])) return 0;
        if ((hit[n + DownLine()] != id) && (vv >= v[n + DownLine()])) return 0;

        return 1;
    }

    inline void MarkAsBoundary(const int i, const int j, const int id,
                               double &bordersum, int &borderhits, bool show) {
        int n = Index(i, j);
        if (hit[n + 1] != id && hit[n + 1] != -id) {
            hit[n + 1] = -id;
            bordersum += v[n + 1];
            borderhits++;
        }
        if (hit[n - 1] != id && hit[n - 1] != -id) {
            hit[n - 1] = -id;
            bordersum += v[n - 1];
            borderhits++;
        }
        if (hit[n + UpLine()] != id && hit[n + UpLine()] != -id) {
            hit[n + UpLine()] = -id;
            bordersum += v[n + UpLine()];
            borderhits++;
        }
        if (hit[n + DownLine()] != id && hit[n + DownLine()] != -id) {
            hit[n + DownLine()] = -id;
            bordersum += v[n + DownLine()];
            borderhits++;
        }
    }

    // Starting from peak, search downhill, stopping only at uphill or other
    // peak.  Add to stats. Mark with id's as you go prev is -1 on first call id
    // is >=1
    void ExplorePeakSlope(const int id, const int i, const int j, double thresh,
                          double prev, double &xsum, double &ysum, double &xsig,
                          double &ysig, double &vsum, int &nhits,
                          double &bordersum, int &borderhits) {
        // if at edge, leave
        if (i < 1 || i >= mConversion.mNMZ - 1 || j < 1 ||
            j >= mConversion.mNRT - 1)
            return;

        // n the array offset of this pos
        int n = Index(i, j);
        // vv is the value there
        double vv = v[n];

        bool qpk = false;

        // if not start, and goes up, then leave
        // but mark as edge value for background
        if ((prev >= 0) && (prev <= vv)) {
            // if already marked as current peak or bkgnd, skip
            if (hit[n] == -id || hit[n] == id) {
                return;
            }

            // otherwise mark it as border
            bordersum += vv;
            borderhits++;

            // always mark as bkgnd - even if it's the edge or part of a
            // previously marked peak as long as not part of current peak
            hit[n] = -id;
            return;
        }

        // if spot has been id'd already, or shows no path downward, leave
        if ((hit[n] > 0) ||
            GoingUp(i, j, thresh, id)) {  // GoingUp should check all dirs
            MarkAsBoundary(i, j, id, bordersum, borderhits, qpk);
            return;
        }

        // ok - this is part of the peak.  Mark it and continue exploration

        double x = mConversion.IndexToWorldX(i);
        double y = mConversion.IndexToWorldY(j);
        xsum += x * vv;
        ysum += y * vv;
        xsig += x * x * vv;
        ysig += y * y * vv;

        vsum += vv;
        nhits++;

        hit[n] = id;  // mark with positive id

        ExplorePeakSlope(id, i - 1, j, thresh, vv, xsum, ysum, xsig, ysig, vsum,
                         nhits, bordersum, borderhits);
        ExplorePeakSlope(id, i + 1, j, thresh, vv, xsum, ysum, xsig, ysig, vsum,
                         nhits, bordersum, borderhits);
        ExplorePeakSlope(id, i, j + 1, thresh, vv, xsum, ysum, xsig, ysig, vsum,
                         nhits, bordersum, borderhits);
        ExplorePeakSlope(id, i, j - 1, thresh, vv, xsum, ysum, xsig, ysig, vsum,
                         nhits, bordersum, borderhits);
    }

    void ExplorePeakSlope2(const int id, const int i, const int j,
                           const double pheight, const double thresh,
                           const double peakheightmin, const double prev,
                           double &xsum, double &ysum, double &xsig,
                           double &ysig, double &vsum, int &nhits) {
        // if at edge, leave
        if (i < 1 || i >= mConversion.mNMZ - 1 || j < 1 ||
            j >= mConversion.mNRT - 1)
            return;

        // n the array offset of this pos
        int n = Index(i, j);
        // vv is the value there
        double vv = v[n];

#ifdef DBGPK
        if ((fabs(xcoord(i) - tx)) < eps && fabs(ycoord(j) - ty) < eps)
            qpk = true;
#endif

        // here set threshold to MAXIMUM of fractional peak height and a min
        // value
        double bestthresh = thresh * pheight;
        if (bestthresh < peakheightmin) bestthresh = peakheightmin;

#if 1  // This is how it is written 5/26/09, and it allows overwrite of previous
       // peaks. if not start, and goes up, then leave
        if ((prev >= 0) &&
            ((hit[n] == id) || (prev < vv) || (vv < bestthresh))) {
            return;
        }
#endif

#if 0  //  This is different implementation that lets first peaks "win" the
       //  territory
        if ((prev >= 0) && ((hit[n] != 0) || (prev < vv) || (vv < bestthresh))) {
			return;
        }
#endif

        // ok - this is part of the peak.  Mark it and continue exploration

        double x = i;  // xcoord(i);
        double y = j;  // ycoord(j);
        xsum += x * vv;
        ysum += y * vv;
        xsig += x * x * vv;
        ysig += y * y * vv;

        vsum += vv;
        nhits++;

#ifdef DBGPK
        if (qpk)
            std::cout << id << " 0 " << i << " " << j << " " << hit[n] << " "
                      << x << " " << y << " " << vv << std::endl;

            // std::cout<< "id x y nhits " << id << " " << x << " " << y << " "
            // << nhits << std::endl;
#endif

        hit[n] = id;  // mark with positive id

        ExplorePeakSlope2(id, i - 1, j, pheight, thresh, peakheightmin, vv,
                          xsum, ysum, xsig, ysig, vsum, nhits);
        ExplorePeakSlope2(id, i + 1, j, pheight, thresh, peakheightmin, vv,
                          xsum, ysum, xsig, ysig, vsum, nhits);
        ExplorePeakSlope2(id, i, j + 1, pheight, thresh, peakheightmin, vv,
                          xsum, ysum, xsig, ysig, vsum, nhits);
        ExplorePeakSlope2(id, i, j - 1, pheight, thresh, peakheightmin, vv,
                          xsum, ysum, xsig, ysig, vsum, nhits);

        ExplorePeakSlope2(id, i - 1, j - 1, pheight, thresh, peakheightmin, vv,
                          xsum, ysum, xsig, ysig, vsum, nhits);
        ExplorePeakSlope2(id, i + 1, j + 1, pheight, thresh, peakheightmin, vv,
                          xsum, ysum, xsig, ysig, vsum, nhits);
        ExplorePeakSlope2(id, i - 1, j + 1, pheight, thresh, peakheightmin, vv,
                          xsum, ysum, xsig, ysig, vsum, nhits);
        ExplorePeakSlope2(id, i + 1, j - 1, pheight, thresh, peakheightmin, vv,
                          xsum, ysum, xsig, ysig, vsum, nhits);
    }

    void FindBoundary(const int id, const int i, const int j, double &bordersum,
                      int &borderhits) {
        // if at edge, leave
        if (i < 1 || i >= mConversion.mNMZ - 1 || j < 1 ||
            j >= mConversion.mNRT - 1)
            return;

        // n the array offset of this pos
        int n = Index(i, j);

        // bool qpk = false;
#ifdef DBGPK
        if ((fabs(xcoord(i) - tx)) < eps && fabs(ycoord(j) - ty) < eps)
            qpk = true;
#endif

        // if it's not the peak or boundary, mark it as boundary and leave
        if (hit[n] != -id && hit[n] != id) {
            // vv is the value there
            double vv = v[n];
            hit[n] = -id;
            bordersum += vv;
            borderhits++;

#ifdef DBGPK
            if (qpk)
                std::cout << id << " 1 " << i << " " << j << " " << hit[n]
                          << " " << xcoord(i) << " " << ycoord(j) << " " << vv
                          << std::endl;
#endif
            return;
        }

        // if already marked as boundary, leave
        if (hit[n] == -id) return;

        // if it's part of the peak, mark it as boundary, but don't count it as
        // boundary
        if (hit[n] == id) {
            hit[n] = -id;
        }

        FindBoundary(id, i + 1, j, bordersum, borderhits);
        FindBoundary(id, i + 1, j - 1, bordersum, borderhits);
        FindBoundary(id, i, j - 1, bordersum, borderhits);
        FindBoundary(id, i - 1, j - 1, bordersum, borderhits);
        FindBoundary(id, i - 1, j, bordersum, borderhits);
        FindBoundary(id, i - 1, j + 1, bordersum, borderhits);
        FindBoundary(id, i, j + 1, bordersum, borderhits);
        FindBoundary(id, i + 1, j + 1, bordersum, borderhits);
    }

    // this converts index space to world space
    void WarpPeakToMz(Peak *p) const {
        p->mXSig = p->mXSig * WorldSigmaFromIndex(p->mX);
        p->mX = mConversion.IndexToWorldX(p->mX);
        p->mY = mConversion.IndexToWorldY(p->mY);
        p->mXFullCentroid = mConversion.IndexToWorldX(p->mXFullCentroid);
        p->mYFullCentroid = mConversion.IndexToWorldY(p->mYFullCentroid);
        p->mXPeak = mConversion.IndexToWorldX(p->mXPeak);
        p->mYPeak = mConversion.IndexToWorldY(p->mYPeak);
    }

    // FindPeaks
    // Calls IsPeak to find ALL local maxima
    //   and adds each one to list
    // Then goes through and calls ExplorePeakSlope
    void FindPeaks(int npeaks, double thresh, double peakheightmin) {
        for (int y = 1; y < mConversion.mNRT - 1; y++) {
            for (int x = 1; x < mConversion.mNMZ - 1; x++) {
                if (IsPeak(x, y)) {
                    AddPeak(x, y, npeaks);
                }
            }
        }

        // this sorts based on peak value, but don't know bkgnd yet
        sort(peaks.begin(), peaks.end());

        int nallpeaks = peaks.size();

        if (npeaks > nallpeaks) npeaks = nallpeaks;

        // Now run through each peak and explore nbhrd
        for (int i = 0; i < npeaks; i++) {
            Peak *p = &peaks[i];
            p->mID = i;
            p->mCount = 1;
            double x = p->mI;  // xcoord(p->mI);
            double y = p->mJ;  // ycoord(p->mJ);
            p->mXPeak = x;
            p->mYPeak = y;
            p->mXFullCentroid = p->mHeight * x;
            p->mYFullCentroid = p->mHeight * y;
            p->mXSig = p->mHeight * x * x;
            p->mYSig = p->mHeight * y * y;
            p->mVolume = p->mHeight;
            double bordersum = 0;
            int borderhits = 0;
            ExplorePeakSlope2(i + 1, p->mI, p->mJ, p->mHeight, thresh,
                              peakheightmin, -1, p->mXFullCentroid,
                              p->mYFullCentroid, p->mXSig, p->mYSig, p->mVolume,
                              p->mCount);
            FindBoundary(i + 1, p->mI, p->mJ, bordersum, borderhits);
            if (borderhits < 1) borderhits = 1;
            p->mBorderBkgnd = bordersum / borderhits;
            // this is full centroid
            p->mXFullCentroid /= p->mVolume;
            p->mYFullCentroid /= p->mVolume;
            FindWindowedCentroid(
                *p);  // find local centroid using only i, j, xsig, ysig
                      // now find sigmas based on full centroid
            p->mXSig = sqrt(p->mXSig / p->mVolume -
                            p->mXFullCentroid * p->mXFullCentroid + 0.0001);
            p->mYSig = sqrt(p->mYSig / p->mVolume -
                            p->mYFullCentroid * p->mYFullCentroid + 0.0001);
#ifdef DBGPK
            std::cout << i + 1 << " 9 " << p->mI << " " << p->mJ << " "
                      << p->mCount << " " << p->mX << " " << p->mY << " "
                      << p->mVolume << " " << p->mBorderBkgnd << " "
                      << borderhits << " " << p->mHeight << std::endl;
#endif

            p->mSNHeight = p->mHeight / p->mBorderBkgnd;
            p->mSNVolume = p->mVolume / p->mCount / p->mBorderBkgnd;

            // all above calcs should be in index space
            // now convert to m/z space and account for warping
            // coordinates and sigmas need to be mapped, including windowed
            // centroids

            WarpPeakToMz(p);

            // negative volume is sign of pathological peak
            if (p->mVolume < 0 || p->mHeight < 0) {
                p->mVolume = 0.1;
                p->mHeight = 0.1;
                p->mSNHeight = 0.001;
                p->mSNVolume = 0.001;
            }
        }

        std::vector<std::pair<int, Peak>> paired_peaks;
        paired_peaks.reserve(peaks.size());

        size_t size = peaks.size();
        for (size_t p = 0; p < size; ++p) {
            paired_peaks.push_back({p, peaks[p]});
        }

        paired_peaks.erase(
            std::remove_if(paired_peaks.begin(), paired_peaks.end(),
                           [](const std::pair<int, Peak> &peak) {
                               if (peak.second.mCount > 1) {
                                   return false;
                               } else {
                                   return true;
                               }
                           }),
            paired_peaks.end());

        std::sort(paired_peaks.begin(), paired_peaks.end(), IndexedSigSort);

        // Cleans the Mesh.peaks and Mesh.indexed_peaks std::vectors and
        // reserves the required amount of elements.
        peaks.clear();
        indexed_peaks.clear();
        peaks.reserve(paired_peaks.size());
        indexed_peaks.resize(paired_peaks.size());

        // Fills the Mesh.peaks std::vector with peaks;
        for (std::pair<int, Peak> &paired_peak : paired_peaks) {
            peaks.push_back(std::move(paired_peak.second));
            indexed_peaks[paired_peak.first] = &peaks.back();
        }
    }

    // go +/- 4 around peak and find weighted centroid
    // also find weighted s/n
    void FindWindowedCentroid(Peak &p) {
        enum { RCENT = 1 };
        enum { RWID = 4 * RCENT };
        double invrsq = 1.0 / (RCENT * RCENT);
        double xwsum = 0;
        double ywsum = 0;
        double xsum = 0;
        double ysum = 0;
        double wvsum = 0;
        double wbksum = 0;
        double wsum = 0;  // sum of w
        double vsum = 0;
        double xvsum = 0;
        double yvsum = 0;
        int imin = p.mI - RWID;
        if (imin < 0) imin = 0;
        int imax = p.mI + RWID + 1;
        if (imax > mConversion.mNMZ) imax = mConversion.mNMZ;
        int jmin = p.mJ - RWID;
        if (jmin < 0) jmin = 0;
        int jmax = p.mJ + RWID + 1;
        if (jmax > mConversion.mNRT) jmax = mConversion.mNRT;
        float *pv;

		for (int j = jmin; j < jmax; j++) {
            pv = v.get() + j * mConversion.mNMZ + imin;  // v.get() instead of v
            for (int i = imin; i < imax; i++, pv++) {
                int di = p.mI - i;
                int dj = p.mJ - j;
                double w = exp(-0.5 * (di * di + dj * dj) * invrsq);
                double vv = *pv;
                double wv = vv * w;
                xvsum += i * vv;
                yvsum += j * vv;
                vsum += vv;
                wsum += w;        // sum of weights alone
                xwsum += wv * i;  // wv*xcoord(i);
                ywsum += wv * j;  // wv*ycoord(j);
                xsum += i;
                ysum += j;
                wvsum += wv;  // sum of weights*v
                wbksum += w * p.mBorderBkgnd;
            }
        }
        p.mX = xvsum / vsum;
        p.mY = yvsum / vsum;
        p.mVCentroid = vsum;
        p.mHeight = wvsum / wsum;
        if (wvsum <= wbksum || wbksum == 0) {
            p.mSNCentroid = 0.001;
        } else {
            p.mSNCentroid = (wvsum) / wbksum;
        }
    }

    void DumpPeaks() { DumpPeaks(std::cout); }

    static void DumpPeaks(std::vector<Peak> peaks, std::ostream &sout) {
        int npeaks = peaks.size();
        std::vector<TAPP::Filetypes::TAPP::PKS> output_vector;
        output_vector.reserve(npeaks);

        for (int i = 0; i < npeaks; i++) {
            Peak p = peaks[i];
            output_vector.push_back({(size_t)i, p.mX, p.mY, p.mHeight,
                                     p.mVolume, p.mVCentroid, p.mXSig, p.mYSig,
                                     (size_t)p.mCount, p.mBorderBkgnd,
                                     p.mSNVolume, p.mSNHeight, p.mSNCentroid});
        }

        TAPP::Filetypes::TAPP::SetGlobalPrecision(sout);
        TAPP::Filetypes::TAPP::WritePKS(sout, output_vector, {'\n', ' ', '\"'},
                                        true);
    }

    void DumpPeaks(std::ostream &sout) { DumpPeaks(peaks, sout); }

    inline void AddPeak(int i, int j, int npeaks) {
        if (peaks.size() >= npeaks * 1.5) {
            sort(peaks.begin(), peaks.end());
            peaks.erase(peaks.begin() + npeaks, peaks.end());
        }
        peaks.push_back(
            Peak(i, j, v[Index(i, j)]));  // for warped peak don't need to know
                                          // x, y at this point - just indices
    }

    static int getNTokens(std::string &s) {
        size_t ptr = 0;
        int ntoks = 0;
        while (ptr != std::string::npos) {
            // find start of token
            ptr = s.find_first_not_of(' ', ptr);
            if (ptr == std::string::npos) break;
            // find end of token
            ptr = s.find_first_of(' ', ptr);
            ntoks++;
        }
        return ntoks;
    }

    static std::unordered_set<uint32_t> LoadPeaksFromPBIN(
        const std::string &filepath, std::vector<Peak> &peak_vector) {
        MiniPeakSet mps;
        mps.loadPeaksFile(filepath);

        std::unordered_set<uint32_t> classes;
        for (const MiniPeak &mini_peak : mps.mPeaks) {
            Peak new_peak;
            new_peak.mID = mini_peak.mID;
            new_peak.mX = mini_peak.mX;
            new_peak.mY = mini_peak.mY;
            new_peak.mHeight = mini_peak.mH;
            new_peak.mClass = mini_peak.mClass;
            new_peak.mFile = mini_peak.mFileID;
            peak_vector.push_back(std::move(new_peak));
            classes.insert(mini_peak.mClass);
        }

        return classes;
    }

    // expect:
    // N X Y Height Volume VCentroid XSigma YSigma Count LocalBkgnd SNVolume
    // SNHeight SNCentroid  > 6 N X Y Height XSigma YSigma = 6 X Y Height XSigma
    // YSigma = 5 N X Y Height = 4 X Y Height = 3
    static void loadPeaks(std::vector<Peak> &thePeaks, std::string peakFilename,
                          int FileID = 0, int Class = 0, double xwidth = 0.2,
                          double ywidth = 0.2) {
        std::ifstream inFile(peakFilename.c_str());
        std::string fileRecord;
        getline(inFile, fileRecord);  // Skip header record.

        bool skipGridData =
            false;  // If grid positions column present, skip for now.
        if (fileRecord.find("GridX") != std::string::npos) skipGridData = true;

        bool hasCentroid = false;
        if (fileRecord.find("SNCentroid") != std::string::npos)
            hasCentroid = true;

        bool hasVCentroid = false;
        if (fileRecord.find("VCentroid") != std::string::npos)
            hasVCentroid = true;

        int ncols = getNTokens(fileRecord);
        int npeak = 0;

        if (ncols < 3) {
            std::cout << "Peak file " << peakFilename << " has only " << ncols
                      << " columns, and requires a minimum of 3." << std::endl;
            exit(-1);
        }
        std::cout << "Loading peak file " << peakFilename << " with " << ncols
                  << " columns." << std::endl;
        std::cout << "Header Line:" << std::endl;
        std::cout << fileRecord << std::endl;
        if (ncols < 5)
            std::cout << "  Using fixed mzwidth, rtwidth = " << xwidth << " "
                      << ywidth;
        std::cout << std::endl;

        if (ncols > 6) {
            while (!inFile.fail() && !inFile.eof()) {
                getline(inFile, fileRecord);
                if (!inFile.eof()) {
                    std::stringstream iss(fileRecord);
                    double skipDbl = 0;
                    Peak newPeak;
                    iss >> newPeak.mID;
                    if (skipGridData) {
                        iss >> skipDbl;
                        iss >> skipDbl;
                    }  // skip over gridx, gridy
                    if (!iss.eof()) {
                        iss >> newPeak.mX;
                        iss >> newPeak.mY;
                        iss >> newPeak.mHeight;
                        iss >> newPeak.mVolume;
                        if (hasVCentroid)
                            iss >> newPeak.mVCentroid;
                        else
                            newPeak.mVCentroid = 0;
                        iss >> newPeak.mXSig;
                        iss >> newPeak.mYSig;
                        iss >> newPeak.mCount;
                        iss >> newPeak.mBorderBkgnd;
                        iss >> newPeak.mSNVolume;
                        iss >> newPeak.mSNHeight;
                        if (hasCentroid)
                            iss >> newPeak.mSNCentroid;
                        else
                            newPeak.mSNCentroid = 1;
                        newPeak.mClass = Class;
                        newPeak.mFile = FileID;
                        thePeaks.push_back(newPeak);
                    }
                }
            }
        } else {
            while (!inFile.fail() && !inFile.eof()) {
                getline(inFile, fileRecord);
                if (!inFile.eof()) {
                    std::stringstream iss(fileRecord);
                    Peak newPeak;

                    if (ncols == 4 || ncols == 6) {
                        iss >> newPeak.mID;
                    } else {
                        newPeak.mID = npeak;
                        npeak++;
                    }

                    iss >> newPeak.mX;
                    iss >> newPeak.mY;
                    iss >> newPeak.mHeight;

                    if (ncols == 6) {
                        iss >> newPeak.mXSig;
                        iss >> newPeak.mYSig;
                    } else {
                        newPeak.mXSig = xwidth;
                        newPeak.mYSig = ywidth;
                    }
                    newPeak.mVolume = 0;
                    newPeak.mVCentroid = 0;
                    newPeak.mSNHeight = 1;
                    newPeak.mSNVolume = 0;
                    newPeak.mClass = Class;
                    newPeak.mFile = FileID;
                    newPeak.mBorderBkgnd = 0;
                    newPeak.mSNCentroid = 1;

                    thePeaks.push_back(newPeak);
                }
            }
        }
        inFile.close();
    }

    static std::vector<Peak> loadPeaks(std::string peakFilename,
                                       double xwidth = 0.2,
                                       double ywidth = 0.2) {
        std::vector<Peak> thePeaks;
        loadPeaks(thePeaks, peakFilename, 0, 0, xwidth, ywidth);
        return thePeaks;
    }

    static std::vector<Peak> cullPeaks(std::vector<Peak> pall,
                                       std::vector<double> kill, double w,
                                       int ndesired, double maxmzwidth,
                                       double maxrtwidth) {
        std::vector<Peak> thePeaks;
        int npeaks = pall.size();
        int nkill = kill.size();

        int ngood = 0;
        for (int i = 0; i < npeaks && ngood < ndesired; i++) {
            Peak p = pall[i];
            if (p.mXSig > maxmzwidth || p.mYSig > maxrtwidth) continue;
            bool good = true;
            for (int j = 0; j < nkill; j++) {
                if (fabs(p.mX - kill[j]) < w) {
                    good = false;
                    break;
                }
            }
            if (good) {
                thePeaks.push_back(p);
                ngood++;
            }
        }
        return thePeaks;
    }

    static std::vector<Peak> cullPeaks(std::vector<Peak> pall, char *fname,
                                       double w, int ndesired,
                                       double maxmzwidth, double maxrtwidth) {
        std::vector<double> kill;
        FILE *f = fopen(fname, "r");
        if (!f) {
            std::cout << "Kill.txt not found - using all peaks for warp"
                      << std::endl;
        } else {
            while (!feof(f)) {
                double d;
                int nread = fscanf(f, "%lf", &d);
                if (nread == 1) kill.push_back(d);
            }
            fclose(f);
        }
        return cullPeaks(pall, kill, w, ndesired, maxmzwidth, maxrtwidth);
    }

    static DoubleMatrix &makeSimpleTICFromPeaks(std::vector<Peak> thePeaks,
                                                float ymin, float ymax,
                                                float dy, float heightTh0 = 0) {
        int numPoints = (ymax - ymin) / dy + 1;
        double squeeze = 3.0;

        DoubleMatrix &theTic = *(new DoubleMatrix(numPoints, 2));
        theTic.zeroIt();

        float GaussFactor = 1.0;

        for (int i = 0; i < (int)thePeaks.size(); i++) {
            Peak thePeak = thePeaks[i];

            // Ignore peaks below height threshold.
            if (thePeak.mHeight < heightTh0) continue;

            float y0 = thePeak.mY - 5 * thePeak.mYSig;
            float y1 = thePeak.mY + 5 * thePeak.mYSig;
            int i0 = (y0 - ymin) / dy;
            int i1 = (y1 - ymin) / dy;
            int im = (thePeak.mY - ymin) / dy;

            if (i0 < 0) i0 = 0;
            if (i1 >= numPoints) i1 = numPoints - 1;

            for (int j = i0; j < i1; j++) {
                float s = (im - j) * dy / thePeak.mYSig *
                          squeeze;  // dist from peak in sigma
                float v = exp(-s * s / 2) * thePeak.mHeight * GaussFactor;
                theTic.get(j, 1) += v;
            }
        }

        for (int i = 0; i < numPoints; i++) {
            theTic.get(i, 0) = ymin + i * dy;
        }

        return theTic;
    }
};

#endif
