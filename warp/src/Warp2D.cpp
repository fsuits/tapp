#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <time.h>
#include <assert.h>
#include <float.h>

#define mymin(x,y) ((x<y)?x:y);

#include <iostream>
#include <fstream>
#include <sstream>

#include "Mesh/Mesh.hpp"
#include "Warp2D.h"
#include "PeakWarpDB.h"
#include "PeakMatchDB.h"
#include "Mesh/StringTokenizer.h"

int debug2 = 0;
FILE *f;

Warp2D::Warp2D(int maxPeaksSegmt, int nTPts, int winSize_m, int theSlack_t) {
	//
	debug = false;
	tRangeExpandFactor = 0.20;
	//
	maxNPeaksPSegmt = maxPeaksSegmt;
	nTimePoints = nTPts;
	windowSize_m = winSize_m;
	slack_t = theSlack_t;
}

Warp2D::~Warp2D() {
}

void Warp2D::computeWarp(vector<Peak>& referencePeaks, vector<Peak>& samplePeaks) {
	cow_2D(referencePeaks, samplePeaks, nTimePoints, windowSize_m, slack_t);
}

double Warp2D::similarity2D(double refTimeStart, double refTimeSeg, vector<Peak>& refPeaks,
							double smpTimeStart, double smpTimeSeg, vector<Peak>& smpPeaks) {
	//
	// Warp sample peaks to reference time.
	int smpCount = (int)smpPeaks.size(); 
	for (int j=0; j < smpCount; j++) { 
		Peak& smpPeak = smpPeaks[j];
		// Notice we are modifying the sample peaks time values. The changes will be
		// be visible to the caller.
		smpPeak.mY = (smpPeak.mY-smpTimeStart)*(refTimeSeg)/smpTimeSeg+refTimeStart;
	}
	return similarity2D(refPeaks, smpPeaks);
}

double Warp2D::similarity2D( vector<Peak>& refPeaks,vector<Peak>& smpPeaks) {
    double similVal=0;
    //
    // Compute overlap among all peaks.
    //
    int smpCount = (int)smpPeaks.size(); 

    // if either sample count is small, or max peaks per segment has been specified...
    if (smpCount<100 || maxNPeaksPSegmt>0) {
        int refCount = (int)refPeaks.size(); 
        if (debug2)
            fprintf(f, "similarity %d against %d\n", smpCount, refCount);
        for (int i=0; i<refCount; i++) {
            Peak& refPeak = refPeaks[i];
            for (int j=0; j<smpCount; j++) {
                Peak& smpPeak = smpPeaks[j];
                similVal += peakOverlap(refPeak, smpPeak);
                if (debug2)
                    fprintf(f, "simsum %d %d %f\n", i, j, similVal);
            }
        }
    } else {
        PeakMatchDB *smpDB = new PeakMatchDB(0);
        for (int j=0; j<smpCount; j++) {
            Peak& smpPeak = smpPeaks[j];
            smpDB->addPeak(smpPeak);
        }
        int refCount = (int)refPeaks.size(); 
        for (int i=0; i<refCount; i++) {
            Peak& refPeak = refPeaks[i];
            vector<Peak> mzdummy = smpDB->getPeaksAtMZBand(refPeak.mX,5);  // why 5??

            vector<Peak> &mzBandSmpPeaks = mzdummy;
            int bpkCount = (int)mzBandSmpPeaks.size();
            for (int j=0; j<bpkCount; j++) {
                Peak &smpPeak = mzBandSmpPeaks[j];
                similVal += peakOverlap(refPeak, smpPeak);
            }
        }
        delete smpDB;
    }
    return similVal;
}

