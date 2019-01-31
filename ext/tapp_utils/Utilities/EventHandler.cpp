// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#include "Utilities/EventHandler.h"

namespace Utilities
{
	EventHandler::EventHandler(void)
	{
	}

	size_t EventHandler::Add(SignalMethod method)
	{
		return this->operator+(method);
	}

	void EventHandler::Remove(size_t id)
	{
		this->operator-(id);
	}

	void EventHandler::Signal(void)
	{
		// Loops through all entries, calling their methods.
		for (std::pair<size_t, SignalMethod> method : m_signal_map_)
		{
			method.second();
		}
	}

	size_t EventHandler::operator+(SignalMethod method)
	{
		size_t new_id = GetId_();
		m_signal_map_.insert({ new_id, method });
		return new_id;
	}

	void EventHandler::operator-(size_t id)
	{
		m_signal_map_.erase(id);
	}

	size_t EventHandler::GetId_(void)
	{
		if (m_signal_map_.empty())
		{
			return 0;
		}

		return m_signal_map_.end()->first + 1;
	}
}
