// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#include "IO/FileReader.h"

#include <stdexcept>

namespace TAPP::IO
{
	/*** Constructors ***********************************************************/

	// Default constructor
	FileReader::FileReader(void)
	{
	}

	// Initializer constructor
	// Constructs the reader and then opens the specified file.
	FileReader::FileReader(const std::string filepath, const bool as_binary)
	{
		OpenFile(filepath, as_binary);
	}

	// Default destructor
	FileReader::~FileReader(void)
	{
		// Makes sure to close the file if it's still open.
		CloseFile();
	}

	/*** Public Functions *******************************************************/

	// Closes the file stream.
	void FileReader::CloseFile(void)
	{
		if (input_stream.is_open())
		{
			input_stream.close();
			m_current_file_.clear();
		}
	}

	// Returns whether or not a file is currently opened.
	bool FileReader::IsFileOpen(void)
	{
		return input_stream.is_open();
	}

	// Returns the filepath of the current file.
	std::string FileReader::GetFilepath(void)
	{
		return m_current_file_;
	}

	// Opens a file stream to the supplied filepath.
	void FileReader::OpenFile(const std::string filepath, const bool as_binary)
	{
		// Checks if a file is currently opened and closes it if true.
		if (input_stream.is_open())
		{
			CloseFile();
		}

		// If the file has to be interpreted as binary.
		if (as_binary)
		{
			input_stream.open(filepath, std::ios::in | std::ios::binary);
		}
		// If the file has to be interpreted as text.
		else
		{
			input_stream.open(filepath, std::ios::in);
		}

		// If the file hasn't been opened, throw an error.
		if (!input_stream.is_open())
		{
			throw std::runtime_error("FileReader is unable to open the file: " + filepath + ".");
		}

		m_current_file_ = filepath;
	}

	// Returns the amount of characters specified as a string. Stops reading if the end of file has been reached.
	std::string FileReader::ReadCharacters(size_t count)
	{
		if (input_stream.eof())
		{
			throw std::runtime_error("FileReader has reached the end of the file: " + m_current_file_ + ".");
		}
		else
		{
			// Reads the characters and returns them as a string.
			std::string characters;
			characters.resize(count);
			input_stream.read(&characters[0], count);
			characters.reserve(characters.size());
			return characters;
		}
	}

	// Returns the next line from the stream as a string.
	std::string FileReader::ReadLine(void)
	{
		if (input_stream.is_open() && !input_stream.eof())
		{
			std::string lineBuffer;
			if (getline(input_stream, lineBuffer))
			{
				return lineBuffer;
			}
		}

		// If an error occurs, decide which statement to throw.
		else if (input_stream.eof())
		{
			throw std::runtime_error("FileReader has reached the end of file: " + m_current_file_ + ".");
		}
		else
		{
			throw std::runtime_error("FileReader has not opened a file.");
		}
	}
}