double Warp2D::peakOverlap(Peak& peak1,Peak& peak2) {
	// 
	// Integrate the product of two gaussian densities representing the peaks.
	//
	// Retention time direction.
	//

    double ys1 = peak1.mYSig;
    double ys2 = peak2.mYSig;
	double rtsig1_sq = pow(ys1,2);
	double rtsig2_sq = pow(ys2,2);
	double rtsig_sq = (rtsig1_sq*rtsig2_sq)/(rtsig1_sq+rtsig2_sq);

	double rtmu1 = peak1.mY;
	double rtmu2 = peak2.mY;
	double rtmu = (rtmu1*rtsig2_sq+rtmu2*rtsig1_sq)/(rtsig1_sq+rtsig2_sq);

	double rtExpFac=0;
	double rtmudiff = (rtmu*rtmu)/rtsig_sq - (rtmu1*rtmu1)/rtsig1_sq - (rtmu2*rtmu2)/rtsig2_sq;
	if (rtmudiff>-100) {
		rtExpFac = exp(0.5*rtmudiff);
	} else return 0;
	//
	// M/Z direction.
	//
	double mzsig1_sq = pow(peak1.mXSig,2);
	double mzsig2_sq = pow(peak2.mXSig,2);
	double mzsig_sq = (mzsig1_sq*mzsig2_sq)/(mzsig1_sq+mzsig2_sq);

	double mzmu1 = peak1.mX;
	double mzmu2 = peak2.mX;
	double mzmu = (mzmu1*mzsig2_sq+mzmu2*mzsig1_sq)/(mzsig1_sq+mzsig2_sq);

	double mzExpFac=0;
	double mzmudiff = (mzmu*mzmu)/mzsig_sq - (mzmu1*mzmu1)/mzsig1_sq - (mzmu2*mzmu2)/mzsig2_sq;
	if (mzmudiff>-100) {
		mzExpFac = exp(0.5*mzmudiff);
	} else 
		return 0;
	//
	rtExpFac /= sqrt(rtsig1_sq+rtsig2_sq);
	mzExpFac /= sqrt(mzsig1_sq+mzsig2_sq);
	//
	// Intensity of peaks product.
	//
	double overlp = (rtExpFac*mzExpFac*   (peak1.mHeight)*   (peak2.mHeight));

    return overlp;
}

typedef struct FU {
/* Used for dynamic programming: f - cummulative function value, u - position of the optimum predecessor */
	double f;
	int u;
} FU;

bool biggerPeakVolume(const Peak& peak1, const Peak& peak2) {
	return (peak1.mVolume > peak2.mVolume);
}

bool biggerPeakHeight(const Peak& peak1, const Peak& peak2) {
	return (peak1.mHeight > peak2.mHeight);
}

bool lessRTPeak(const Peak& peak1, const Peak& peak2) {
	return (peak1.mY < peak2.mY);
}

vector<Peak> Warp2D::filterPeaks(vector<Peak> thePeaks, int nSegments) {
	//
	// Filter out minor peaks, so there are about maxNPeaksPSegmt peaks
	// in each RT segment.
	// We take the whole peak vector by value so the caller does not see
	// a change of order in the peaks.
	sort(thePeaks.begin(), thePeaks.end(), lessRTPeak);
	vector<Peak> filteredPeaks;
	double rtMin = (*thePeaks.begin()).mY;
	double rtMax = (*--thePeaks.end()).mY;
	double rtSeg = (rtMax-rtMin)/nSegments;
	int k=0;
        int max_k = (int)thePeaks.size();
	for (int i=1; i<=nSegments; i++) {
            double segEnd = rtMin+i*rtSeg;
	    vector<Peak> byHeightPeaks;
            Peak nextPeak;
	    while ((k<max_k) && ((nextPeak=thePeaks[k]).mY <= segEnd)) {
                byHeightPeaks.push_back(nextPeak);
                k++;
            }
	    sort(byHeightPeaks.begin(), byHeightPeaks.end(), biggerPeakHeight);
	    int nTopPeaks = mymin(maxNPeaksPSegmt,byHeightPeaks.size());
	    for (int l=0;l<nTopPeaks;l++)
                filteredPeaks.push_back(byHeightPeaks[l]);
	}
	cout << "Warp2D> Peaks After filter: " << filteredPeaks.size() << " out of " << thePeaks.size() << endl;
	return filteredPeaks;
}

