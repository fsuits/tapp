// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#pragma once 
#include <string>
#include <vector>
#include <algorithm>
#include <climits>
#include <cstring>

class TileInfo {
public:
    unsigned mStartIndex;
    unsigned mNPeaks;

    TileInfo(): TileInfo(0, 0) {}

    TileInfo(const unsigned si, const unsigned np): mStartIndex(si), mNPeaks(np) {}
};

class MiniPeak
{
public:
    float mX;
    float mY;
    float mH;
    unsigned mID;    // ID refers to the original external reference ID in the file
    unsigned mTile;  // Tile is not read from the file but is needed for sorting
    unsigned mClass;
    unsigned mFileID;

    MiniPeak() : MiniPeak(0,0,0,0) {}

    MiniPeak(const int id, const float x, const float y, const float h) : mX(x), mY(y), mH(h), mID(id), mTile(0), mClass(0), mFileID(0) {}

    // dump 4 items of a single peak
    void dump(FILE *f) const {
        fwrite(&mID, sizeof(unsigned), 1, f);
        fwrite(&mX, sizeof(float), 1, f);
        fwrite(&mY, sizeof(float), 1, f);
        fwrite(&mH, sizeof(float), 1, f);
    }

    // Load 4 items of a single peak.  This can't set tileid because peak does not know about grid
    void load(FILE *f) const {
        fread((void *)&mID, sizeof(unsigned), 1, f);
        fread((void *)&mX, sizeof(float), 1, f);
        fread((void *)&mY, sizeof(float), 1, f);
        fread((void *)&mH, sizeof(float), 1, f);
    }

    void describe() const {
        std::cout << mID << " " << mX << " " << mY << " " << mH << " " << mTile << " " << mFileID << " " << mClass << std::endl;
    }

    const bool operator<(const MiniPeak &rhs) const {
        if (mTile != rhs.mTile)
            return mTile < rhs.mTile;
        return mID < rhs.mID;
    }

    const bool operator==(const MiniPeak &rhs) const {
        return mTile == rhs.mTile;
    }

    const bool operator>(const MiniPeak &rhs) const {
        if (mTile != rhs.mTile)
            return mTile > rhs.mTile;
        return mID > rhs.mID;
    }
};

class MiniPeakSet {
public:
    enum {FNAMELENGTH=1024};
    enum {VERSION=100};
    enum {BADTILE=UINT_MAX};
    enum {HEADERSIZE = 1072};
    unsigned mVersion;
    float mXMin;
    float mYMin;
    float mXMax;
    float mYMax;
    float mDx;
    float mDy;
    unsigned mNx;
    unsigned mNy;
    unsigned mNTiles;
    unsigned mNPeaks;
    unsigned mClass;
    unsigned mFileID;
    char mFileName[FNAMELENGTH];
    long mPeakStartSeek;
    std::vector<TileInfo> mTileStartList;
    std::vector<MiniPeak> mPeaks;

    MiniPeakSet(): mVersion(0), mXMin(0), mYMin(0), mXMax(0), mYMax(0), mDx(0), mDy(0), mNx(0), mNy(0), mNTiles(0), mNPeaks(0), mClass(0), mFileID(0), mPeakStartSeek(0) {
        *mFileName = 0;
    }

    void describe() {
        std::cout << "XMin, XMax, YMin, YMax " << mXMin << " " << mXMax << " " << mYMin << " " << mYMax << std::endl;
        std::cout << "Dx, Dy, Nx, Ny " << mDx << " " << mDy << " " << mNx << " " << mNy << std::endl;
        std::cout << "Class FileID NTiles NPeaks " << mClass << " " << mFileID << " " << mNTiles << " " << mNPeaks << std::endl;
    }

    // set grid counts based on grid size and step size
    void setGridCounts() {
        mNx = (mXMax-mXMin)/mDx+0.5;
        mNy = (mYMax-mYMin)/mDy+0.5;
        mNTiles = mNx*mNy;
    }

    // get tile id from x, y
    unsigned getTileID(const float x, const float y) const {
        unsigned tile;
        if (x < mXMin || x >= mXMax || y < mYMin || y >= mYMax) {
            tile = BADTILE;
        } else {
            unsigned xn = (x-mXMin)/(mXMax-mXMin)*mNx;
            unsigned yn = (y-mYMin)/(mYMax-mYMin)*mNy;
            tile = yn*mNx+xn;
        }
        return tile;
    }

