// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#pragma once

#ifndef LCMSFILE_H
#define LCMSFILE_H

#include <fstream>
#include <iostream>

#include "LCMSFile/Attribute.h"
#include "Mesh/Mesh.hpp"

#define SEPS " "

enum { MAXFILE = 2048 };
enum { BUFFSIZE = 5000000 };
enum { MAXROW = 2000000 };

#define TOOLVERSION "1.12"

class LCMSFile {
   public:
    char mRawFileName[MAXFILE];
    char mCompressedFileName[MAXFILE];
    char mIndexFileName[MAXFILE];
    char mTICFileName[MAXFILE];
    char mHeaderFileName[MAXFILE];
    char mMetaRegionFileName[MAXFILE];
    unsigned *mRow;
    unsigned *mRowAccum;
    FILE *mRawFile;
    FILE *mCompressedFile;
    FILE *mIndexFile;
    FILE *mTICFile;
    FILE *mHeaderFile;
    FILE *mMetaRegionFile;
    char *mBuffer;
    unsigned char *mOutBuffer;
    enum CompressMode {
        ZEROVALS = 0,
        SMALLVALS = 1,
        MEDVALS = 2,
        BIGVALS = 3,
        STARTRUN = 4,
        ENDLINE = 5
    };
    enum LengthMode { SHORTRUN = 0, MEDRUN = 1, LONGRUN = 2, NODATA = 3 };
    CompressMode mCompressMode;
    LengthMode mLengthMode;
    AttributeMap mAttributes;
    unsigned mRunCount;
    unsigned char *mBytePtr;
    int mLineLength;
    int mTIC;
    float mCurrentTime;
    char mCurrentTimeString[MAXFILE];
    int mNLinesLoaded;
    bool mFirstLine;
    float mStartTime;
    float mEndTime;
    float mStartMass;
    float mEndMass;
    float mMeanDeltaTime;
    float mMeanDeltaMass;
    float mSigmaMass;
    float mSigmaRT;
    float mSubmeshStartMass;
    float mSubmeshStartTime;
    float mSubmeshEndMass;
    float mSubmeshEndTime;
    int mNTimes;
    int mNMasses;
    // Mesh *mpMesh;
    Mesh mMesh;
    int mTexture;

    LCMSFile() {
        strcpy(mRawFileName, "");
        // strcpy(mCompressedFileName, "");
        strcpy(mIndexFileName, "");
        strcpy(mTICFileName, "");
        strcpy(mMetaRegionFileName, "");
        mRawFile = NULL;
        mCompressedFile = NULL;
        mIndexFile = NULL;
        mTICFile = NULL;
        mNLinesLoaded = 0;
        mFirstLine = true;
        mRow = new unsigned[MAXROW];
        mRowAccum = new unsigned[MAXROW];
        mBuffer = new char[BUFFSIZE];
        mOutBuffer = new unsigned char[BUFFSIZE];
        setDefaultAttributes();
    }

    void setDefaultAttributes() {
        mAttributes.add("StartMass", 400.0);
        mAttributes.add("EndMass", 1200.0);
        mAttributes.add("StartTime", 40.0);
        mAttributes.add("EndTime", 200.0);
        mAttributes.add("MeanDeltaMass", 0.05);
        mAttributes.add("MeanDeltaTime", 0.25);
        mAttributes.add("CompilePlatformLittleEndian", FSLittleEndian::get());
    }

