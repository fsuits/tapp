// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#ifndef GRID_H_INCLUDE
#define GRID_H_INCLUDE

#include <string>
#include <iostream>
#include <vector>
// #include <stdlib.h>
// #include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <map>

#include "Attribute.h"

using namespace std ;


template<class CoordType>
class Point2D
{
public:
    CoordType mX;
    CoordType mY;
    Point2D() {
        zero();
    }

    Point2D(CoordType x, CoordType y) : mX(x), mY(y) {}

    void zero() {
        mX = mY = 0;
    }
};

template<class T>
class Range2D
{
    T mMin;
    T mMax;
    T mRange;

public:
    Range2D() {
        mMin = mMax = mRange = 0;
    }
    Range2D(const T min, const T max) {
        mMin = min;
        mMax = max;
        mRange = mMax-mMin;
    }
    void set(char *s) {
        sscanf(s, "%f-%f", &mMin, &mMax);
        mRange = mMax-mMin;
    }
    void set(const T min, const T max) {
        mMin = min;
        mMax = max;
    }
    void get(T &min, T &max) {
        min = mMin;
        max = mMax;
    }
    T getRange() {
        return mRange;
    }
};


// Coord is base class
// simple interface to access elements and get size
template<class CoordType>
class Coord : public AttributeMap
{
public:
    // virtual CoordType operator[](const int i);
    // virtual int size() const;
};

template<class CoordType>
class CoordRegular : public Coord<CoordType>
{
    CoordType mMin;
    CoordType mMax;
    double mDenom;
    int mCount;

public:
    void setRange(const CoordType a, const CoordType b, const int N) {
        mMin = a;
        mMax = b;
        mCount = N;
        mDenom = (mMax-mMin)/double(mCount-1);
    }

    // this can't be virtual for some reason...
#if 1
    const CoordType operator[](const int i) const {
        return mMin + i*mDenom;
        // return mMin + mDenom;
    }
#endif
    
    virtual int size() const {
        return mCount;
    }

    class iterator
    {
    public:
        int mI;
        CoordType mIMin;
        double mIDenom;
        int mIN;
        iterator operator++(int dummy) {
            mI++;
            if (mI < mIN) {
                return *this;
            }
            mI = -1;
            return *this;
        }
        CoordType operator*() {
            return static_cast<CoordType>(mIMin) + static_cast<CoordType>(mI)*static_cast<CoordType>(mIDenom);
        }
        bool operator!=(const iterator &rhs) {
            return (mI != rhs.mI);
        }
    };

    iterator begin() {
        iterator I;
        I.mI = 0;
        I.mIMin = mMin;
        I.mIDenom = mDenom;
        I.mIN = mCount;
        return I;
    }

    const static iterator end() {
        iterator I;
        I.mI = -1;
        return I;
    }
};


template<class CoordType>
class CoordIrregular : public Coord<CoordType>
{
public:
    vector<CoordType> mData;
#if 1
    const virtual CoordType operator[](const int i) const {
        return mData[i];
    }
#endif
    void append(const CoordType a) {
        mData.push_back(a);
    }

    virtual int size() const {
        return mData.size();
    }

    class iterator
    {
    public:
        typename vector<CoordType>::iterator mI;

        iterator operator++(int dummy) {
            mI++;
            return *this;
        }

        CoordType operator*() {
            return *mI;
        }

        bool operator!=(const iterator &rhs) {
            return (mI != rhs.mI);
        }
    };

    iterator begin() {
        iterator I;
        I.mI = mData.begin();
        return I;
    }

    const iterator end() {
        iterator I;
        I.mI = mData.end();
        return I;
    }
};

template<class CoordType>
class Rect2D
{
public:
    Point2D<CoordType> mLL;
    Point2D<CoordType> mUR;
    Point2D<CoordType> mSpan;

    Rect2D() {
        zero();
    }

    Rect2D(CoordType xmin, CoordType ymin, CoordType xmax, CoordType ymax) :
        mLL(xmin, ymin), mUR(xmax, ymax), mSpan(xmax-xmin, ymax-ymin) {}

    void zero() {
        mLL.zero();
        mUR.zero();
        mSpan.zero();
    }
};

template<class CoordType, class DataType>
class Grid
{
    Rect2D<CoordType> mRegion;
    Point2D<CoordType> mStep;
    Point2D<int> mNSteps;
    Rect2D<CoordType> mLoadedRegion;
    DataType *mData;
    int mNPts;
    bool mDataOnConnections;
    AttributeMap mAttributes;
    bool mFullyLoaded;

public:

    Grid() {
        mData = NULL;
        zero();
    }

