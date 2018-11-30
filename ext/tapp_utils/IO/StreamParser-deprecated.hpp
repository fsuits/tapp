#pragma once

#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>

#include "IO/FileReader.h"

namespace TAPP::IO
{
	template <typename T> // Template object
	/*	LineParser

		<summary>
		This abstract class provides basic functionality for streaming a file. It can be used as a base
		to implement various types of parsers.
		</summary>
	*/
	class StreamParser
	{
		public:
			/*** Constructors / Destructors *********************************************/

			/*	StreamParser
				<summary>Constructs the StreamParser base.</summary>
				<param name="buffer_size">A size_t representing the buffer size in bytes.</param>
			*/
			StreamParser(size_t buffer_size) : m_buffer_size_(buffer_size)
			{
			}

			StreamParser(const StreamParser& other) = delete; // Copy construction isn't allowed.

			/*	~StreamParser
				<summary>Deconstructs the StreamParser.</summary>
			*/
			~StreamParser()
			{
				m_mutex$.lock();
				m_mutex$.unlock();
			}

			/*** Functions **************************************************************/

			/*	ParseFile
				<summary>Reads segments of the file into the buffer and passes this to the parse functionality.</summary>
				<param name="filepath">A reference to a string containing the filepath.</param>
				<returns>The template defined object.</returns>
			*/
			T ParseFile(const std::string filepath, const bool as_binary = false)
			{
				m_mutex$.lock();

				// Calls the setup function, which allows derivatives to prepare for parsing.
				Setup$(filepath);

				// Checks if there is a sufficient amount of available memory for the buffer.
				std::string buffer("");
				buffer.reserve(m_buffer_size_); // Throws an error if the buffer cannot be allocated.

				// Opens the file and throws an error if the file isn't opened properly.
				try
				{
					m_file_reader$.OpenFile(filepath, as_binary); // Will throw an exception if the file cannot be opened.

					// Streams the file until the end has been reached.
					size_t characters_to_read = m_buffer_size_;
					while (!m_file_reader$.input_stream.eof() || !buffer.empty())
					{
						if (!m_file_reader$.input_stream.eof())
						{
							buffer += m_file_reader$.ReadCharacters(characters_to_read);
						}
						
						if (!buffer.empty())
						{
							ParseString$(buffer);
						}
						characters_to_read = m_buffer_size_ - buffer.size();
					}
				}
				catch (const std::runtime_error& e)
				{
					// Unlocks the mutex to prevent a permanent lockout and then rethrows the exception.
					m_mutex$.unlock();
					throw e;
				}

				Closure$();
				m_file_reader$.CloseFile();
				m_mutex$.unlock();

				// Moves the return object, so that it can be cleaned.
				T temporary_return_object(std::move(m_return_object$));
				m_return_object$ = T();

				return temporary_return_object;
			}

			/*	SetBufferSize
				<summary>Sets the size of the buffer used during parsing.</summary>
				<param name="buffer_size">A size_t representing the buffer size in bytes.</param>
			*/
			void SetBufferSize(size_t buffer_size)
			{
				m_mutex$.lock();
				m_buffer_size_ = buffer_size;
				m_mutex$.unlock();
			}
		
		protected:
			/*** Variables **************************************************************/

			FileReader	m_file_reader$;
			std::mutex	m_mutex$;
			T			m_return_object$; // This will require the templated object to have a default constructor.

			/*** Functions **************************************************************/

			/*	Closure$
			<summary>The implementation allows the parser to clean up or perform other actions..</summary>
			*/
			virtual void Closure$(void) = 0;
			/*	ParseString$
				<summary>
				The implementation of this function should handle the actual parsing of the buffer. The buffer
				must be partially or fully cleaned by this implementation. The StreamParser will contencate
				an additional chunk that corresponds to that of the characters being available in the buffer.
				
				This depends on the buffer size.
				<param name="buffer">A reference to the buffer string.</param>
			*/
			virtual void ParseString$(std::string& buffer) = 0;
			/*	Setup$
				<summary>The implementation of this function should prepare the parser for the next run.</summary>
				<param name="filepath">A constant reference to a string containing the filepath.</param>
			*/
			virtual void Setup$(const std::string& filepath) = 0;
		private:
			/*** Variables **************************************************************/

			size_t		m_buffer_size_;
	};
}