    void setAttributes(char *str = "") {
        mAttributes.get("StartMass", mStartMass);
        mAttributes.get("EndMass", mEndMass);
        mAttributes.get("StartTime", mStartTime);
        mAttributes.get("EndTime", mEndTime);
        mAttributes.get("MeanDeltaMass", mMeanDeltaMass);
        mAttributes.get("MeanDeltaTime", mMeanDeltaTime);
        mAttributes.get("NMasses", mNMasses);
        mLineLength = mNMasses;
        mAttributes.get("NLines", mNTimes);

        mAttributes.add("ConversionStartTime", mStartTime);
        mAttributes.add("ConversionEndTime", mEndTime);
        mAttributes.add("ConversionStartMass", mStartMass);
        mAttributes.add("ConversionEndMass", mEndMass);
        mAttributes.add("ConversionMeanDeltaTime", mMeanDeltaTime);
        mAttributes.add("ConversionMeanDeltaMass", mMeanDeltaMass);
        mAttributes.add("ConversionSigmaMass", mSigmaMass);
        mAttributes.add("ConversionSigmaTime", mSigmaRT);
        mAttributes.add("ConversionNPeaksToFind", 100000);
        mAttributes.add("ConversionPeakThreshold", 0.05);
        mAttributes.add("ConversionPeakHeightMin", 3);
        mAttributes.add("ConversionSmoothWindow", 7);
        mAttributes.add("ConversionSmoothMethod", "Gauss");
        mAttributes.add("ConversionMassReduction", 1);
        mAttributes.add("ConversionTimeReduction", 1);
        mAttributes.add("ConversionMassResolution", 0);
        mAttributes.add("ConversionWarpedMesh", 0);
        mAttributes.add("ConversionMassAtSigma", 0);
        mAttributes.add("ConversionInvertXMLEndian", 0);
        mAttributes.add("ConversionMeshLittleEndian", FS_LITTLEENDIAN);
    };

    void setConversionAttributes() {
        std::string s;
        mAttributes.get("ConversionStartTime", mMesh.mConversion.mMinRT);
        mAttributes.get("ConversionEndTime", mMesh.mConversion.mMaxRT);
        mAttributes.get("ConversionStartMass", mMesh.mConversion.mMinMZ);
        mAttributes.get("ConversionEndMass", mMesh.mConversion.mMaxMZ);
        mAttributes.get("ConversionMeanDeltaTime", mMesh.mConversion.mMeanDRT);
        mAttributes.get("ConversionMeanDeltaMass", mMesh.mConversion.mMeanDMZ);
        mAttributes.get("ConversionSigmaMass", mMesh.mConversion.mSigmaMZ);
        mAttributes.get("ConversionSigmaTime", mMesh.mConversion.mSigmaRT);
        mAttributes.get("ConversionMassReduction",
                        mMesh.mConversion.mMZReduction);
        mAttributes.get("ConversionTimeReduction",
                        mMesh.mConversion.mRTReduction);
        mAttributes.get("ConversionNPeaksToFind",
                        mMesh.mConversion.mNPeaksToFind);
        mAttributes.get("ConversionPeakThreshold",
                        mMesh.mConversion.mPeakThreshold);
        mAttributes.get("ConversionPeakHeightMin",
                        mMesh.mConversion.mPeakHeightMin);
        mAttributes.get("ConversionSmoothWindow",
                        mMesh.mConversion.mSmoothWindow);
        mAttributes.get("ConversionSmoothMethod",
                        mMesh.mConversion.mSmoothMethod);
        mAttributes.get("ConversionMassResolution",
                        mMesh.mConversion.mMassResolution);
        mAttributes.get("ConversionWarpedMesh", mMesh.mConversion.mWarpedMesh);
        mAttributes.get("ConversionMassAtSigma", mMesh.mConversion.mMZAtSigma);
        mAttributes.get("ConversionInvertXMLEndian",
                        mMesh.mConversion.mInvertXMLEndian);
        mAttributes.add("ConversionMeshLittleEndian",
                        FS_LITTLEENDIAN);  // need to override any value here.
                                           // mesh is always platform endian
        mAttributes.get("ConversionMeshLittleEndian",
                        mMesh.mConversion.mMeshLittleEndian);
        mAttributes.get("ConversionMassSpecType", s);
        mMesh.mConversion.mMassSpecType = ConversionSpecs::UNKNOWN;
        if (s.size() > 2) {
            if (toupper(s[0]) == 'Q' && toupper(s[1]) == 'U')
                mMesh.mConversion.mMassSpecType = ConversionSpecs::QUAD;
            else if (toupper(s[0]) == 'I')
                mMesh.mConversion.mMassSpecType = ConversionSpecs::IONTRAP;
            else if (toupper(s[0]) == 'F')
                mMesh.mConversion.mMassSpecType = ConversionSpecs::FTICR;
            else if (toupper(s[0]) == 'O')
                mMesh.mConversion.mMassSpecType = ConversionSpecs::ORBITRAP;
            else if (toupper(s[0]) == 'T' || toupper(s[1]) == 'T')
                mMesh.mConversion.mMassSpecType = ConversionSpecs::QTOF;
        }

        if (mMesh.mConversion.mWarpedMesh) {
            if (mMesh.mConversion.mMassSpecType == ConversionSpecs::UNKNOWN) {
                std::cerr
                    << "Use of warped mesh requires ConversionMassSpecType and "
                       "ConversionMassAtSigma"
                    << std::endl;
                std::cerr
                    << "ConversionMassAtSigma specifies the m/z value at which "
                       "the sigma value is correct"
                    << std::endl;
                exit(-1);
            }
            if (mMesh.mConversion.mMassSpecType != ConversionSpecs::QUAD &&
                mMesh.mConversion.mMassSpecType != ConversionSpecs::IONTRAP) {
                if (mMesh.mConversion.mMZAtSigma < 1) {
                    std::cerr
                        << "Use of warped mesh with MassSpec other than "
                           "quad/iontrap requires setting ConversionMassAtSigma"
                        << std::endl;
                    std::cerr << "to the value of m/z where the sigma value is "
                                 "correct."
                              << std::endl;
                    exit(-1);
                }
            }
        }
    }