void DumpPeaks(FILE *f, vector<Peak> &p, vector<Peak> &q) {
    fprintf(f, "%5zd %5zd\n", p.size(), q.size());
    int n = max(p.size(), q.size());
    int a0, b0;
    float a1, a2, a3, b1, b2, b3;
    for (int i=0; i<n; i++) {
        a0 = b0 = 0;
        a1 = a2 = a3 = b1 = b2 = b3 = 0;
        if (i < p.size()) {
            Peak *s = &p[i];
            a0 = s->mID;
            a1 = s->mX;
            a2 = s->mY;
            a3 = s->mHeight;
        }
        if (i < q.size()) {
            Peak *s = &q[i];
            b0 = s->mID;
            b1 = s->mX;
            b2 = s->mY;
            b3 = s->mHeight;
        }
        fprintf(f, "%5d %5d %8f %8f %8f %8f %8f %8f\n", a0, b0, a1, b1, a2, b2, a3, b3);
    }
    fprintf(f, "\n");
}

int Warp2D::cow_2D(vector<Peak>& refPeaksIn, vector<Peak>& smpPeaksIn, int nT, int mT, int t) {

    /*
    Warp sample vector P (length nP) by alligning with target vector T (length nT).
    Alignment will use segment length mT and slack t 
    Returns:
    NN - number of segments,
    warp[NN+1] - pointer to warping data (T[i*m] maps to P[warp[i]]) (don't forget to free it)
    corr[NN] - pointer to correlation/segment values: give the quality of warping per segment
    */

    // The number of segments.
    int N = nT/mT;
    nT = N*mT;	
    vector<Peak> filtrdSmpPeaksdummy = filterPeaks(smpPeaksIn,N);
    vector<Peak> filtrdRefPeaksdummy = filterPeaks(refPeaksIn,N);
    vector<Peak>& filtrdRefPeaks = filtrdRefPeaksdummy; vector<Peak>& refPeaks = filtrdRefPeaks;
    vector<Peak>& filtrdSmpPeaks = filtrdSmpPeaksdummy; vector<Peak>& smpPeaks = filtrdSmpPeaks;

    // Create peak DBs for fast lookup on RT.
    //
    PeakWarpDB *refWarpDB = new PeakWarpDB(0);
    for(int i=0;i<(int)refPeaks.size();i++)
        refWarpDB->addPeak(refPeaks[i]);

    PeakWarpDB *smpWarpDB = new PeakWarpDB(1);
    for(int i=0;i<(int)smpPeaks.size();i++)
        smpWarpDB->addPeak(smpPeaks[i]);

    // Select minimum time range containing both spectrums.
    //
    double rtMin = refWarpDB->getPeakMinRT();
    if (rtMin>smpWarpDB->getPeakMinRT())
        rtMin = smpWarpDB->getPeakMinRT();

    double rtMax = refWarpDB->getPeakMaxRT();
    if (rtMax<smpWarpDB->getPeakMaxRT())
        rtMax = smpWarpDB->getPeakMaxRT();

    // Expand the time range by some factor, so we can have warping at the peaks near the
    // range limits (COW will not warp at the range limits).
    rtMin -= (rtMax-rtMin)*tRangeExpandFactor;
    rtMax += (rtMax-rtMin)*tRangeExpandFactor;

    int nP = nT; // Assuming sample has same number of time points as reference.
    double deltaRT = (rtMax-rtMin)/(double)(nT-1); // The minimum time step.

    int *xstart,*xlength,*xend;
    FU **nodes, *node, *next_node;

    time_t time1,time2;

	// Sample segment size.
    int mP = nP/N;
    nP = N*mP;

    printf("cow_2D()> Target segment(window) length: %d Slack: %d\n",mT,t);
    printf("cow_2D()> Target length: %d\n",nT);
    printf("cow_2D()> Sample length: %d\n",nP);
    printf("cow_2D()> Number of segments: %d\n",N);
    printf("cow_2D()> Redefined target length: %d\n",nT);
    printf("cow_2D()> #Ref. Peaks: %zd #Sample Peaks: %zd\n",refPeaks.size(),smpPeaks.size());
    printf("cow_2D()> Minimum time step: %f\n",deltaRT);
    printf("cow_2D()> Time range Min: %f Max: %f\n",rtMin,rtMax);

    /*
    Allocate data structures
    */
    xstart  = (int *) malloc((N+1)*sizeof(int));
    xlength = (int *) malloc((N+1)*sizeof(int));
    xend    = (int *) malloc((N+1)*sizeof(int));

    nodes = (FU **) malloc((N+1)*sizeof(FU *)); // points to rows of F,U  matrix
    time1 = time(NULL);
    int nMatrixEntries=0;
    for(int i=0; i<=N; i++) {
        // Compute range of possible movement for each of the (N+1) nodes.
        // Nodes 0 and N do not move, no warping possible at start or end.
        xstart[i] = max((i*(mP-t)),(nP-(N-i)*(mP+t)));
        xend  [i] = min((i*(mP+t)),(nP-(N-i)*(mP-t)));
        xlength[i]=  xend[i] - xstart[i] + 1;
        //
        if (debug) printf("cow_2D()> i,xstart(i),xlength(i): %d %d %d\n",i,xstart[i],xlength[i]);
        // i-th row of F,U matrix
        nodes[i] = (FU *) malloc(xlength[i]*sizeof(FU));
        nMatrixEntries += xlength[i];
        assert(nodes[i]);
    }
    printf("cow_2D()> Number of dynamic prog. matrix entries: %d\n",nMatrixEntries);

    /*
    Initialize F,U matrix
    */

    for (int i=0;i<=N;i++) {
        node=nodes[i];
        for(int j=0; j<xlength[i]; j++, node++) {
            node->f = -DBL_MAX;
            node->u = -1;
        }
    }


    /*
    Find the best warping using dynamic programming 
    */

    // initialize the only node at level N
    int level = N;
    node = nodes[level];  
    node->f = 0.0;
    node->u = 0;

    if (debug2) {
        f = fopen("PW.dat","w");
        for (int i=0;i<=N;i++) {
            node=nodes[i];
            for(int j=0; j<xlength[i]; j++, node++) {
                fprintf(f, "%4d %4d %f %d\n", i, j, node->f, node->u);
            }
        }

        fprintf(f, "\n");
    }

    unWarpedCorr = 0;
    for( level=N; level>0; level--) {  
        double refTimeSegmt         = deltaRT*mT;
        double refTimeStart         = rtMin+(level-1)*refTimeSegmt;
        vector<Peak> refPeaksdummy  = refWarpDB->getPeaksAtRTBand(refTimeStart+refTimeSegmt/2,refTimeSegmt/2);
        vector<Peak>& refPeaks      = refPeaksdummy;
        vector<Peak> smPeaksdummy   = smpWarpDB->getPeaksAtRTBand(refTimeStart+refTimeSegmt/2,refTimeSegmt/2);
        vector<Peak> &smpPeaks      = smPeaksdummy;
        unWarpedCorr               += similarity2D(refTimeStart, refTimeSegmt, refPeaks, refTimeStart, refTimeSegmt, smpPeaks);
    }

    if (debug2) {
        fprintf(f, "xxx %4s %4s %4s %10s %10s %10s %10s %4s\n", "Levl", "i", "j", "d", "i->f", "sum", "j->f", "j->u");
        fprintf(f, "\n");
    }

    // back-propagate F values
    // go through levels backwards
    // at each level, find range of allowed segments and find correlation
    for( level=N; level>0; level--) {  
        // for each node at given level
        printf("\rProcessing segment : %d          ",level);
        double refTimeSegmt = deltaRT*mT;
        double refTimeStart = rtMin+(level-1)*refTimeSegmt;
        vector<Peak> refPeaksd = refWarpDB->getPeaksAtRTBand(refTimeStart+refTimeSegmt/2,refTimeSegmt/2);
        vector<Peak> &refPeaks = refPeaksd;
        for (int i=0; i<xlength[level]; i++) {
            int xi,xjmin,xjmax,jmin,jmax; FU *nodei;
            // Position of the node at level.
            xi = xstart[level]+i;  // x position of the i-th node at level
            nodei = nodes[level]+i;  // ith node at level
            // The position of a connected node at level-1 is constrained by the position
            // of the node at level and the window size and slack constraints.
            // find all connected nodes at level-1
            xjmin = max(xi-mP-t, xstart[level-1]);
            xjmax = min(xi-mP+t, xend  [level-1]);
            jmin = xjmin-xstart[level-1];
            jmax = xjmax-xstart[level-1];
            double bestf = -DBL_MAX;
            for (int j=jmin; j<=jmax; j++) {
                int xj;
                FU *nodej;
                double d, sum;
                nodej = nodes[level-1]+j;  // jth node at level-1 to be filled in and point to i if best
                // compute distance between connected nodes
                xj = xstart[level-1]+j; // x position of the j-th node at level-1 
                //
                double smpTimeStart    = rtMin+xj*deltaRT;
                double smpTimeSegmt    = (xi-xj) *deltaRT;

                // this makes a copy of the peaks so the time can be changed in the samples
                vector<Peak> smpPeakd  = smpWarpDB->getPeaksAtRTBand(smpTimeStart+smpTimeSegmt/2,smpTimeSegmt/2);
                vector<Peak>& smpPeaks = smpPeakd;
                //
                d = similarity2D(refTimeStart, refTimeSegmt, refPeaks, smpTimeStart, smpTimeSegmt, smpPeaks);
                if (debug2) {
                    fprintf(f, "%5d %5d %5d %f\n", level, i, j, d);
                    DumpPeaks(f, refPeaks, smpPeaks);
                }
                sum = d + nodei->f;
                // store optimum path information
                // this needs to resolve ties...

                if (sum > nodej->f) {
                    if (debug2)
                        fprintf(f, "YES %4d %4d %4d %10f %10f %10f %10f %4d\n", level, i, j, d, nodei->f, sum, nodej->f, nodej->u);
                    nodej->f = sum;
                    nodej->u = i;
                    if (bestf < nodej->f)
                        bestf = nodej->f;
                } else {
                    if (debug2)
                        fprintf(f, " NO %4d %4d %4d %10f %10f %10f %10f %4d\n", level, i, j, d, nodei->f, sum, nodej->f, nodej->u);
                }
            }
            // now need to check for ties and choose the one closest to the middle
            int midj = (jmin+jmax)/2;
            int bestj = midj;
            int bestjdist = 10000000;
            for (int j=jmin; j<=jmax; j++) {
                FU *nodej;
                nodej = nodes[level-1]+j;  // jth node at level-1 to be filled in and point to i if best
            }
        } 
    }
    printf("\n");
    // forward-propagate U values, compute correlation/segment
    int* warping = (int *) malloc((N+1)*sizeof(int));
    assert(warping);

    // here is where the nodes are walked back
    warping[0] = 0;
    int i = 0; totalCorr = 0;
    for (int level=0; level<N; level++) {
        node = nodes[level]+i;
        if (debug2)
            printf("Level %d points to node %d\n", level, node->u);
        i = node->u;
        warping[level+1] = xstart[level+1] + i;   
        next_node = nodes[level+1]+i;
        double sgCorr = node->f - next_node->f;
        totalCorr += sgCorr;
        segmtCorr.push_back(sgCorr);
        if (debug)
            printf("cow_2D()> Segment: %d Correlation: %f\n",level,sgCorr);
    }
    printf("cow_2D()> Total Correlation Un-Warped: %e\n",unWarpedCorr);
    printf("cow_2D()> Total Correlation Warped: %e\n",totalCorr);

    // resample timeP[]
    double sampleTime = rtMin;
    double refrncTime = rtMin;
    origTime.push_back(sampleTime);
    warpedTime.push_back(refrncTime);
    for(i=0; i<N; i++) {
        sampleTime += (warping[i+1]-warping[i])*deltaRT;
        refrncTime += mT*deltaRT;
        origTime.push_back(sampleTime); warpedTime.push_back(refrncTime);
    }

    time2 = time(NULL);
    double dtime = difftime(time2, time1);
    printf("cow_2D()> elapsed time %10.3lg sec\n",dtime);

    if (debug2) {
        // diagnostic output 
        for(level=0; level<N; level++) {
            printf("%d %d %d\n",level,warping[level+1]-warping[level],warping[level]);
        }

        for (int i=0;i<=N;i++) {
            node=nodes[i];
            for(int j=0; j<xlength[i]; j++, node++) {
                fprintf(f, "%4d %4d %f %d\n", i, j, node->f, node->u);
            }
        }
        fclose(f);
    }

    /*
    Free data structures
    */
    for(int i=0; i<=N; i++) free(nodes[i]); free(nodes);
    free(xend); free(xlength); free(xstart);
    free(warping);

    return(1);
}

