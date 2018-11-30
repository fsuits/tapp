#include "Exceptions/FileAccessError.h"

namespace TAPP::Exceptions
{
	FileAccessError::FileAccessError(const std::string message) : std::runtime_error(message)
	{
	}
}
