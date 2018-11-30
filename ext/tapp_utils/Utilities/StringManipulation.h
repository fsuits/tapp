#pragma once

#include <string>
#include <vector>

/*
	This namespace defines several functions that can aid in string manipulation. It offers various
	solutions for common problems.
*/

namespace TAPP::Utilities::StringManipulation
{
	enum BracketType
	{
		APOSTROPHE,
		BRACES,
		BRACKETS,
		CHEVRONS,
		PARENTHESES,
		QOUTES
	};

	size_t CountSubstrings(const std::string& original_string, const std::string& substring);

	std::string FilepathToFilename(const std::string& filepath);

	size_t GetAmountOfLines(const std::string& text);

	std::string GetEmbeddedCharacters(const std::string& string, const BracketType type);
	std::string GetEmbeddedCharacters(const std::string& string, const BracketType type, const size_t start);
	std::string GetEmbeddedCharacters(const std::string& string, const char opening_bracket, const char closing_bracket);
	std::string GetEmbeddedCharacters(const std::string& string, const char opening_bracket, const char closing_bracket, const size_t start);

	std::string GetLines(const std::string& text, const size_t line_a, const size_t line_b);
	std::string GetLines(const std::string& text, const size_t line_a, const size_t line_b, size_t& lines_read);

	std::vector<std::string> Split(const std::string& list, char delimiter);
}