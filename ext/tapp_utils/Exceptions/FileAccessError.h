#pragma once
#include <stdexcept>
#include <string>

namespace TAPP::Exceptions
{
	/// <summary>An exception representing a failed attempt at accessing a file.</summary>
	class FileAccessError : public std::runtime_error
	{
		public:
			/// <summary>Constructs a FileAccessError exception.</summary>
			/// <param name="message">The message the exception should carry.</param>
			FileAccessError(const std::string message);
	};
}