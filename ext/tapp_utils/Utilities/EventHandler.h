#pragma once
#include <functional>
#include <string>
#include <unordered_map>

namespace Utilities
{
	typedef std::function<void(void)> SignalMethod;

	class EventHandler
	{
		public:
			EventHandler(void);

			size_t Add(SignalMethod method);

			void Remove(size_t id);
			void Signal(void);

			size_t operator+(SignalMethod method);
			void operator-(size_t id);

		private:
			std::unordered_map<size_t, SignalMethod> m_signal_map_;

			size_t GetId_(void);
	};
}