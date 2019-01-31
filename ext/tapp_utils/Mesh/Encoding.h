// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#pragma once
#include <iostream>
#include "Mesh/FSUtil.h"

// Base64 alphabet lookup table
// Based on RFC http://www.rfc-editor.org/rfc/rfc4648.txt 
class Encoding {
private:
	static inline constexpr const unsigned char lut[] =
	{ 
		00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,	00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
		00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 62, 00, 00, 00, 63,	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 00, 00, 00, 00, 00, 00,
		00,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 00, 00, 00, 00, 00,
		00, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 00, 00, 00, 00, 00,
		00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,	00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
		00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,	00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
		00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,	00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
		00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,	00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00
	};

    inline static unsigned LCMSInt32 get32(char * &p, int &bit, int littleendian)
    {
		unsigned int b;

		switch (bit) {
			case 0: b = lut[*p++] << 26;
				b |= lut[*p++] << 20;
				b |= lut[*p++] << 14;
				b |= lut[*p++] << 8;
				b |= lut[*p++] << 2;
				b |= lut[*p] >> 4;
				bit = 2;
				break;
			case 2: b = lut[*p++] << 28;
				b |= lut[*p++] << 22;
				b |= lut[*p++] << 16;
				b |= lut[*p++] << 10;
				b |= lut[*p++] << 4;
				b |= lut[*p] >> 2;
				bit = 4;
			break;
			case 4: b = lut[*p++] << 30;
				b |= lut[*p++] << 24;
				b |= lut[*p++] << 18;
				b |= lut[*p++] << 12;
				b |= lut[*p++] << 6;
				b |= lut[*p++];
				bit = 0;
			break;
		}

		Swap::MakeInt32(b, littleendian);
		return b;
	}
 
    inline static unsigned LCMSInt64 get64(char * &p, int &bit, int littleendian)
    {
		unsigned LCMSInt64 b;
		unsigned LCMSInt32 b1, b2;

		switch (bit) {
			case 0: b1 = lut[*p++] << 26;
				b1 |= lut[*p++] << 20;
				b1 |= lut[*p++] << 14;
				b1 |= lut[*p++] << 8;
				b1 |= lut[*p++] << 2;
				b1 |= lut[*p] >> 4;
				bit = 2;
				break;
			case 2: b1 = lut[*p++] << 28;
				b1 |= lut[*p++] << 22;
				b1 |= lut[*p++] << 16;
				b1 |= lut[*p++] << 10;
				b1 |= lut[*p++] << 4;
				b1 |= lut[*p] >> 2;
				bit = 4;
				break;
			case 4: b1 = lut[*p++] << 30;
				b1 |= lut[*p++] << 24;
				b1 |= lut[*p++] << 18;
				b1 |= lut[*p++] << 12;
				b1 |= lut[*p++] << 6;
				b1 |= lut[*p++];
				bit = 0;
			break;
		}

		switch (bit) {
			case 0: b2 = lut[*p++] << 26;
				b2 |= lut[*p++] << 20;
				b2 |= lut[*p++] << 14;
				b2 |= lut[*p++] << 8;
				b2 |= lut[*p++] << 2;
				b2 |= lut[*p] >> 4;
				bit = 2;
				break;
			case 2: b2 = lut[*p++] << 28;
				b2 |= lut[*p++] << 22;
				b2 |= lut[*p++] << 16;
				b2 |= lut[*p++] << 10;
				b2 |= lut[*p++] << 4;
				b2 |= lut[*p] >> 2;
				bit = 4;
				break;
			case 4: b2 = lut[*p++] << 30;
				b2 |= lut[*p++] << 24;
				b2 |= lut[*p++] << 18;
				b2 |= lut[*p++] << 12;
				b2 |= lut[*p++] << 6;
				b2 |= lut[*p++];
				bit = 0;
			break;
		}
		b=(unsigned LCMSInt64)b1<<32;
		b|=b2;
		Swap::MakeInt64(b, littleendian);
		return b;
	}

public:

    inline static void getFloatFloat(char * &p, int &bit, double &f, double &i, int precision, int littleendian)
    {
		if (precision==32) {
			LCMSInt32 b;
			b = get32(p, bit, littleendian);
			f = *(float *)&b;
			b = get32(p, bit, littleendian);
			i = *(float *)&b;
		} else if (precision==64){
			LCMSInt64 b;
			b = get64(p, bit, littleendian);
			f = *(double *)&b;
			b = get64(p, bit, littleendian);
			i = *(double *)&b;
		} else {
			std::cout << "Not handled precision!";
			exit(2);
		}
    }
};