vector<Peak> Warp2D::warpPeaks(vector<Peak>& thePeaks) {
	vector<Peak> warpedPeaks;
	for(int k=0; k<(int)thePeaks.size(); k++) {
            Peak& sourcePeak = thePeaks[k];
	    sourcePeak.mY = sampleToRefTimeInterpolate(sourcePeak.mY);
	    warpedPeaks.push_back(sourcePeak);
	}
        return warpedPeaks;
}

int Warp2D::binSearch(vector<double>& theVector, double target, int pIdx1, int rIdx1) {
	int m = (rIdx1-pIdx1)/2;
	double testVal1;
	if (m==0) { // r and p differ by one, at most.
		testVal1 = theVector[pIdx1-1];
		if (testVal1==target) return (pIdx1-1);
		//
		double testVal2 = theVector[rIdx1-1];
		if (testVal2==target) 
			return (rIdx1-1);
		//
		// Value is not there exactly, return index to the closest value.
		//
		if (testVal1<target&&target<testVal2) 
			return (pIdx1-1);
		else if (target<testVal1) 
			return (pIdx1-1);
		else if (target>testVal2) 
			return (rIdx1-1);
	} 
		
	testVal1 = theVector[pIdx1-1+m];
	if (testVal1>target)
		return binSearch(theVector, target, pIdx1, pIdx1+m-1);
	else
		return binSearch(theVector, target, pIdx1+m, rIdx1  );
}

