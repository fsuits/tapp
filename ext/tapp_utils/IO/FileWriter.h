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
	class FileWriter
	{
		public:
			/*** Public Variables *******************************************************/

			std::ofstream input_stream;

			/*** Constructors / Destructors *********************************************/

			FileWriter(void); // Default Constructor
			FileWriter(const std::string filepath, const bool as_binary = false); // Opens a file straight away.
			FileWriter(const FileWriter& other) = delete; // Copy construction isn't allowed.
			~FileWriter(void);

			void		CloseFile(void);
			std::string GetFilepath(void);
			bool		IsFileOpen(void);
			void		OpenFile(const std::string filepath, const bool as_binary = false);

		private:
			/*** Private Variables ******************************************************/

			std::string m_current_file_;
	};
}