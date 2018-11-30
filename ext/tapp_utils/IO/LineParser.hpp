#pragma once

#include <functional>
#include <vector>

#include "IO/StreamParser-deprecated.hpp"
#include "Utilities/StringManipulation.h"

namespace TAPP::IO
{
	/*** Structs ****************************************************************/

	template <typename T> // Template object
	/*	LineParser

		<summary>
		This class parses a file line by line, based upon a functor that can parse a list of lines.	It passes
		the lines towards the parse method through substrings. During the creation of the substrings, the 
		memory usage might be scaled to twice the size of the buffer.
		</summary>
	*/
	class LineParser : public StreamParser<std::vector<T>>
	{
		public:
			/*** Typedefs ***************************************************************/
			
			typedef std::function<void(const std::vector<std::string>&)>			HeaderParseFunction; // std::function<void(const std::vector<std::string>&)>
			typedef std::function<std::vector<T>(const std::vector<std::string>&)>	ParseFunction; // std::function<std::vector<T>(const std::vector<std::string>&)>

			/*** Constructors / Destructors *********************************************/

			/*	LineParser
				<summary>Constructs the LineParser object. Header lines will be ignored if no header parse functor is added.</summary>
				<param name="parse_function">A functor that uses a vector of strings to parse the various lines present in the buffer.</param>
				<param name="buffer_size">The amount of bytes available for the buffer.</param>
				<param name="header_lines">The amount of lines that will be considered as part of the header.</param>
			*/
			LineParser(ParseFunction line_parse_function, size_t buffer_size = 5000000, int header_lines = 0)
				: m_line_parse_function_(line_parse_function), m_header_lines_(header_lines), StreamParser<std::vector<T>>(buffer_size)
			{
			}

			/*	LineParser
				<summary>Constructs the LineParser object.</summary>
				<param name="header_parse_function">A functor that uses a vector of strings to parse the various header lines present in the buffer.</param>
				<param name="parse_function">A functor that uses a vector of strings to parse the various lines present in the buffer.</param>
				<param name="buffer_size">The amount of bytes available for the buffer.</param>
				<param name="header_lines">The amount of lines that will be considered as part of the header.</param>
			*/
			LineParser(HeaderParseFunction header_parse_function, ParseFunction line_parse_function, size_t buffer_size = 5000000, int header_lines = 0)
				: m_header_parse_function_(header_parse_function), m_line_parse_function_(line_parse_function), m_header_lines_(header_lines), StreamParser<std::vector<T>>(buffer_size)
			{
			}

			LineParser(const LineParser& other) = delete; // Copy construction isn't allowed.
			
			/*** Functions **************************************************************/

			/*	SetHeaderParseFunction
				<summary>Replaces the current header parse function.</summary>
				<param name="header_parse_function">A functor that uses a vector of strings to parse the various header lines present in the buffer.</param>
			*/
			void SetHeaderParseFunction(HeaderParseFunction header_parse_function)
			{
				this->m_mutex$.lock();
				m_header_parse_function_ = header_parse_function;
				this->m_mutex$.unlock();
			}

			/*	SetLinesToIgnore
				<summary>Changes the amount of header lines being passed.</summary>
				<param name="header_parse_function">A functor that uses a vector of strings to parse the various header lines present in the buffer.</param>
			*/
			void SetLinesToIgnore(size_t header_lines)
			{
				this->m_mutex$.lock();
				m_header_lines_ = header_lines;
				this->m_mutex$.unlock();
			}

			/*	SetParseFunction
				<summary>Replaces the current parse function.</summary>
				<param name="parse_function">A functor that uses a vector of strings to parse the various lines present in the buffer.</param>
			*/
			void SetParseFunction(ParseFunction parse_function)
			{
				this->m_mutex$.lock();
				m_line_parse_function_ = parse_function;
				this->m_mutex$.unlock();
			}
		
		protected:
			/*** Functions **************************************************************/

			/*	Setup$
				<summary>Preparses the LineParser for parsing.</summary>
				<param name="filepath">A constant reference to a string containing the filepath.</param>
			*/
			void Setup$(const std::string& filepath)
			{
				m_current_header_lines_ = m_header_lines_;
			}

			/*	Closure$
				<summary>The implementation allows the parser to clean up or perform other actions.</summary>
			*/
			void Closure$(void)
			{
			}

