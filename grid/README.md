# How to build

Clone this repository and initialize the submodules:

```
git clone http://129.125.166.241/tapp/grid
cd grid
git submodule init
git submodule update
```

## Linux/Mac

Go to the build folder to perform an out-of-source build:

```
cd build
cmake ..
make
```

Or using the Ninja build system:

```
cd build
cmake .. -G Ninja
ninja
```

## Windows

Open the `CMakeLists.txt` with Visual Studio and build normally. If you wish to
specify launch parameters do so in the `CMakeSettings.json` file. This file will
be generated when you select `CMake -> Change CMakeSettings -> CMakeLists.txt` on
the top menu. For more information you can visit [the following link][1].

# Usage

The following parameters are necessary for the program to work properly. Due to
a current bug, if the `-outdir` path is not an absolute path, the program will run
but no files will be generated.

```
./bin/Grid -compressxml -hdr <path-to-hdr-file> -outdir <absolute-path-to-output-directory> <path-to-mzxml-file>
```

[1]: https://blogs.msdn.microsoft.com/vcblog/2016/10/05/cmake-support-in-visual-studio/
