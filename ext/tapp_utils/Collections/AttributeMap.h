#pragma once
#include <string>
#include <unordered_map>

namespace TAPP::Collections
{
	class AttributeMap
	{
		public:
			AttributeMap(void);

			void		AddAttribute(const std::string name, const std::string value);
			void		RemoveAttribute(const std::string name);

			std::string GetAttribute(const std::string name);
			bool		GetAttributeAsBool(const std::string name);
			char		GetAttributeAsChar(const std::string name);
			double		GetAttributeAsDouble(const std::string name);
			float		GetAttributeAsFloat(const std::string name);
			int			GetAttributeAsInt(const std::string name);
			long		GetAttributeAsLong(const std::string name);

		private:
			std::unordered_map<std::string, std::string> attribute_map_;

			std::string GetAttributeValue_(const std::string name);
	};
}