    void zero() {
        mRegion.zero();
        mStep.zero();
        mNSteps.zero();
        mLoadedRegion.zero();
        mNPts = 0;
        mFullyLoaded = false;
        mDataOnConnections = false;
        if (mData)
            delete [] mData;
    }

    void init(CoordType xmin, CoordType ymin, CoordType xmax, CoordType ymax, CoordType dx, CoordType dy) {
        mRegion = Rect2D<CoordType>(xmin, ymin, xmax, ymax);
        mStep = Point2D<CoordType>(dx, dy);
        init();
    }


    void init() {
        mNSteps.mX = (int)(mRegion.mSpan.mX/mStep.mX);
        mNSteps.mY = (int)(mRegion.mSpan.mY/mStep.mY);
        mNPts = mNSteps.mX*mNSteps.mY;
        mData = new DataType[mNPts];
    }

    // Subregion
    // Subregion iterator (line by line)
    // line iterator

};

typedef Coord<Coord<float> > CoordMesh;

typedef CoordIrregular<CoordIrregular<float> > CoordIrregMesh;

// typedef CoordRegular<CoordRegular<float> > CoordRegularMesh;

template <class CoordType>
class CoordRegularMesh
{
    CoordRegular<CoordType> mX;
    CoordRegular<CoordType> mY;

    class iterator
    {
    public:
        int mI;
        CoordType mIMin;
        double mIDenom;
        int mIN;
        iterator operator++(int dummy) {
            mI++;
            if (mI < mIN) {
                return *this;
            }
            mI = -1;
            return *this;
        }
        CoordType operator*() {
            return static_cast<CoordType>(mIMin) + static_cast<CoordType>(mI)*static_cast<CoordType>(mIDenom);
        }
        bool operator!=(const iterator &rhs) {
            return (mI != rhs.mI);
        }
    };

    iterator begin() {
        iterator I;
        I.mI = 0;
        I.mIMin = mMin;
        I.mIDenom = mDenom;
        I.mIN = mCount;
        return I;
    }

    const static iterator end() {
        iterator I;
        I.mI = -1;
        return I;
    }


};

class LCMSRawFile : public AttributeMap
{
    // load file, determine if raw or not
    // load header attributes
    // create appropriate grid

    // want to iterate over time and mass - getting values.
    // want to extract sub regions and remap, filter.
    // want to work with regular and irregular grids - all with single block of data.

    CoordRegularMesh<float> mGrid;

public:

    void load(char *fname);
};

// could  be 1d or 2d, but has data and location bound
template<class CoordType, class DataType>
class DataPoint
{
    CoordType mPosition;
    DataType mValue;

public:
    DataType getValue() {
        return mValue;
    }
    CoordType getPosition() {
        return mPosition;
    }
    void setValue(const DataType v) {
        mValue = v;
    }
    void setPosition(const CoordType p) {
        mPosition = p;
    }
};

// assumed to be row-major
// Y is single value
// X is regular array
// Data in separate vector array
template<class CoordType, class DataType>
class CoordListReg : AttributeMap
{
    CoordType mY;
    CoordRegular<CoordType> mcrX;
    vector<DataType> *mpvData;
};

// assumed to be row-major
// Y is single value
// X is irregular array
// Data in separate array
template<class CoordType, class DataType>
class CoordListIrreg : AttributeMap
{
    CoordType mY;
    CoordIrregular<CoordType> *mpvX;
    vector<DataType> *mpvData;
};

// assumed to be row-major
// irreg, irreg
template<class CoordType, class DataType>
class DataMeshIrregIrreg : AttributeMap
{
    vector<CoordListIrreg<CoordType, DataType> > mMesh;
};

// assumed to be row-major
template<class CoordType, class DataType>
class DataMeshRegIrreg : AttributeMap
{
    vector<CoordListReg<CoordType, DataType> > mMesh;
};

// assumed to be row-major
template<class CoordType, class DataType>
class DataMeshRegReg : AttributeMap
{
    CoordRegular<CoordType> mcrY;
    CoordRegular<CoordType> mcrX;
    DataType *mData;
};

template<class CoordType, class DataType>
class DataMeshCursor
{
    DataPoint<CoordType, DataType> operator*() {
        DataPoint<CoordType, DataType> dp;
        dp.setPosition(getPosition());
        dp.setValue(getValue());
        return dp;
    }

    DataPoint<CoordType, DataType> &operator++(int dummy) {
    }

    DataType getValue() {
        return *mValue;
    }

    CoordType getPosition() {
        return *mPosition;
    }
    
};

#endif