    // find file offset of first peak in tile - and number of peaks in that tile
    void findPeakStartAndCount(const unsigned NTile, long &startseek, unsigned &npeaks) {
        startseek = HEADERSIZE + mNTiles*8 + mTileStartList[NTile].mStartIndex*16;
        npeaks = mTileStartList[NTile].mNPeaks;
    }

    // go through sorted peaks and dump out tile indices
    // each tile will have an entry like:
    // StartPeakIndexForTile  NPeaksInTile
    void dumpHeader(FILE *fout) {
        fwrite(&mVersion, sizeof(unsigned), 1, fout);
        fwrite(&mXMin,    sizeof(float), 1, fout);
        fwrite(&mXMax,    sizeof(float), 1, fout);
        fwrite(&mYMin,    sizeof(float), 1, fout);
        fwrite(&mYMax,    sizeof(float), 1, fout);
        fwrite(&mDx,      sizeof(float), 1, fout);
        fwrite(&mDy,      sizeof(float), 1, fout);
        fwrite(&mNx,      sizeof(unsigned), 1, fout);
        fwrite(&mNy,      sizeof(unsigned), 1, fout);
        mNPeaks = mPeaks.size();
        fwrite(&mNPeaks,  sizeof(unsigned), 1, fout);
        fwrite(&mClass,   sizeof(unsigned), 1, fout);
        fwrite(&mFileID,  sizeof(unsigned), 1, fout);  // 12*4 = 48 bytes
        fwrite(mFileName, sizeof(mFileName), 1, fout);  // 1024 bytes

        // total initial header size is 1072
        // index is NTiles*8
        // peaks is npeaks*16

        unsigned tcurrent = 0;  // the current tileid that is being skipped over till a new one
        unsigned pindex = 0;    // the index into the peak list
        unsigned zero = 0;
        unsigned currentnpeaks = 0;

        // now start going through peaks and find next bump in tile value;
        // find first peak with tileid greater than tcurrent and output its index and fill with zeros for all missing tiles

        std::vector<MiniPeak>::const_iterator p = mPeaks.begin();

        // no matter what output zero as first index
        fwrite(&zero, sizeof(unsigned), 1, fout);

        // skip any empty tiles until the true start of next tile
        while(p != mPeaks.end()) {
            if (p->mTile > tcurrent) {
                // complete prev tile and fill in any empty ones
                for (unsigned i=tcurrent; i<p->mTile; i++) {
                    // output total for previous tile
                    fwrite(&currentnpeaks, sizeof(unsigned), 1, fout);
                    fwrite(&pindex, sizeof(unsigned), 1, fout);
                    currentnpeaks = 0;
                }
                tcurrent = p->mTile;
            }
            pindex++;
            p++;
            currentnpeaks++;
        }

        // output final non-zero tile count
        fwrite(&currentnpeaks, sizeof(unsigned), 1, fout);

        // now fill any remaining empty tiles
        for (unsigned i=tcurrent+1; i<mNTiles; i++) {
            fwrite(&zero, sizeof(unsigned), 1, fout);
            fwrite(&zero, sizeof(unsigned), 1, fout);
        }

        mPeakStartSeek = ftell(fout);
    }

    // dump all the peaks in this set
    void dumpPeaks(FILE *fout) const {
        for (std::vector<MiniPeak>::const_iterator p = mPeaks.begin(); p != mPeaks.end(); p++) {
            p->dump(fout);
        }
    }

    // load the binary header and point to start of peak list
    void loadHeader(FILE *fin) {
        fread(&mVersion, sizeof(unsigned), 1, fin);
        fread(&mXMin, sizeof(float), 1, fin);
        fread(&mXMax, sizeof(float), 1, fin);
        fread(&mYMin, sizeof(float), 1, fin);
        fread(&mYMax, sizeof(float), 1, fin);
        fread(&mDx, sizeof(float), 1, fin);
        fread(&mDy, sizeof(float), 1, fin);
        fread(&mNx, sizeof(unsigned), 1, fin);
        fread(&mNy, sizeof(unsigned), 1, fin);  
        fread(&mNPeaks, sizeof(unsigned), 1, fin);
        fread(&mClass, sizeof(unsigned), 1, fin);
        fread(&mFileID, sizeof(unsigned), 1, fin);  // 12*4 = 48 bytes
        fread(mFileName, sizeof(mFileName), 1, fin);  // 1024 bytes
        mNTiles = mNx*mNy;
        for (unsigned i=0; i<mNTiles; i++) {
            unsigned tindex;
            unsigned npeaks;
            fread(&tindex, sizeof(unsigned), 1, fin);
            fread(&npeaks, sizeof(unsigned), 1, fin);
            mTileStartList.push_back(TileInfo(tindex, npeaks));
        }
        mPeakStartSeek = ftell(fin);
    }