    // this pulls values from the attribute table
    // now load other values and overwrite selectively
    // return 1 on error
    int loadAttributes(const char *fname = NULL) {
        FILE *f;

        if (!fname || strlen(fname) == 0) {
            std::cerr << "Header file not specified.  Trying Default.hdr"
                      << std::endl;
            fname = "Default.hdr";
        }

        f = fopen(fname, "r");
        // quietly (?) ignore lack of att file
        if (f) {
            while (fgets(mBuffer, BUFFSIZE, f))
                mAttributes.addStoredPair(mBuffer);
            fclose(f);
        } else {
            // std::cerr << "Header file " << fname << " not found or opened.
            // Using defaults." << std::endl;
            return 1;
        }
        return 0;
    }

    void closeFiles() {
        if (mRawFile) fclose(mRawFile);
        if (mCompressedFile) fclose(mCompressedFile);
        if (mIndexFile) fclose(mIndexFile);
        if (mTICFile) fclose(mTICFile);
        if (mHeaderFile) fclose(mHeaderFile);
        if (mIndexFile) fclose(mIndexFile);

        mRawFile = NULL;
        mCompressedFile = NULL;
        mIndexFile = NULL;
        mTICFile = NULL;
        mHeaderFile = NULL;
        mIndexFile = NULL;
    }

    // given raw file name, set other file names and open the files
    int setFileNames(char *name) {
        strcpy(mRawFileName, name);
        sprintf(mTICFileName, "%s.tic", mRawFileName);
        sprintf(mHeaderFileName, "%s.hdr", mRawFileName);
        sprintf(mMetaRegionFileName, "%s.mrg", mRawFileName);
        sprintf(mHeaderFileName, "%s.hdr", mRawFileName);
        return 0;
    }

    // open main file for read, and others for write
    int openFilesForWrite() {
        mRawFile = fopen(mRawFileName, "r");
        if (!mRawFile) {
            std::cerr << "Error opening raw file " << mRawFileName << std::endl;
            exit(-1);
        }

        mCompressedFile = fopen(mCompressedFileName, "wb");
        if (!mCompressedFile) {
            std::cerr << "Error opening compressed file " << mCompressedFileName
                      << std::endl;
            exit(-1);
        }

        mIndexFile = fopen(mIndexFileName, "w");
        if (!mIndexFile) {
            std::cerr << "Error opening index file " << mIndexFileName
                      << std::endl;
            exit(-1);
        }

        mTICFile = fopen(mTICFileName, "w");
        if (!mTICFile) {
            std::cerr << "Error opening TIC file " << mTICFileName << std::endl;
            exit(-1);
        }

        mHeaderFile = fopen(mHeaderFileName, "w");
        if (!mHeaderFile) {
            std::cerr << "Error opening Header file " << mHeaderFileName
                      << std::endl;
            exit(-1);
        }

        return 0;
    }

