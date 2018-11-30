#pragma once
#include <fstream>
#include <functional>
#include <stdexcept>
#include <vector>

#include "IO/FileFormat/DSV/Grammar.h"
#include "IO/StreamReading.h"
#include "Exceptions/FormatError.h"
#include "Exceptions/FileAccessError.h"

namespace TAPP::Filetypes::DSV
{
	/// <summary>Marks the starting and ending iterators for characters in a CircularBuffer.</summary>
	struct ValuePosition
	{
		Collections::CircularBuffer<char>::iterator begin;
		Collections::CircularBuffer<char>::iterator end;
	};

	// TODO: Base parsing on memory mapped file.

	template <typename T>
	/// <summary>A parser that can translate streamed characters from a DSV format.</summary>
	class Parser
	{
		public:
			/// <summary>A function that can use a header and value positions to parse characters into the desired format.</summary>
			typedef std::function<std::vector<T>(const std::vector<std::string>&, const std::vector<std::vector<ValuePosition>>&)> ParseFunction;

		private:
			size_t						m_buffer_size_;
			Grammar						m_grammar_;
			bool						m_contains_header_;
			ParseFunction				m_parse_function_;

		public:
			/// <summary>Constructs the parser.</summary>
			/// <param name="grammar">The grammar for the DSV format to parse.</param>
			/// <param name="contains_header">Whether or not the stream contains a header row.</param>
			/// <param name="buffer_size">The maximum amount of characters to parse at any given time.</param>
			/// <param name="parse_function">The functor that is used to parse the discovered values into variables.</param>
			Parser(const Grammar grammar, const bool contains_header, const size_t buffer_size, ParseFunction parse_function)
				: m_buffer_size_(buffer_size), m_grammar_(grammar), m_contains_header_(contains_header), m_parse_function_(parse_function)
			{
			}

			std::vector<T> Parse(const std::string& filepath)
			{
				// Opens the file stream and parses it.
				std::ifstream file_stream(filepath);
				try
				{
					std::vector<T> records(Parse(file_stream));
					file_stream.close();
					return records;
				}
				catch (const Exceptions::FileAccessError& exception)
				{
					throw Exceptions::FileAccessError("Unable to read from file: " + filepath);
				}
				catch (const Exceptions::FormatError& exception)
				{
					throw Exceptions::FormatError("File contains grammar format errors: " + filepath);
				}

				// Never reached, used to prevent potential warnings.
				return std::vector<T>();
			}

			/// <summary>Parses the DSV records located within the stream.</summary>
			/// <param name="stream">The stream to parse from.</param>
			/// <returns>A vector with all the parsed records.</returns>
			std::vector<T> Parse(std::istream& stream)
			{
				// Throws an error if the stream is faulty.
				if (stream.bad())
				{
					throw Exceptions::FileAccessError("Unable to read from stream.");
				}

				// Initializes the buffer.
				Collections::CircularBuffer<char> buffer(m_buffer_size_);

				// Used to track various states during parsing.
				size_t	current_character		= 0;
				bool	inside_encapsulation	= false;
				int64_t value_start				= -1;
				int64_t	value_end				= -1;

				std::vector<std::string>	header;
				std::vector<ValuePosition>	current_record;

				// Holds the parsed records.
				std::vector<T> parsed_records;

				// Iterates over the stream until the end has been reached.
				bool end_of_stream = false;
				do
				{
					// Reads stream characters into the buffer.
					end_of_stream = IO::ReadIntoCircularBuffer(stream, buffer);

					// Tracks the amount of characters to remove from the buffer.
					size_t consumed_characters = 0;

					// Loops through each character, looking for the various delimiters.
					std::vector<std::vector<ValuePosition>> records_to_parse;
					for (; current_character < buffer.Size(); ++current_character)
					{
						if (buffer[current_character] == m_grammar_.value_encapsulation)
						{
							SwitchEncapsulation_(inside_encapsulation, value_end, current_character);
						}
						else if (!inside_encapsulation && buffer[current_character] == m_grammar_.value_delimiter)
						{
							StoreValue_(buffer, current_record, current_character, value_start, value_end);
						}
						else if (!inside_encapsulation && buffer[current_character] == m_grammar_.record_delimiter)
						{
							StoreValue_(buffer, current_record, current_character, value_start, value_end);
							StoreRecord_(header, current_record, records_to_parse);
							consumed_characters = current_character;
						}
						else if (value_start == -1)
						{
							value_start = current_character;
						}
					}

					// Takes care of the last record which wasn't record delimited.
					if (end_of_stream && !current_record.empty())
					{
						StoreValue_(buffer, current_record, current_character, value_start, value_end);
						StoreRecord_(header, current_record, records_to_parse);
					}

					// Parses the collected records and inserts them into the return vector.
					std::vector<T> temporary_records(m_parse_function_(header, records_to_parse));
					parsed_records.insert(parsed_records.end(), temporary_records.begin(), temporary_records.end());

					// Consumes the amount of characters currently parsed. Updates the current character to reflect the last character read.
					buffer.Consume(consumed_characters);
					current_character -= consumed_characters;

					// Updates the value start and end if they have a value.
					if (value_start > -1)
					{
						value_start	-= consumed_characters;
					}

					if (value_end > -1)
					{
						value_end	-= consumed_characters;
					}
				} while (!end_of_stream);

				return parsed_records;
			}

