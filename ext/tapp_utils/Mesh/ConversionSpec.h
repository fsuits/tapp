#pragma once
#include <cmath>
#include <iostream>
#include <string>

// Mesh is intended to be generic but it helps to have some lcms stuff included
// These refer to parameters used for the conversion process - which is separate from the mesh properties themselves
class ConversionSpecs {
public:
	enum MassSpecType { QUAD = 0, IONTRAP = 1, TOF = 2, QTOF = 3, FTICR = 4, ORBITRAP = 5, UNKNOWN = 99 };
	double mMinRT;
	double mMaxRT;
	double mMinMZ;
	double mMaxMZ;
	double mDMZ;
	double mDRT;
	int mNMZ;
	int mNRT;
	double mMeanDRT;
	double mMeanDMZ;
	double mSigmaMZ;
	double mSigmaRT;
	double mMZAtSigma;
	double mMZReduction;
	double mRTReduction;
	int mSmoothWindow;
	std::string mSmoothMethod;
	int mNPeaksToFind;
	double mPeakThreshold;
	double mPeakHeightMin;
	double mMassResolution;
	int mWarpedMesh;
	MassSpecType mMassSpecType;
	int mInvertXMLEndian;
	int mMeshLittleEndian;

	void Dump(char *meshname, char *datname) {
		double normfactor = 1;
		int donormalize = 0;
		double unitweight = 1;

		FILE *f = fopen(meshname, "w");
		fprintf(f, "# %d %d %lf %lf %f %f\n", mNMZ, mNRT, mDMZ, mDRT, mSigmaMZ, mSigmaRT);
		fprintf(f, "# %lf %lf %lf %lf %lf %d %lf\n", mMinMZ, mMinRT, mMaxMZ, mMaxRT, normfactor, donormalize, unitweight);
		fprintf(f, "# %s\n", datname);
		fprintf(f, "# warping %d resolution %f\n", mWarpedMesh, mMassResolution);
		fprintf(f, "# massspectype %d mzatsigma %lf\n", mMassSpecType, mMZAtSigma);
#if FS_LITTLEENDIAN
		fprintf(f, "# lsb\n");
#else
		fprintf(f, "# msb\n");
#endif
		fclose(f);
	}

	void Load(const char *meshname, const char *datname) {
		double normfactor, unitweight;
		int donormalize;
		FILE *f = fopen(meshname, "r");
		if (!f) {
			std::cout << "Error loading mesh file " << meshname << std::endl;
			exit(-1);
		}
		fscanf(f, "# %d %d %lf %lf %lf %lf\n", &mNMZ, &mNRT, &mDMZ, &mDRT, &mSigmaMZ, &mSigmaRT);
		fscanf(f, "# %lf %lf %lf %lf %lf %d %lf\n", &mMinMZ, &mMinRT, &mMaxMZ, &mMaxRT, &normfactor, &donormalize, &unitweight);
		fscanf(f, "# %s\n", datname);
		if (2 != fscanf(f, "# warping %d resolution %lf\n", &mWarpedMesh, &mMassResolution)) {
			mWarpedMesh = 0;
			mMassResolution = 0;
		}
		if (2 != fscanf(f, "# massspectype %d mzatsigma %lf\n", &mMassSpecType, &mMZAtSigma)) {
			mMassSpecType = UNKNOWN;
			mMZAtSigma = 0;
			std::cout << "Warning - .mesh file does not include mass spec type or MZAtSigma" << std::endl;
		}
		fclose(f);
	}


	/*

	There are three coordinate systems:  Index, Mesh, World

	Index just goes from 0 to nx-1

	Mesh goes from xmin to xmax in steps of dx

	World corresponds to the actual physical units of m/z or rt

	Both Index and Mesh are linear coordinate systems

	But for warped mesh and nonlinear mass-spec, the mapping from Mesh to World is nonlinear

	*/

	// given 0-based index, map to linear mesh coordinate
	// Map 0 to xmin and maxN to xmax
	inline double IndexToMeshX(const double i) const {
		return mMinMZ + i*mDMZ;
	}

	// map 0 to world min and maxN to world max
	inline double IndexToWorldX(const double i) const {
		return MeshToWorldX(IndexToMeshX(i));
	}

	// given linear mesh coordinate from mMinMZ to xmax, map to 0-based index
	inline double MeshToIndexX(const double x) const {
		return (x - mMinMZ) / mDMZ;
	}

	// given world x value, map to 0-based index
	inline double WorldToIndexX(const double x) const {
		return MeshToIndexX(WorldToMeshX(x));
	}

	// given linear mesh coordinate from mMinMZ to xmax, return true m/z
	// mesh coordinate is 0 at MassAtSigma
	inline double MeshToWorldX(const double x) const {
		if (!mWarpedMesh) {
			return x;
		}

		double v1, v2;

		// warped
		switch (mMassSpecType) {
		case ConversionSpecs::QUAD:
		case ConversionSpecs::IONTRAP:
			return x + mMZAtSigma;
			break;
		case ConversionSpecs::TOF:
		case ConversionSpecs::QTOF:
			return mMZAtSigma*exp(mSigmaMZ / mMZAtSigma*x);  // returns MassAtSigma when x 0
			break;
		case ConversionSpecs::ORBITRAP:
			v1 = pow(double(mMZAtSigma), double(1.5));
			v2 = double(1.0 / sqrt(mMZAtSigma) - mSigmaMZ * x / v1);  // v2 = 1/sqrt(MassAtSigma) when x 0
			return 1.0 / (v2*v2);  // returns MassAtSigma when x 0
			break;
		case ConversionSpecs::FTICR:  // Needs to be added
			return 0;
			break;
        case ConversionSpecs::UNKNOWN:
			return 0;
			break;
		}
		return x;
	}

	// Given true x, find linear mesh coordinate
	// this can be negative...
	// MassAtSigma should give 0 for warped mesh, and MassAtSigma for nonwarped
	inline double WorldToMeshX(const double x) const {
		if (!mWarpedMesh) {
			return x;
		}

		switch (mMassSpecType) {
		case ConversionSpecs::QUAD:
		case ConversionSpecs::IONTRAP:
			return x - mMZAtSigma;
			break;
		case ConversionSpecs::TOF:
		case ConversionSpecs::QTOF:
			return mMZAtSigma / mSigmaMZ*log(x / mMZAtSigma);
			break;
		case ConversionSpecs::ORBITRAP:
			return (1.0 / sqrt(mMZAtSigma) - 1.0 / sqrt(x)) * pow(double(mMZAtSigma), double(1.5)) / mSigmaMZ;
			break;
		case ConversionSpecs::FTICR:  // add later
			return 0;
			break;
        case ConversionSpecs::UNKNOWN:
			return 0;
			break;
		}
		return x;
	}

	inline double IndexToMeshY(const double j) const {
		return mMinRT + j*mDRT;
	}

	inline double IndexToWorldY(const double j) const {
		return IndexToMeshY(j);
	}

	inline double MeshToIndexY(const double y) const {
		return (y - mMinRT) / mDRT;
	}

	inline double MeshToWorldY(const double y) const {
		return y;
	}

	inline double WorldToMeshY(const double y) const {
		return y;
	}

	inline double WorldToIndexY(const double y) const {
		return (y - mMinRT) / mDRT;
	}
};