    int openFilesForRead() {
        mCompressedFile = fopen(mCompressedFileName, "rb");
        if (!mCompressedFile) {
            std::cerr << "Error opening compressed file " << mCompressedFileName
                      << std::endl;
            exit(-1);
        }

        mIndexFile = fopen(mIndexFileName, "rb");
        if (!mIndexFile) {
            std::cerr << "Error opening index file " << mIndexFileName
                      << std::endl;
            exit(-1);
        }

        mTICFile = fopen(mTICFileName, "rb");
        if (!mTICFile) {
            std::cerr << "Error opening TIC file " << mTICFileName << std::endl;
            exit(-1);
        }

        mHeaderFile = fopen(mHeaderFileName, "rb");
        if (!mTICFile) {
            std::cerr << "Error opening Header file " << mHeaderFileName
                      << std::endl;
            exit(-1);
        }

        return 0;
    }

    // ASCIIFILESUBREGIONEXTRACTION
    int asciiSubRegion(float m1, float m2, float t1, float t2, int &flag,
                       int &switcher) {
        //#define SEPS ","
        mCompressMode = STARTRUN;
        mBytePtr = mOutBuffer;
        mTIC = 0;
        int newnmasses = mNMasses * (m2 - m1) / (mEndMass - mStartMass);
        int mskip = mNMasses * (m1 - mStartMass) / (mEndMass - mStartMass);
        int umskip = mNMasses * (m2 - mStartMass) / (mEndMass - mStartMass);
        // if (mpMesh->nx+mskip > mNMasses)
        //      mpMesh->nx = mNMasses-mskip;
        int count = 0;

        FILE *fraw = mRawFile;
        if (switcher == 0) {
            fprintf(mMetaRegionFile, "%f\t%f\t%d\n", m1, mMeanDeltaMass,
                    newnmasses);
            switcher = 1;
        }
        switcher = 1;
        char *s = fgets(mBuffer, BUFFSIZE, fraw);
        if (!s) {
            // cout<<"over\n";
            return 0;
        }

        char *p = strtok(mBuffer, SEPS);
        strcpy(mCurrentTimeString, p);
        mCurrentTime = convertedTime(atof(p));

        if (mCurrentTime >= t1) {
            flag = 1;
            if (mCurrentTime <= t2) {
                fprintf(mMetaRegionFile, "%f  ", mCurrentTime);
                // cout<<mCurrentTime<<" ";
                flag = 0;
                // check for mass range and get the masses printed
                // cout<<"True\n";
                p = strtok(NULL, SEPS);  // +
                p = strtok(NULL, SEPS);  // ESI
                p = strtok(NULL, SEPS);  // ms1
                p = strtok(NULL, SEPS);  // -
                p = strtok(NULL, SEPS);  // profile or line
                p = strtok(NULL, SEPS);  // 100.0-1500.0
                p = strtok(NULL, SEPS);  // 22401/0.0625
                while (p = strtok(NULL, SEPS)) {
                    if (count >= mskip && count < umskip) {
                        int k = atoi(p);
                        fprintf(mMetaRegionFile, "%d  ", k);
                        // cout<<k<<" ";
                    }
                    count++;
                }
                fprintf(mMetaRegionFile, "\n");
                // cout<<endl;
                return 1;
            } else
                return 0;
        } else
            return 1;
    }

