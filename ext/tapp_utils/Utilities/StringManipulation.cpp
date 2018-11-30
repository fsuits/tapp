#include "Utilities/StringManipulation.h"

#include <algorithm>
#include <stdexcept>

namespace TAPP::Utilities::StringManipulation
{
	size_t CountSubstrings(const std::string& original_string, const std::string& substring)
	{
		size_t substrings_found = 0;
		size_t substring_position = original_string.find(substring);
		size_t substring_size = substring.size(); // Used for a slight speed increase.

		while (substring_position != std::string::npos)
		{
			++substrings_found;
			substring_position = original_string.find(substring, substring_position + substring_size);
		}

		return substrings_found;
	}

	std::string FilepathToFilename(const std::string& filepath)
	{
		// Linux style filepath.
		size_t slash_position = filepath.find_last_of('/');
		if (slash_position != std::string::npos)
		{
			return filepath.substr(slash_position+1);
		}
		// Windows style filepath.
		slash_position = filepath.find_last_of('\\');
		if (slash_position != std::string::npos)
		{
			return filepath.substr(slash_position+1);
		}
		// No slashes found.
		else
		{
			return filepath;
		}
	}

	/*** GetEmbeddedCharacters **************************************************/

	// Acquires characters embedded between brackets from the string. Bracket type defines the boundaries for the characters.
	std::string GetEmbeddedCharacters(const std::string& string, const BracketType type)
	{
		switch (type)
		{
			case APOSTROPHE:	return GetEmbeddedCharacters(string, '\'', '\'', 0); break;
			case BRACES:		return GetEmbeddedCharacters(string, '{', '}', 0); break;
			case BRACKETS:		return GetEmbeddedCharacters(string, '[', ']', 0); break;
			case CHEVRONS:		return GetEmbeddedCharacters(string, '<', '>', 0); break;
			case PARENTHESES:	return GetEmbeddedCharacters(string, '(', ')', 0); break;
			case QOUTES:		return GetEmbeddedCharacters(string, '"', '"', 0); break;
		}
	}

	// Acquires characters embedded between brackets from the string. Bracket type defines the boundaries for the characters.
	// The start parameter defines the first character to be parsed.
	std::string GetEmbeddedCharacters(const std::string& string, const BracketType type, const size_t start)
	{
		switch (type)
		{
			case APOSTROPHE:	return GetEmbeddedCharacters(string, '\'', '\'', start); break;
			case BRACES:		return GetEmbeddedCharacters(string, '{', '}', start); break;
			case BRACKETS:		return GetEmbeddedCharacters(string, '[', ']', start); break;
			case CHEVRONS:		return GetEmbeddedCharacters(string, '<', '>', start); break;
			case PARENTHESES:	return GetEmbeddedCharacters(string, '(', ')', start); break;
			case QOUTES:		return GetEmbeddedCharacters(string, '"', '"', start); break;
		}
	}

	// Acquires characters embedded between brackets from the string. The passed characters define the boundaries.
	std::string GetEmbeddedCharacters(const std::string& string, const char opening_bracket, const char closing_bracket)
	{
		return GetEmbeddedCharacters(string, opening_bracket, closing_bracket, 0);
	}

	// Acquires characters embedded between brackets from the string. The passed characters define the boundaries.
	// The start parameter defines the first character to be parsed.
	std::string GetEmbeddedCharacters(const std::string& string, const char opening_bracket, const char closing_bracket, const size_t start)
	{
		size_t position = string.find(opening_bracket, start);

		if (position == std::string::npos)
		{
			throw std::runtime_error("Couldn't locate opening bracket.");
		}

		++position;
		size_t length = string.find(closing_bracket, position);

		if (length == std::string::npos)
		{
			throw std::runtime_error("Couldn't locate closing bracket.");
		}

		length -= position;
		return string.substr(position, length);
	}

	/*** String Acquisition *****************************************************/

	// Acquires the amount of lines within a file.
	size_t GetAmountOfLines(const std::string& text)
	{
		return std::count(text.begin(), text.end(), '\n');
	}

	// Returns a string with just the selected range of lines.
	std::string GetLines(const std::string& text, const size_t line_a, const size_t line_b)
	{
		size_t size = text.size();
		size_t pos_a = 0, pos_b = size;

		size_t counter = 0;
		for (size_t c = 0; c < size; ++c)
		{
			if (counter == line_a)
			{
				pos_a = c;
				break;
			}

			else if (text[c] == '\n')
			{
				++counter;
			}
		}

		for (size_t c = pos_a; c < size; ++c)
		{
			if (counter == line_b)
			{
				pos_b = c;
				break;
			}

			else if (text[c] == '\n')
			{
				++counter;
			}
		}

		return text.substr(pos_a, (pos_b - pos_a) - 1);
	}

	// Returns a string with just the selected range of lines. Uses a reference to a size_t to report the amount of read lines.
	std::string GetLines(const std::string& text, const size_t line_a, const size_t line_b, size_t& lines_read)
	{
		std::string temp(GetLines(text, line_a, line_b));
		lines_read = GetAmountOfLines(temp);
		return temp;
	}

	// Splits every value delimited by the passed char.
	std::vector<std::string> Split(const std::string& list, char delimiter)
	{
		// Prepares the parsing variables.
		size_t start_position = 0;
		size_t end_position = list.find(delimiter);
		size_t size = list.size();

		// Iterates through the string, acquiring the various values.
		std::vector<std::string> values;
		values.reserve(std::count(list.begin(), list.end(), delimiter));
		while (end_position < size && end_position != std::string::npos)
		{
			std::string value(list.substr(start_position, end_position - start_position));
			if (!value.empty())
			{
				values.push_back(std::move(value));
			}
			start_position = end_position + 1;
			end_position = list.find(delimiter, start_position);
		}

		// Adds the remaining characters to a value entry.
		std::string value(list.substr(start_position));
		if (!value.empty() && value.find_first_not_of('\0') != std::string::npos)
		{
			values.push_back(std::move(value));
		}

		return values;
	}
}
