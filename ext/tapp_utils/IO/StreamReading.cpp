#include "IO/StreamReading.h"

namespace TAPP::IO
{
	bool ReadIntoCircularBuffer(std::istream& stream, Collections::CircularBuffer<char>& circular_buffer)
	{
		// If the stream has reached its end.
		if (stream.eof())
		{
			return true;
		}

		// Calculates how many characters are going to be read, and whether this can be done in a single call.
		size_t characters_to_read	= circular_buffer.MaximumSize() - circular_buffer.Size();
		size_t initial_read			= circular_buffer.UnclaimedElementsAtBack();

		// If the read can accomplished with a single call.
		if (characters_to_read <= initial_read)
		{
			stream.read(&circular_buffer[circular_buffer.Size()], characters_to_read);
			circular_buffer.IncreaseCount(stream.gcount());
		}
		// If the read requires two calls, to avoid issues with the CircularBuffers boundary.
		else
		{
			stream.read(&circular_buffer[circular_buffer.Size()], initial_read);
			circular_buffer.IncreaseCount(stream.gcount());
			stream.read(&circular_buffer[circular_buffer.Size()], characters_to_read - initial_read);
			circular_buffer.IncreaseCount(stream.gcount());
		}

		return stream.eof();
	}
}