    // ASCIIFILESUBREGIONEXTRACTION from non compressed data
    int asciiSubRegionDatFile(float m1, float m2, float t1, float t2,
                              Mesh *mpMesh, char *fNameWrite, bool append) {
        // first get x and y coordinates
        int startMassIndex = 0, endMassIndex = 0, startTimeIndex = 0,
            endTimeIndex = 0;

#if 0
	if (mpMesh->xmin <= m1)
		startMassIndex = mpMesh->xindex(m1);
	else
		startMassIndex = mpMesh->xindex(mpMesh->xmin);
	if (mpMesh->xmax >= m2)
		endMassIndex = mpMesh->xindex(m2);
	else
		endMassIndex = mpMesh->xindex(mpMesh->xmax);
	if (mpMesh->ymin <= t1)
		startTimeIndex = mpMesh->yindex(t1);
	else
		startTimeIndex = mpMesh->yindex(mpMesh->ymin);
	if (mpMesh->ymin >= t2)
		endTimeIndex = mpMesh->yindex(t2);
	else
		endTimeIndex = mpMesh->yindex(mpMesh->ymax);
#endif
        if ((endTimeIndex <= startTimeIndex) ||
            (startMassIndex > endMassIndex)) {
            std::cerr << "Bad region parameters! Exiting!" << std::endl;
            return -1;
        }

        // cout << "startMassIndex: " << startMassIndex << std::endl;
        // cout << "endMassIndex: " << endMassIndex << std::endl;
        // cout << "startTimeIndex: " << startTimeIndex << std::endl;
        // cout << "endTimeIndex: " << endTimeIndex << std::endl;

        std::string fileNameWrite = fNameWrite;
        fileNameWrite.append(".txt");

        int p;
        FILE *pFile;
        if (append)
            pFile = fopen(fileNameWrite.c_str(), "a");
        else
            pFile = fopen(fileNameWrite.c_str(), "w");
        if (pFile != NULL) {
            fputs("Intensity\n", pFile);
            for (int i = startMassIndex; i <= endMassIndex; i++) {
                for (int j = startTimeIndex; j <= endTimeIndex - 1; j++) {
                    p = mpMesh->Index(i, j);
                    if (mpMesh->v[p] > 0)
                        fprintf(pFile, "%1.12e%s", mpMesh->v[p], SEPS);
                    else
                        fprintf(pFile, "%g%s", mpMesh->v[p], SEPS);
                }
                p = mpMesh->Index(i, endTimeIndex);
                if (mpMesh->v[p] > 0)
                    fprintf(pFile, "%1.12e%s", mpMesh->v[p], "\n");
                else
                    fprintf(pFile, "%g%s", mpMesh->v[p], "\n");
            }
#if 0
		fputs("Mass\n", pFile);
		for (int i = startMassIndex; i <= endMassIndex; i++) {
			if (i != endMassIndex)
				fprintf(pFile, "%1.12e%s", (mpMesh->xmin + (mpMesh->dx)*i), SEPS);
			else
				fprintf(pFile, "%1.12e%s", (mpMesh->xmin + (mpMesh->dx)*i), "\n");
		}
		fputs("Retention time\n", pFile);
		for (int j = startTimeIndex; j <= endTimeIndex; j++) {
			if (j != endTimeIndex)
				fprintf(pFile, "%1.12e%s", (mpMesh->ymin + (mpMesh->dy)*j), SEPS);
			else
				fprintf(pFile, "%1.12e%s", (mpMesh->ymin + (mpMesh->dy)*j), "\n");
		}
#endif
            fputs("Parameters\n", pFile);
            fprintf(pFile, "%1.12e%s%1.12e%s%1.12e%s%1.12e%s", m1, SEPS, m2,
                    SEPS, t1, SEPS, t2, "\n");
            fprintf(pFile, "%1.12e%s%1.12e%s%1.12e%s%1.12e%s", ((m1 + m2) / 2),
                    SEPS, (m2 - (m1 + m2) / 2), SEPS, t1, SEPS, t2, "\n");
            fputs("File name\n", pFile);
            fputs(mpMesh->fname, pFile);
            fputs("\n\n", pFile);
            fclose(pFile);
        }

        return 0;
    }

