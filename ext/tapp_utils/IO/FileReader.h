#pragma once

#include <fstream>
#include <string>
#include <vector>

namespace TAPP::IO
{
	/*
		FileReader

		This class is a simple wrapper for an ifstream and provides
		some simple functions. The class is not thread safe.
	*/
	class FileReader
	{
		public:
			/*** Public Variables *******************************************************/

			std::ifstream input_stream;

			/*** Constructors / Destructors *********************************************/

			FileReader(void); // Default Constructor
			FileReader(const std::string filepath, const bool as_binary = false); // Opens a file straight away.
			FileReader(const FileReader& other) = delete; // Copy construction isn't allowed.
			~FileReader(void);

			void		CloseFile(void);
			std::string GetFilepath(void);
			bool		IsFileOpen(void);
			void		OpenFile(const std::string filepath, const bool as_binary = false);

			void Read(char* buffer, const size_t count);
			std::string	ReadCharacters(const size_t count);
			std::string ReadLine(void);

		private:
			/*** Private Variables ******************************************************/

			std::string m_current_file_;
	};
}