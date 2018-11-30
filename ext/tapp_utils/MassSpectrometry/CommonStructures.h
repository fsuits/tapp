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