			/*	ParseString$
				<summary>Parses the text that's currently present in the buffer.</summary>
				<param name="buffer">A reference to the buffer string.</param>
			*/
			void ParseString$(std::string& buffer)
			{
				// Initializes a vector for the line positions within the buffer.
				std::vector<std::string> substrings;
				substrings.reserve(Utilities::StringManipulation::GetAmountOfLines(buffer));

				// Loops until no more endlines can be found. Fills the buffer_positions vector.
				size_t next_line = 0;
				size_t final_element = 0; // Keeps track of the last character that hasn't been parsed.
				while (next_line != std::string::npos)
				{
					size_t start = next_line;
					next_line = buffer.find('\n', next_line);

					if (next_line == std::string::npos)
					{
						break;
					}
					else if (start != next_line)
					{
						std::string buffer_substring(buffer.substr(start, -1 * (start - next_line)));
						PushLine_(buffer_substring, substrings);
					}

					++next_line;
					final_element = next_line;
				}

				// Adds the final line of a file to the buffer_positions list.
				if (this->m_file_reader$.input_stream.eof())
				{
					std::string final_line;
					size_t last_endline = buffer.find_last_of('\n');

					if (last_endline == std::string::npos)
					{
						final_line = buffer;
					}
					else
					{
						final_line = buffer.substr(last_endline + 1);

						if (final_line.find_first_not_of('\0') == std::string::npos)
						{
							final_line.clear();
						}
					}

					// Removes any trailing null characters.
					size_t null_character = final_line.find('\0');
					if (null_character != std::string::npos)
					{
						final_line = final_line.substr(0, null_character);
					}

					// Pushes the new line.
					if (!final_line.empty())
					{
						PushLine_(final_line, substrings);
					}

					final_element = buffer.size();
				}

				// Clears the buffer characters that haven't been put into the parsing queue.
				if (final_element < buffer.size()) // Current content has characters that still require parsing.
				{
					buffer = buffer.substr(final_element);
				}
				else // All characters have been parsed.
				{
					buffer = "";
				}

				HandleHeader_(substrings); // Acquires header lines, if they still need parsing.

				// If the substring vector still contains parseable elements.
				if (!substrings.empty())
				{
					// Parses the lines and inserts them into the return vector.
					std::vector<T> temp_vector(m_line_parse_function_(substrings));
					this->m_return_object$.reserve(this->m_return_object$.size() + temp_vector.size()); // Ensures unnecessary resizing is avoided.
					this->m_return_object$.insert(this->m_return_object$.end(), temp_vector.begin(), temp_vector.end()); // Merges the two vectors.
				}
			}

		private:
			/*** Variables **************************************************************/

			size_t				m_current_header_lines_; // The amount of lines that still need to be ignored. Decremented by: HandleHeader_.
			size_t				m_header_lines_; // The total amount of lines that are going to be ignored.

			HeaderParseFunction	m_header_parse_function_; // The functor responsible for the line parsing.
			ParseFunction		m_line_parse_function_; // The functor responsible for the line parsing.

			/*** Functions **************************************************************/

			/*	HandleHeader_
				<summary>Handles the parsing of the header lines.</summary>
				<param name="substrings">A vector with std::strings that need parsing.</param>
				<returns>A size_t containing the location of the last element which hasn't been parsed.</returns>
			*/
			void HandleHeader_(std::vector<std::string>& substrings)
			{
				// If header lines still require parsing.
				if (m_current_header_lines_ > 0 && !substrings.empty())
				{
					// Only a part of the vector is required.
					if (substrings.size() - m_current_header_lines_ > 0)
					{
						if (m_header_parse_function_)
						{
							std::vector<std::string> header_lines(substrings.begin(), substrings.begin() + m_current_header_lines_);
							m_header_parse_function_(header_lines);
						}
						
						substrings.erase(substrings.begin(), substrings.begin() + (m_current_header_lines_));
						m_current_header_lines_ = 0;
					}
					// The entire vector is required.
					else
					{
						if (m_header_parse_function_)
						{
							m_header_parse_function_(substrings);
						}

						m_current_header_lines_ -= substrings.size();
						substrings.clear();
					}
				}
			}
			

			/*	PushLine_
				<summary>Checks whether the line is valid to push.</summary>
				<param name="line">Line to check.</param>
				<param name="line_vector">Vector to push the line into.</param>
			*/
			void PushLine_(std::string& line, std::vector<std::string>& line_vector)
			{
				if (!line.empty() && line != "")
				{
					line_vector.push_back(std::move(line));
				}
			}
	};
}



