// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#include "Filetypes/TAPP/TAPP_Output.h"

namespace TAPP::Filetypes::TAPP
{
	void SetGlobalPrecision(std::ostream& out)
	{
		out.precision(TAPP_OUTPUT_PRECISION);
	}
}
