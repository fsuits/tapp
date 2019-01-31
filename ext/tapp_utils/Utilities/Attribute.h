// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#pragma once

#include <string>

namespace Utilities
{
	class Attribute
	{
		public:
			Attribute(const std::string value);

			bool		GetAsBool(void);
			char		GetAsChar(void);
			double		GetAsDouble(void);
			float		GetAsFloat(void);
			int			GetAsInt(void);
			long		GetAsLong(void);
			std::string	GetAsString(void);

		private:
			std::string m_attribute_value_;
	};
}