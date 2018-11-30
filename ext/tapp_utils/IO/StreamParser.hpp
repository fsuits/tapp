#pragma once

#include <istream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>

#include "Collections/CircularBuffer.hpp"

namespace TAPP::IO
{
	template <typename T>
	class StreamParser
	{
		private:
			size_t		m_buffer_size_;

		public:
			StreamParser(const size_t buffer_size) : m_buffer_size_(buffer_size)
			{
			}

			StreamParser(const StreamParser<T>& other) : m_buffer_size_(other.m_buffer_size_)
			{
			}

			StreamParser(StreamParser<T>&& other) noexcept : m_buffer_size_(other.m_buffer_size_)
			{
				other.m_buffer_size_ = 0;
			}

			/*	~StreamParser
				<summary>Deconstructs the StreamParser.</summary>
			*/
			~StreamParser()
			{
				m_mutex$.lock();
				m_mutex$.unlock();
			}

			/** Copy assignment operator */
			StreamParser<T>& operator= (const StreamParser<T>& other)
			{
				m_buffer_size_			= other.m_buffer_size_;
				return *this;
			}

			/** Move assignment operator */
			StreamParser<T>& operator= (StreamParser<T>&& other) noexcept
			{
				m_buffer_size_			= other.m_buffer_size_;
				other.m_buffer_size_	= 0;
				return *this;
			}

		/*** Functions **************************************************************/

			T Parse(std::istream& stream)
			{
				if (!stream.good())
				{
					throw std::runtime_error("Unable to parse from stream.");
				}

				return ParseStream_(stream);
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
			virtual void ParseBuffer$(Collections::CircularBuffer<char>& buffer, const bool reached_end_of_file) = 0;

			/*	Setup$
				<summary>The implementation of this function should prepare the parser for the next run.</summary>
				<param name="filepath">A constant reference to a string containing the filepath.</param>
			*/
			virtual void Setup$(void) = 0;

		private:
			T ParseStream_(std::istream& stream)
			{
				m_mutex$.lock();

				// Calls the setup function, which allows derivatives to prepare for parsing.
				Setup$();

				// Initializes the buffer. An exception will be thrown if this causes memory allocation issues.
				Collections::CircularBuffer<char> buffer(m_buffer_size_);

				bool reached_end_of_file = false;
				while (!reached_end_of_file)
				{
					size_t characters_to_read	= buffer.MaximumSize() - buffer.Size();
					size_t initial_read			= buffer.UnclaimedElementsAtBack();

					if (characters_to_read <= initial_read)
					{
						stream.read(&buffer[buffer.Size()], characters_to_read);
						buffer.IncreaseCount(stream.gcount());
					}
					else
					{
						stream.read(&buffer[buffer.Size()], initial_read);
						buffer.IncreaseCount(stream.gcount());
						stream.read(&buffer[buffer.Size()], characters_to_read - initial_read);
						buffer.IncreaseCount(stream.gcount());
					}

					reached_end_of_file = stream.eof();

					ParseBuffer$(buffer, reached_end_of_file);
				}

				Closure$();
				m_mutex$.unlock();

				// Moves the return object, so that it can be cleaned.
				T temporary_return_object(std::move(m_return_object$));
				m_return_object$ = T();

				return temporary_return_object;
			}
	};
}