    // Find the tile ID of the next peak.  May be same as current.  May require skipping over empty tiles
    unsigned nextPeakTile(bool init = false) {
        static unsigned npeaks = 0;
        static unsigned currenttile = 0;

        if (init) {
            npeaks = 0;
            currenttile = 0;
            while (mTileStartList[currenttile].mNPeaks == 0)
                currenttile++;  // this could be first one
            return currenttile;
        }

        npeaks++;
        if (npeaks > mNPeaks) {
            return BADTILE;
        }
        // did we finish previous tile
        if (npeaks > mTileStartList[currenttile].mNPeaks) {
            // skip to next non-empty tile
            currenttile++;
            while (mTileStartList[currenttile].mNPeaks == 0)
                currenttile++;  // this could be first one
        }
        return currenttile;
    }

    // load all peaks from binary file
    void loadPeaks(FILE *fin) {
        //int nloaded = 0;
        mPeaks.clear();
        nextPeakTile(true);
        for (unsigned i=0; i<mNPeaks; i++) {
            MiniPeak p;
            p.load(fin);
            p.mTile = nextPeakTile();
            p.mClass = mClass;
            p.mFileID = mFileID;
            mPeaks.push_back(p);
        }
    }

    // assumes file has already been opened and header loaded
    void loadPeaksForTile(FILE *fin, std::vector<MiniPeak> &v, const unsigned NTile) {
        long startseek;
        unsigned npeaks;
        findPeakStartAndCount(NTile, startseek, npeaks);
        fseek(fin, startseek, SEEK_SET);
        for (unsigned i=0; i<npeaks; i++) {
            MiniPeak p;
            p.load(fin);
            p.mTile = NTile;
            p.mClass = mClass;
            p.mFileID = mFileID;
            v.push_back(p);
        }
    }

    // load peaks from the big text file but only save the ones within the tile layout
    // then sort them
    void loadPeaksFile(const std::string& filepath) {
        strcpy(mFileName, filepath.c_str());
        FILE *f = fopen(filepath.c_str(), "r");
        char s[2048];

        fgets(s, 2048, f);

        while(!feof(f) && fgets(s, 2048, f)) {
            float x, y, h;
            int id;
            sscanf(s, "%d %f %f %f", &id, &x, &y, &h);
            MiniPeak p = MiniPeak(id, x, y, h);
            p.mTile = getTileID(x, y);
            if (p.mTile < mNx*mNy && p.mTile != BADTILE)
                mPeaks.push_back(p);
        }
        fclose(f);
        sort(mPeaks.begin(), mPeaks.end());
    }

    // dump the list of peaks after sorting by tile.  Dump entire file - header and peaks.
    void dumpSortedPeaksFile(const std::string& outname) {
        FILE *fout = fopen(outname.c_str(), "wb");

        dumpHeader(fout);

        dumpPeaks(fout);

        fclose(fout);
    }

    // initialize tile layout based on args
    void initTileSettingsFromArgs(const double x_min, const double x_max, const double y_min, const double y_max, const double x_dimensions, const double y_dimensions) {
        mXMin = x_min;
        mYMin = y_min;
        mXMax = x_max;
        mYMax = y_max;
        mDx = x_dimensions;
        mDy = y_dimensions;
        //mClass = atoi(argv[7]);
        //mFileID = atoi(argv[8]);
        memset(mFileName, 0, FNAMELENGTH);
        mVersion = VERSION;
        mNPeaks = 0;
        setGridCounts();
    }

    // simple test 1 - load peaks and save binary file
    void doTest1(const double x_min, const double x_max, const double y_min, const double y_max, const double x_dimensions, const double y_dimensions, std::string& filename, std::string& output)
	{
        initTileSettingsFromArgs(x_min, x_max, y_min, y_max, x_dimensions, y_dimensions);
	    loadPeaksFile(filename);
		dumpSortedPeaksFile(output);
    }

    void clear() {
        mPeaks.clear();
        mNPeaks = 0;
        *mFileName = 0;
    }

    // simple test 2 - load peaks from binary file for one tile
    void doTest2(char *outname, std::vector<MiniPeak> &v) {
        FILE *fin = fopen(outname, "rb");
        loadHeader(fin);
        loadPeaksForTile(fin, v, 15);
        fclose(fin);
    }
};
