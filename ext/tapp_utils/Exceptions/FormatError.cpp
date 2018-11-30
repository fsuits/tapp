#include "Exceptions/FormatError.h"

namespace TAPP::Exceptions
{
	FormatError::FormatError(const std::string message) : std::runtime_error(message)
	{
	}
}
