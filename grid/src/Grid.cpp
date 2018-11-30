#include <iostream>

#include "LCMSFile/LCMSFile.h"
#include "Utilities/Sanitization.h"
#include "Utilities/StringManipulation.h"

void SetInputOutputPaths(std::string &input, std::string &output,
                         std::string &output_stem) {
    input = TAPP::Utilities::SanitizeFilepath(input);
    output += "/" + output_stem +
              TAPP::Utilities::StringManipulation::FilepathToFilename(input);
    TAPP::Utilities::SanitizeFilepath(output);
    output = output.substr(0, output.find_first_of('.'));
}

int main(int argc, char *argv[]) {
    LCMSFile mLCMS;
    AttributeMap am;

    if (argc < 3) {
        std::cout << argv[0]
                  << " [-compressxml] [-hdr name.hdr] [-outstem stem] [-dump] "
                     "[-index] [-outdir dir] LCMSFileName.mzXML "
                  << std::endl;
        std::cout << std::endl;
        std::cout << "To build index file simply do " << argv[0]
                  << " -index fname.mzXML" << std::endl;
        std::cout << "other options:" << std::endl
                  << "1. -asciiregionMultipleDatFile TICDataFileName "
                     "[TICasciiFileName]"
                  << std::endl
                  << "TICDataFileName is a text file containing information to "
                     "extract regions from .dat files. it contains line with "
                     "\"File FileName.mesh\""
                     "and \"[m/z] [m/z width] [rt start] [rt end]\""
                  << std::endl;

        exit(-1);
    }

    // Used to define an output directory.
    std::string output_directory("");

    FSLittleEndian::get();

    am.add("hdr", "Default.hdr");
    am.add("compression", "none");
    am.add("dump", 0);
    am.add("index", 0);
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-index")) {
            am.add("index", 1);
        } else if (!strcmp(argv[i], "-hdr")) {
            am.add("hdr", argv[i + 1]);
            i++;
        } else if (!strcmp(argv[i], "-compressxml")) {
            am.add("compression", argv[i]);
        } else if (!strcmp(argv[i], "-outstem")) {
            am.add("outstem", argv[i + 1]);
            i++;
        } else if (!strcmp(argv[i], "-dump")) {
            am.add("dump", 1);
        } else if (!strcmp(argv[i], "-asciiregionMultipleDatFile")) {
            am.add("asciiregionMultipleDatFile", argv[i + 1]);
            i++;
            if (argv[i + 1]) {
                am.add("asciiregionWriteFileName", argv[i + 1]);
                i++;
            } else {
                am.add("asciiregionWriteFileName", "");
            }
        } else if (!strcmp(argv[i], "-asciiregionDatFile")) {
            am.add("asciiregionDatFile", 1);
            am.add("asciiregionT1", atof(argv[i + 1]));
            i++;
            am.add("asciiregionT2", atof(argv[i + 1]));
            i++;
            am.add("asciiregionM1", atof(argv[i + 1]));
            i++;
            am.add("asciiregionM2", atof(argv[i + 1]));
            i++;
        } else if (std::string(argv[i]) == "-outdir") {
            ++i;
            output_directory = argv[i];
        } else if (*argv[i] != '-') {
            am.add("fname", argv[i]);
        } else {
            std::cerr << "Unrecognized option " << argv[i] << " ... quitting"
                      << std::endl;
            exit(-1);
        }
    }

    std::string stem;
    if (!am.get("outstem", stem)) {
        std::string ff;
        am.get("fname", ff);
        int n = ff.rfind('.');
        if (n >= 0) ff[n] = '\0';
        n = ff.rfind('/');
        if (n >= 0) ff = ff.substr(n + 1);
        n = ff.rfind('\\');
        if (n >= 0) ff = ff.substr(n + 1);
        n = ff.rfind(':');
        if (n >= 0) ff = ff.substr(n + 1);

        am.add("outstem", ff);
    }

    int indx;
    am.get("index", indx);

    if (indx) {
        std::string fstem;
        am.get("outstem", fstem);
        mLCMS.mMesh.buildIndex(fstem.c_str());
        exit(0);
    }

    // when writing mesh, set lcms file name to xml and load it.  mesh name has
    // outname as do stem etc. when writing,
    if (am.MatchesString("compression", "-compressxml")) {
        mLCMS.setAttributes();
        std::string hdr;
        am.get("hdr", hdr);
        mLCMS.loadAttributes(hdr.c_str());
        mLCMS.setConversionAttributes();
        char cmdstr[2048];
        strcpy(cmdstr, argv[0]);
        for (int i = 1; i < argc; i++) {
            strcat(cmdstr, " ");
            strcat(cmdstr, argv[i]);
        }
        mLCMS.mAttributes.add("ConversionCommandLineGrid", cmdstr);

        std::string input_path, output_path, output_stem;
        am.get("fname", input_path);
        am.get("outstem", output_stem);
        am.get("texture", mLCMS.mTexture);

        // Still messy, entire main file should be cleaned up at some point.
        output_path = output_directory;
        SetInputOutputPaths(input_path, output_path, output_stem);

        std::cout << "Input path: " << input_path << std::endl
                  << "Output path: " << output_path << std::endl;

        // this used to allocate full mesh but now will do out of core
        mLCMS.mMesh.initUsingConversion(
            input_path, output_path,
            false);  // create a mesh with the specified name.  don't do
                     // anything about the source xml.

        if (!output_directory.empty()) {
            strcpy(mLCMS.mMesh.indexname,
                   std::string(output_directory + "/" +
                               std::string(mLCMS.mMesh.indexname))
                       .c_str());
        }

        // set sigmas for splat
        mLCMS.mMesh.setSigmas();

        std::cout << "Created mesh " << mLCMS.mMesh.meshname
                  << " with m/z and t range " << mLCMS.mMesh.mConversion.mMinMZ
                  << "->" << mLCMS.mMesh.mConversion.mMaxMZ << " "
                  << mLCMS.mMesh.mConversion.mMinRT << "->"
                  << mLCMS.mMesh.mConversion.mMaxRT << std::endl;
        std::cout << "mstep, tstep "
                  << (mLCMS.mMesh.mConversion.mMaxMZ -
                      mLCMS.mMesh.mConversion.mMinMZ) /
                         (mLCMS.mMesh.mConversion.mNMZ - 1)
                  << " "
                  << (mLCMS.mMesh.mConversion.mMaxRT -
                      mLCMS.mMesh.mConversion.mMinRT) /
                         (mLCMS.mMesh.mConversion.mNRT - 1)
                  << std::endl;
        std::cout << "Using sigmas " << mLCMS.mMesh.mConversion.mSigmaMZ << " "
                  << mLCMS.mMesh.mConversion.mSigmaRT << std::endl;
        std::cout << "loading data file" << std::endl;

        std::string xname;
        am.get("fname", xname);

        bool DoDump = false;
        int dmp;
        am.get("dump", dmp);
        if (dmp == 1) DoDump = true;

        mLCMS.mMesh.loadXML(xname.c_str(), DoDump);

        std::cout << "Data loaded.  signal min, max, mean: " << mLCMS.mMesh.vmin
                  << " " << mLCMS.mMesh.vmax << " " << mLCMS.mMesh.vmean
                  << std::endl;

        mLCMS.mMesh.mConversion.Dump(mLCMS.mMesh.meshname, mLCMS.mMesh.datname);

        mLCMS.mMesh.dumpHisto();

        mLCMS.addStandardAttributes("Grid");
        mLCMS.setMeshAttributes();
        mLCMS.dumpHeader(mLCMS.mMesh.headername);

        exit(0);
    }

    int paramFile = 0;
    am.get("asciiregionDatFile", paramFile);
    if (paramFile == 1) {
        float t1, t2, m1, m2;
        am.get("asciiregionT1", m1);
        am.get("asciiregionT2", m2);
        am.get("asciiregionM1", t1);
        am.get("asciiregionM2", t2);
        std::string fName;
        std::string hdrName;
        am.get("fname", fName);
        int n = fName.rfind('.');
        if (n >= 0) {
            hdrName = fName.substr(0, n + 1) + "hdr";
            if (mLCMS.loadAttributes(hdrName.c_str())) {
                std::cerr << "Header file " << hdrName << " not found."
                          << std::endl;
            }
        }

        mLCMS.setConversionAttributes();

        mLCMS.setFileNames(const_cast<char *>(fName.substr(0, n).c_str()));
        exit(0);
    }

    std::string fName;
    am.get("asciiregionMultipleDatFile", fName);
    if (!fName.empty()) {
        std::string writeFileName;
        am.get("asciiregionWriteFileName", writeFileName);
        mLCMS.asciiSubRegionMultipleDatFile(
            const_cast<char *>(fName.c_str()),
            const_cast<char *>(writeFileName.c_str()));
        exit(0);
    }
}
