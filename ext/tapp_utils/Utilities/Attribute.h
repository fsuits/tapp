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