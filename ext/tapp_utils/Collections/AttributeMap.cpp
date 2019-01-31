// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#include <algorithm>

#include "Collections/AttributeMap.h"

namespace TAPP::Collections
{
	AttributeMap::AttributeMap(void)
	{
	}

	void AttributeMap::AddAttribute(const std::string name, const std::string value)
	{
		attribute_map_.insert({ name, value });
	}

	void AttributeMap::RemoveAttribute(const std::string name)
	{
		attribute_map_.erase(name);
	}

	std::string AttributeMap::GetAttribute(const std::string name)
	{
		return GetAttributeValue_(name);
	}

	bool AttributeMap::GetAttributeAsBool(const std::string name)
	{
		std::string value = GetAttributeValue_(name);
		if (value.size() > 1)
		{
			std::transform(value.begin(), value.end(), value.begin(), ::tolower);
			if (value == "true")
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return value[0] == '1';
		}
	}

	char AttributeMap::GetAttributeAsChar(const std::string name)
	{
		return GetAttributeValue_(name)[0];
	}

	double AttributeMap::GetAttributeAsDouble(const std::string name)
	{
		return std::stod(GetAttributeValue_(name));
	}

	float AttributeMap::GetAttributeAsFloat(const std::string name)
	{
		return std::stof(GetAttributeValue_(name));
	}

	int	AttributeMap::GetAttributeAsInt(const std::string name)
	{
		return std::stoi(GetAttributeValue_(name));
	}

	long AttributeMap::GetAttributeAsLong(const std::string name)
	{
		return std::stol(GetAttributeValue_(name));
	}

	std::string AttributeMap::GetAttributeValue_(const std::string name)
	{
		return attribute_map_.find(name)->second;
	}
}
