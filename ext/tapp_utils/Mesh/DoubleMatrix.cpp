#include <string>
#include <iostream>
#include <fstream>

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "DoubleMatrix.h"

using namespace std;

DoubleMatrix::~DoubleMatrix() {
	if (values!=nullptr) {
		for (int i=0; i<nRows; i++)
			delete [] values[i];
		delete values; 
		values = nullptr;
	}
}

DoubleMatrix::DoubleMatrix(int theNRows, int theNCols) {
	nRows = theNRows; 
	nCols = theNCols; 
	values = nullptr;
	if (nRows>0 && nCols>0) {
		values = allocMatrix(nRows, nCols);
	}
}

DoubleMatrix::DoubleMatrix(double** data, int theNRows, int theNCols) {
	values = data;
	nRows = theNRows;
	nCols = theNCols;
}

DoubleMatrix::DoubleMatrix(const DoubleMatrix& theM) {
	nRows = theM.nRows;
	nCols = theM.nCols;
	if (nRows > 0 && nCols > 0) {
		values = allocMatrix(nRows, nCols);
		for (int i=0; i < nRows; i++)
			for (int j=0; j < nCols; j++)
				values[i][j] = ((DoubleMatrix&)theM).get(i, j);
	} else
		values = nullptr;
}

DoubleMatrix& DoubleMatrix::operator=(const DoubleMatrix& theM) {
	if (this != &theM) { 
		if (values != nullptr) {
			for (int i=0; i < nRows; i++)
				delete values[i];
			delete values; 
			values = nullptr;
		}
		nRows = theM.nRows;
		nCols = theM.nCols;
		if (nRows > 0 && nCols > 0) {
			values = allocMatrix(nRows, nCols);
			for (int i=0; i < nRows; i++)
				for (int j=0; j < nCols; j++)
					values[i][j] = ((DoubleMatrix&)theM).get(i, j);
		}
	}
	return (*this);
}

void DoubleMatrix::zeroIt() {
	for (int i=0; i < nRows; i++) 
		for(int j=0; j < nCols; j++)
			values[i][j] = 0;
}

void DoubleMatrix::dumpMatrix(string filename) {
	dumpMatrix(filename, values, nRows, nCols);
}

void DoubleMatrix::dumpMatrix(string filename, double** matrix, int rows, int cols) {
	ofstream outFile(filename.c_str());
	for (int i=0; i < rows; i++) {
		if (cols > 0)
			outFile << (float)matrix[i][0];
		for (int j=1; j < cols; j++) { 
			outFile << ('\t');
			outFile << ((float)matrix[i][j]);
		}
		outFile << endl;
	}
	outFile.close();
}

double** DoubleMatrix::allocMatrix(int nRows, int nCols) {
	double **values = new double*[nRows];
	for (int i=0; i < nRows; i++)
		values[i] = new double[nCols];
	return values;
}

void DoubleMatrix::freeMatrix(double** values, int nRows) {
	if (values != nullptr) {
		for(int i=0; i < nRows; i++) 
			delete [] values[i];
		delete values;
	}
}
