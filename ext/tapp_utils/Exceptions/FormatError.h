// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#pragma once
#include <stdexcept>
#include <string>

namespace TAPP::Exceptions
{
	/// <summary>An exception representing a format error discovered during any parsing attempt.</summary>
	class FormatError : public std::runtime_error
	{
		public:
			/// <summary>Constructs a FormatError exception.</summary>
			/// <param name="message">The message the exception should carry.</param>
			FormatError(const std::string message);
	};
}