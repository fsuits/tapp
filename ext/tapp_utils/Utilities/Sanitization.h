#pragma once
#include <string>

namespace TAPP::Utilities
{
	/// <summary>Sanitizes a filepath to ensure it's correct.</summary>
	/// <param name="filepath">The filepath to sanitize.</param>
	/// <returns>The sanitized filepath.</returns>
	std::string SanitizeFilepath(std::string filepath);
};