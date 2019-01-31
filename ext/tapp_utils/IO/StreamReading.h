// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

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