double Warp2D::sampleToRefTimeInterpolate(double sampleTQ) {
	int nWarpedPoints = this->origTime.size();
	int idxZ = binSearch(this->origTime, sampleTQ, 1, nWarpedPoints);
	double sampleT = origTime  [idxZ];
	double warpedT = warpedTime[idxZ];

	if (idxZ==0&&sampleTQ<sampleT) {
		warpedT = sampleTQ+(warpedT-sampleT);
	} else if (idxZ<(nWarpedPoints-1)&&sampleT<sampleTQ) {
		double sampleTplus1 = origTime  [idxZ+1];
		double warpedTplus1 = warpedTime[idxZ+1];
		// Linear interpolate to get warped time at query time sampleTQ.
		warpedT = (warpedTplus1-warpedT)*(sampleTQ-sampleT)/(sampleTplus1-sampleT)+warpedT;
	} else if (idxZ==(nWarpedPoints-1)&&sampleTQ>sampleT) {
		warpedT = sampleTQ+(warpedT-sampleT);
	}
	return warpedT;
}


void Warp2D::saveTimeMap2File(string& filename) {
	int nWarpedPoints = this->origTime.size();
	DoubleMatrix timeWarp(nWarpedPoints,2);
	for(int i=0;i<nWarpedPoints;i++) {
		timeWarp.get(i,0)=origTime  [i];
		timeWarp.get(i,1)=warpedTime[i];
	}
	timeWarp.dumpMatrix(filename);
}
