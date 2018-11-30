#include "Filetypes/TAPP/TAPP_Output.h"

namespace TAPP::Filetypes::TAPP
{
	void SetGlobalPrecision(std::ostream& out)
	{
		out.precision(TAPP_OUTPUT_PRECISION);
	}
}
