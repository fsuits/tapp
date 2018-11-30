# How to build

Clone this repository and initialize the submodules:

```
git clone http://129.125.166.241/tapp/meta_match
cd meta_match
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

[1]: https://blogs.msdn.microsoft.com/vcblog/2016/10/05/cmake-support-in-visual-studio/