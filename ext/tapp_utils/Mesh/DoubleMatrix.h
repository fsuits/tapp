#pragma once
#ifndef DoubleMatrix_h_
#define DoubleMatrix_h_

#include <string>
#include <vector>


class MatrixException {
public:
	MatrixException(std::string msg) {errMessage=msg;}
	std::string errMessage;
};

class DoubleMatrix {
private:
	double **values;
public:
	int nRows; int nCols;

protected:
	DoubleMatrix(double** data, int theNRows, int theNCols);
	double** allocMatrix(int nRows, int nCols);
	void freeMatrix(double** values, int nRows);

public:
	DoubleMatrix() { 
		values = nullptr; 
		nRows = 0;
		nCols = 0;
	}

	DoubleMatrix(int theNRows, int theNCols);

	~DoubleMatrix();

	DoubleMatrix(const DoubleMatrix& theM);
	DoubleMatrix& operator=(const DoubleMatrix& theM);

	void zeroIt();

	int getNRows() { return nRows; }
	int getNCols() { return nCols; }

	inline double& get(int i, int j) { 
		if (i<0||(nRows-1)<i||j<0||(nCols-1)<j)
			throw MatrixException("Index out of range.");
		return values[i][j];
	}

	void dumpMatrix(std::string filename);

    static void dumpMatrix(std::string filename, double** matrix, int rows, int cols);

};

#endif
