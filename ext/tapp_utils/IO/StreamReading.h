#pragma once
#include <istream>

#include "Collections/CircularBuffer.hpp"

namespace TAPP::IO
{
	/// <summary>Reads characters from a stream into a CircularBuffer without overriding existing elements.</summary>
	/// <param name="stream">The input stream to read from.</param>
	/// <param name="circular_buffer">The CircularBuffer to read into.</param>
	/// <returns>Whether or not the stream has reached its end.</returns>
	bool ReadIntoCircularBuffer(std::istream& stream, Collections::CircularBuffer<char>& circular_buffer);
}
