#pragma once
#include "Mesh/Mesh.hpp"

namespace TAPP::MassSpectrometry
{
	inline bool AscendingIntensity(Peak* a, Peak* b)
	{
		return a->mHeight < b->mHeight;
	}

	inline bool DescendingIntensity(Peak* a, Peak* b)
	{
		return a->mHeight > b->mHeight;
	}

	inline bool AscendingMassToCharge(Peak* a, Peak* b)
	{
		return a->mX < b->mX;
	}

	inline bool DescendingMassToCharge(Peak* a, Peak* b)
	{
		return a->mX > b->mX;
	}

	inline bool AscendingRetentionTime(Peak* a, Peak* b)
	{
		return a->mY < b->mY;
	}

	inline bool DescendingRetentionTime(Peak* a, Peak* b)
	{
		return a->mY > b->mY;
	}
}