		private:
			/// <summary>Converts the header Value Positions to strings.</summary>
			/// <param name="header_values">The Value Positions of the header column identifiers.</param>
			/// <returns>A vector containing all the header column identifiers as stirngs.</returns>
			std::vector<std::string> ParseHeader_(const std::vector<ValuePosition>& header_values)
			{
				// Parses the header values into strings.
				std::vector<std::string> header;
				header.reserve(header_values.size());
				for (const ValuePosition& position : header_values)
				{
					header.push_back(std::string(position.begin, position.end));
				}

				return header;
			}

			/// <summary>Stores the current value that is being pointed at by the value start and end.</summary>
			/// <param name="buffer">A reference to the character buffer.</param>
			/// <param name="current_character">The current character being processed.</param>
			/// <param name="value_start">A reference to the starting index of the value.</param>
			/// <param name="value_end">A reference to the ending index of the value, can be -1.</param>
			void StoreValue_(Collections::CircularBuffer<char>& buffer, std::vector<ValuePosition>& current_record, size_t current_character, int64_t& value_start, int64_t& value_end)
			{
				// If the value end hasn't been set, use the current_character as value end.
				if (value_end == -1)
				{
					value_end = current_character;
				}

				// Calculates the length of the value.
				size_t value_length = 1;
				if (value_start != value_end)
				{
					value_length = value_end - value_start;
				}

				// If the value contains any characters.
				if (value_start > -1 && value_length > 0)
				{
					current_record.push_back(ValuePosition{ buffer.begin() + (int)value_start,  buffer.begin() + (int)(value_start + value_length) });
				}
				else
				{
					current_record.push_back(ValuePosition{ buffer.end(),  buffer.end() });
				}

				// Resets the values.
				value_start = -1;
				value_end = -1;
			}

			/// <summary>Stores a record, based on the currently collected values. Parses the header if its the first call for a file.</summary>
			/// <param name="header">A reference to the header vector. </param>
			/// <param name="current_record">A reference to the current record vector.</param>
			/// <param name="records_to_parse">A reference to the vector holding all the unparsed records.</param>
			void StoreRecord_(std::vector<std::string>& header, std::vector<ValuePosition>& current_record, std::vector<std::vector<ValuePosition>>& records_to_parse)
			{
				// If a header needs to be parsed.
				if (m_contains_header_ && header.empty())
				{
					header = ParseHeader_(current_record);
				}
				// Pushes the current record into the list of records.
				else
				{
					records_to_parse.push_back(current_record);
				}

				// Clears the current record.
				current_record.clear();
			}

			/// <summary>Switches the encapsulation and sets the value end position if a closing tag is encountered.</summary>
			/// <param name="inside_encapsulation">A reference to the current encapsulation state.</param>
			/// <param name="value_end">A reference to the current value end position.</param>
			/// <param name="current_character">The current character being processed.</param>
			void SwitchEncapsulation_(bool& inside_encapsulation, int64_t& value_end, size_t current_character)
			{
				// If the encapsulation is being entered.
				if (inside_encapsulation)
				{
					// If  the encapsulation is encountered after the value has already been discovered.
					if (value_end > -1)
					{
						ThrowFormatException();
					}

					value_end = current_character;
				}

				inside_encapsulation != inside_encapsulation;
			}

			/// <summary>Throws an hardcoded runtime_error.</summary>
			void ThrowFormatException(void)
			{
				throw Exceptions::FormatError("File grammar format contains errors.");
			}
	};
}
