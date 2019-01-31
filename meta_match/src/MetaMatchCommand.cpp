// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <time.h>
#include "Mesh/Mesh.hpp"
#include "MetaMatch.hpp"

using namespace std;

void usage() {
    cout << "MetaMatch FileList.txt OutStem MZRadius TRadius Fraction MZMin TMin MZMax TMax [-stats] [-statsfile fname] [ID NIDS]" << endl;
    cout << "-stats generates a 10x10 array of values, based on varying dmz and drt" << endl;
    cout << "-statsfile reads a file containing a list of dmz drt values and calculates stats for each." << endl;
    cout << "ID NIDs, ie 2 6, indicates this is runid 2 out of 6, i.e. 0-5.  This lets the stats run on multiple computers." << endl;
    exit(0);
}

// MetaMatch File.txt OutStem MZRadius TRadius MZMIn TMin MZMax TMax
// loads all files and matches peaks
// outputs OutStem.h  Outstem.v  Outstem.0h Outstem.0v ... Outstem.kh Outstem.kv
// also output Outstem.stats:  hsd, vsd, msd, tsd
int main(int argc, char * argv[])
{
    char mmcFileList[1024];
    char mmcOutStem[1024];
    double mmcMRadius, mmcTRadius, mmcFraction,mmcXmin, mmcYmin,mmcXmax,mmcYmax;

    if (argc == 2){
        // read arguments from arg.txt
        FILE* argFile = fopen(argv[1],"r");

        if (!argFile){
            printf("Error opening argument file!\n");
            exit(-1);
        } else {
            fscanf(argFile,"%s %s %lf %lf %lf %lf %lf %lf %lf",mmcFileList,mmcOutStem,&mmcMRadius,&mmcTRadius,&mmcFraction,&mmcXmin, &mmcYmin, &mmcXmax, &mmcYmax);
        }
        fclose (argFile);
             
    } else if (argc >= 10){             
        strcpy(mmcFileList,argv[1]);  
        strcpy(mmcOutStem,argv[2]);   
        mmcMRadius = atof(argv[3]);
        mmcTRadius = atof(argv[4]);
        mmcFraction = atof(argv[5]);
        mmcXmin = atof(argv[6]);
        mmcYmin = atof(argv[7]);
        mmcXmax = atof(argv[8]);
        mmcYmax = atof(argv[9]);
    } else {
        usage();
    }

    if (mmcXmin >= mmcXmax || mmcYmin >= mmcYmax) {
        cerr << "Error in parameters: min is greater than max.  Terminating." << endl;
        exit(-1);
    }

    // check for stats
    bool dostats = false;
    bool dostatsfile = false;
    char *statsfilename = NULL;

    int idarg;
    if (argc >10){
        if (!strcmp(argv[10], "-statsfile")) {  // needs 12 args
            dostats = true;
            dostatsfile = true;
            if (argc < 12) {
                cerr << "-statsfile option requires file name" << endl;
                exit(-1);
            }
            statsfilename = argv[11];
            idarg = 12;
        } else if (!strcmp(argv[10], "-stats")) {  // needs 11 args
			dostats = true;
            idarg = 11;
		} else {
			cout << "Option " << argv[10] << " not recognized." << endl;
			usage();
        }
    }

    int nID = 0;
    int NIDs = 1;

    if (argc > 12) {  // needs 13 or 14 args
        nID = atoi(argv[idarg]);
        NIDs = atoi(argv[idarg+1]);
    }
            
    MetaMatch mm(mmcFileList);
                        
    mm.setOutStem(mmcOutStem);

    mm.mMRadius = mmcMRadius;

    mm.mTRadius = mmcTRadius;

    mm.mFraction = mmcFraction;

    mm.mTiling.mMinMZ = mmcXmin;

    mm.mTiling.mMinRT = mmcYmin;

    mm.mTiling.mMaxMZ = mmcXmax;

    mm.mTiling.mMaxRT = mmcYmax;

    mm.mNoiseFloor = 0.5;
               
    if (!dostats) {
		mm.buildTiles();
		int niter = 0;

		while (mm.matchPeaks()) {
            niter++;
			mm.glomOrphans();
        }

		std::sort(mm.mMetaPeaks.begin(), mm.mMetaPeaks.end());

		for (int i=0; i<mm.mMetaPeaks.size(); i++)
			mm.mMetaPeaks[i].mID = i;

        mm.dumpPeaks(mm.mOutStem);
        
    } else {  // dostats
        if (!dostatsfile) {
            int nsteps = 11*11;
            int n1, n2;
            n1 = nID*(nsteps/NIDs);
            n2 = (nID+1)*(nsteps/NIDs);
            
            int nrun = -1;
            for (int i=0; i<11; i++) {
		        for (int j=0; j<11; j++) {
                    nrun++;
                    if ((nrun < n1) || (nrun >= n2))
                        continue;
                    double dm = mmcMRadius/4 + i*(mmcMRadius*4-mmcMRadius/4)/10.0;
                    double dt = mmcTRadius/4 + j*(mmcTRadius*4-mmcTRadius/4)/10.0;
                    cout << "Clustering for dm, dt = " << i << " " << j << ": " << dm << " " << dt << endl;
					mm.initClusterParams(dm, dt, mmcFraction, 1.0);

					mm.buildTiles();

					while (mm.matchPeaks())
						mm.glomOrphans();

					cout << "Dumping peaks" << endl;
					mm.dumpPeaks(mm.mOutStem, true);

					cout << "Resetting peaks" << endl;
					mm.reset();
				}
			}
        } else {
            FILE *f = fopen(statsfilename, "r");
            if (!f) {
                cerr << "Stats file " << statsfilename << " not opened.  Terminating." << endl;
                exit(-1);
            }
            vector<double> vdm, vdt;
            double dm, dt;
            while (!feof(f) && 2 == fscanf(f, "%lf %lf\n", &dm, &dt)) {
                vdm.push_back(dm);
                vdt.push_back(dt);
            }
            fclose(f);
            int nsteps = vdm.size();
            int n1, n2;
            n1 = nID*(nsteps/NIDs);
            n2 = (nID+1)*(nsteps/NIDs);

            for (int n = n1; n<n2; n++) {
                dm = vdm[n];
                dt = vdt[n];
                cout << "Clustering for dm, dt = " << dm << " " << dt << endl;
				mm.initClusterParams(dm, dt, mmcFraction, 1.0);

				mm.buildTiles();

                while (mm.matchPeaks())
                {
                    int nmp, norph;
                    mm.getPeakStats(nmp, norph);
                    cout << "glomming with nmpks, norph: " << nmp << " " << norph << endl;
					mm.glomOrphans();
                }

                cout << "Dumping peaks" << endl;
				mm.dumpPeaks(mm.mOutStem, true);

                cout << "Resetting peaks" << endl;
				mm.reset();
            }
        }
    }

    return 0;
}

