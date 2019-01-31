// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#pragma once

#include <fstream>
#include <mutex>
#include <string>
#include <vector>

namespace TAPP::IO
{
	/*
		BufferedWriter

		This class is a simple wrapper around an ofstream object. It allows
		the user to write towards a file in a buffered manner, optimizing the
		writing for IO usage.

		This class is thread safe.
	*/
	class BufferedWriter
	{
		public:
			/*** Constructors / Destructors *********************************************/

			BufferedWriter(size_t buffer_size = 5000000);
			BufferedWriter(const BufferedWriter& other) = delete;
			~BufferedWriter(void);

			void		CloseFile(void);
			bool		IsFileOpen(void);
			void		OpenFile(const std::string& filepath);

			void Write(const std::string& text);
			BufferedWriter& operator << (const std::string& text);

		private:
			/*** Private Variables *******************************************************/

			std::string		m_buffer_;
			size_t			m_maximum_buffer_size_;
			std::mutex		m_mutex_;
			std::ofstream	m_writer_;

			void AddToBuffer_(const std::string& text);
			void WriteBuffer_(void);
	};
}