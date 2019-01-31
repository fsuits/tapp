// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#include "Utilities/Sanitization.h"

#include <vector>

namespace TAPP::Utilities
{
	// TODO: Improve filepath sanitization beyond slashes and dots.
	std::string SanitizeFilepath(std::string filepath)
	{
		// Tracks the elements to remove and the previous element.
		std::vector<size_t> characters_to_remove;
		char previous_character = 0;

		// Loops through all the characters in the filepath.
		for (size_t current_character = 0; current_character < filepath.size(); ++current_character)
		{
			switch (filepath[current_character])
			{
				case '\\':	filepath[current_character] = '/';														break;
				case '/':	if (previous_character == '/') { characters_to_remove.push_back(current_character); }	break;
				case '.':	if (previous_character == '.') { characters_to_remove.push_back(current_character); }	break;
			}
			previous_character = filepath[current_character];
		}

		// Removes all the marked elements.
		size_t removed_characters = 0;
		for (size_t index : characters_to_remove)
		{
			filepath.erase(index - removed_characters, 1);
			++removed_characters;
		}

		return filepath;
	}
}
