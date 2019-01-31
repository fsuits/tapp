// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#include "IO/FileWriter.h"

#include <stdexcept>

namespace TAPP::IO
{
	/*** Constructors ***********************************************************/

	// Default constructor
	FileWriter::FileWriter(void)
	{
	}

	// Initializer constructor
	// Constructs the reader and then opens the specified file.
	FileWriter::FileWriter(const std::string filepath, const bool as_binary)
	{
		OpenFile(filepath, as_binary);
	}

	// Default destructor
	FileWriter::~FileWriter(void)
	{
		// Makes sure to close the file if it's still open.
		CloseFile();
	}

	/*** Public Functions *******************************************************/

	// Closes the file stream.
	void FileWriter::CloseFile(void)
	{
		if (input_stream.is_open())
		{
			input_stream.close();
			m_current_file_.clear();
		}
	}

	// Returns whether or not a file is currently opened.
	bool FileWriter::IsFileOpen(void)
	{
		return input_stream.is_open();
	}

	// Returns the filepath of the current file.
	std::string FileWriter::GetFilepath(void)
	{
		return m_current_file_;
	}

	// Opens a file stream to the supplied filepath.
	void FileWriter::OpenFile(const std::string filepath, const bool as_binary)
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
}
