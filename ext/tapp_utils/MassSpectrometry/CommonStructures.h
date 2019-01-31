// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#pragma once

namespace TAPP::MassSpectrometry
{
	/*	RegionOfInterest
		<summary>Defines a region in the mass to charge and retention time dimensions.</summary>
	*/
	struct RegionOfInterest
	{
		double mz_max;
		double mz_min;
		double rt_max;
		double rt_min;
	};
}