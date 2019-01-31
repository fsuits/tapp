// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#include "Exceptions/FormatError.h"

namespace TAPP::Exceptions
{
	FormatError::FormatError(const std::string message) : std::runtime_error(message)
	{
	}
}
