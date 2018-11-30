#include "Utilities/Attribute.h"

#include "IO/LineParser.hpp"

namespace Utilities
{
	Attribute::Attribute(const std::string value) : m_attribute_value_(value)
	{
	}

	bool Attribute::GetAsBool(void)
	{
		return !(bool)m_attribute_value_.compare("1");
	}

	char Attribute::GetAsChar(void)
	{
		
		return char();
	}

	double		Attribute::GetAsDouble(void)
	{
		return double();
	}

	float		Attribute::GetAsFloat(void)
	{
		return float();
	}

	int			Attribute::GetAsInt(void)
	{
		return int();
	}

	long		Attribute::GetAsLong(void)
	{
		return long();
	}

	std::string	Attribute::GetAsString(void)
	{
		return m_attribute_value_;
	}
}
