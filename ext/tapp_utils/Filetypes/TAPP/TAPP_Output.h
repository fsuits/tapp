// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#pragma once
#include <ostream>

namespace TAPP::Filetypes::TAPP
{
	const static char TAPP_OUTPUT_PRECISION = 10;

	void SetGlobalPrecision(std::ostream& out);
}