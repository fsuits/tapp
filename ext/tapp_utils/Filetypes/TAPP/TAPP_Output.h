#pragma once
#include <ostream>

namespace TAPP::Filetypes::TAPP
{
	const static char TAPP_OUTPUT_PRECISION = 10;

	void SetGlobalPrecision(std::ostream& out);
}