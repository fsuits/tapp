// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#pragma once
#include <string>

namespace TAPP::Utilities
{
	/// <summary>Sanitizes a filepath to ensure it's correct.</summary>
	/// <param name="filepath">The filepath to sanitize.</param>
	/// <returns>The sanitized filepath.</returns>
	std::string SanitizeFilepath(std::string filepath);
};