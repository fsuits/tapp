// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#ifndef INCLUDE_FSUTIL_H
#define INCLUDE_FSUTIL_H

#include <iostream>

#ifdef WIN32
#define ISWINDOWS
#endif

#ifdef WIN64
#define ISWINDOWS
#endif

#ifdef WIN32
#define LCMSInt64 __int64
#define LCMSInt32 __int32
#else
#define LCMSInt64 long long
#define LCMSInt32 int
#endif

// if FS_LITTLEENDIAN is not set in the compiler flags, the default value of 1 (little endian) is assumed
#if !defined(FS_LITTLEENDIAN)  // this must always be defined
#define FS_LITTLEENDIAN 1
#endif

#if !defined(WIN32) && !defined(WIN64)
#define ISUNIX
#endif

class FSLittleEndian
{
public:
	static int get() {
		int i = 1;
		int littleendian = ((*(const char*)&i) != 0) ? 1 : 0;

		if (littleendian != FS_LITTLEENDIAN) {
			std::cerr << "Endian does not match.  Set FS_LITTLEENDIAN to " << littleendian << " and rebuild." << std::endl;
			std::cerr << "Add -DFS_LITTLEENDIAN=" << littleendian << " to the Makefile CARGS" << std::endl;
			std::cerr << "For performance, FS_LITTLEENDIAN must be set at compile time" << std::endl;
			exit(-1);
		}
		return littleendian;
	}
};

class FSUtil
{
public:

	template <class T> static inline T* ArrayAllocation(const size_t n, const std::string& s)
	{
		T* temp_pointer = nullptr;
		try
		{
			temp_pointer = new T[n];
		}
		catch (...)
		{
			std::cout << "Allocation error - exception caught" << std::endl;
		}
		if (!temp_pointer)
		{
			std::cout << s << " Memory allocation failure: " << n << " x " << sizeof(T) << " = " << n*sizeof(T) << " bytes" << std::endl;
			exit(-1);
		}
		return temp_pointer;
	}

    template <class T> static inline void CheckAlloc(T * &X, int N, char *s)
    {

        X = NULL;
        try {
            X = new T[N];
        } catch (...) {
			std::cout << "Allocation error - exception caught" << std::endl;
        }
        if (!X) {
			std::cout << s << " Memory allocation failure: " << N << " x " << sizeof(T) << " = " << N*sizeof(T) << " bytes" << std::endl;
            exit(-1);
        }
    }

    static inline void CheckZero(int v, char *s)
    {
        if (v <= 0) {
			std::cout << "Value is <=0: " << v << " " << s << std::endl;
            exit(-1);
        }
    }
};

class Swap {
public:

    inline static void MakeInt32(unsigned LCMSInt32& x, int littleendian)  // here littleendian specifies original or final form
    {
        if (littleendian != FS_LITTLEENDIAN) {
            char *buff = (char *)&x;
            char t;

            t = buff[0];
            buff[0] = buff[3];
            buff[3] = t;
            t = buff[1];
            buff[1] = buff[2];
            buff[2] = t;
        }
    }

    inline static void MakeFloat32(float& x, int littleendian)
    {
        if (littleendian != FS_LITTLEENDIAN) {
            char *buff = (char *)&x;
            char t;

            t = buff[0];
            buff[0] = buff[3];
            buff[3] = t;
            t = buff[1];
            buff[1] = buff[2];
            buff[2] = t;
        }
    }

    inline static void MakeInt64(unsigned LCMSInt64& x, bool littleendian)
    {
        if (littleendian != FS_LITTLEENDIAN) {
            char *buff = (char *)&x;
            char t;

            t = buff[0];
            buff[0] = buff[7];
            buff[7] = t;
            t = buff[1];
            buff[1] = buff[6];
            buff[6] = t;
            t = buff[2];
            buff[2] = buff[5];
            buff[5] = t;
            t = buff[3];
            buff[3] = buff[4];
            buff[4] = t;
        }
    }
};

#endif