    // extract multiple region in ascci from dat file
    // useful to extract multiple EICs
    int asciiSubRegionMultipleDatFile(char *fName, char *writeAsciiFName) {
        std::string dummyLine, TICFileName = "", hdrName;
        std::ifstream myfile(fName);
        Mesh *m = NULL;
        int n = 0;
        bool append = false, fileset = false;
        float t1, t2, m1, m2, central, range;
        if (myfile.is_open()) {
            while (myfile.good()) {
                getline(myfile, dummyLine);
                // cout << dummyLine << std::endl;
                n = dummyLine.find(' ');
                if (!dummyLine.substr(0, n).compare("File")) {
                    TICFileName = dummyLine.substr(n + 1);
                    n = TICFileName.rfind('.');
                    if (n >= 0) {
                        hdrName = TICFileName.substr(0, n + 1) + "hdr";
                        if (this->loadAttributes(hdrName.c_str())) {
                            std::cerr << "Header file " << hdrName
                                      << " not found." << std::endl;
                        }
                    }

                    this->setConversionAttributes();

                    this->setFileNames(
                        const_cast<char *>(TICFileName.substr(0, n).c_str()));
                    if (append) {
                        delete m;
                        m = NULL;
                    }
                    // if (!(m = new Mesh(const_cast<char
                    // *>(TICFileName.c_str()), this))) 	cerr << "Not
                    // enough memory!"; this->mpMesh = m;
                    fileset = true;
                }

                if (!fileset) {
                    n = 0;
                    central =
                        atof(dummyLine.substr(n, dummyLine.find(" ", n) - n)
                                 .c_str());
                    n = dummyLine.find(" ", n);
                    range =
                        atof(dummyLine.substr(n, dummyLine.find(" ", n + 1) - n)
                                 .c_str());
                    n = dummyLine.find(" ", n + 1);
                    m1 = central - range;
                    m2 = central + range;
                    t1 =
                        atof(dummyLine.substr(n, dummyLine.find(" ", n + 1) - n)
                                 .c_str());
                    n = dummyLine.find(" ", n + 1);
                    t2 =
                        atof(dummyLine.substr(n, dummyLine.find(" ", n + 1) - n)
                                 .c_str());
                    if (!strcmp(writeAsciiFName, ""))
                        this->asciiSubRegionDatFile(m1, m2, t1, t2, m,
                                                    this->mRawFileName, append);
                    else
                        this->asciiSubRegionDatFile(m1, m2, t1, t2, m,
                                                    writeAsciiFName, append);
                    append = true;
                }
                fileset = false;
            }
        }
        myfile.close();

        return 0;
    }

    void setTimeConversionFactor(float f) {
        mMesh.mConversion.mRTReduction = f;
    }

    float convertedTime(float t) { return mMesh.mConversion.mRTReduction * t; }

    void addStandardAttributes(char *str = "") {
        struct tm *newtime;
        time_t aclock;

        time(&aclock);                 // Get time in seconds
        newtime = localtime(&aclock);  // Convert time to struct tm form
        char buff[100];
        strcpy(buff, asctime(newtime));
        char *lf = &buff[strlen(buff) - 1];
        while (lf >= buff && !isprint(*lf)) *lf-- = '\0';
        char buff2[512];
        sprintf(buff2, "ConversionDate_%s", str);

        mAttributes.addUnique(buff2, buff);
        mAttributes.add("ToolVersion", TOOLVERSION);
    }

    void setMeshAttributes() {
        mAttributes.add("StartMass", mMesh.rawxmin);
        mAttributes.add("EndMass", mMesh.rawxmax);
        mAttributes.add("StartTime", mMesh.rawymin);
        mAttributes.add("EndTime", mMesh.rawymax);
        mAttributes.add("MeanDeltaMass", mMesh.rawmeandx);
        mAttributes.add("MeanDeltaTime", mMesh.rawmeandy);
    }

    void dumpHeader() { mAttributes.dumpFile(mHeaderFile); }

    void dumpHeader(char *fname) {
        FILE *f = fopen(fname, "w");
        mAttributes.dumpFile(f);
        fclose(f);
    }
};
#endif
