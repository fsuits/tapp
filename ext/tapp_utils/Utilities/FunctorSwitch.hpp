#pragma once

#include <functional>
#include <unordered_map>

namespace TAPP::Utilities
{
	template <typename T>
	/*	FunctorSwitch

		This class resembles a state pattern where the context isn't encapsulated with the
		functionality. It can be used to generically call functions based on an identifier.

		These functions are stored as a std::function object.
	*/
	class FunctorSwitch
	{
		public:
			FunctorSwitch(void)
			{
			}

			void AddFunctor(T identifier, std::function<void(void)> functor)
			{
				m_functor_map_.insert({ identifier, functor });
			}

			void CallFunctor(T identifier)
			{
				auto functor = m_functor_map_.find(identifier);
				
				if (functor != m_functor_map_.end())
				{
					functor->second();
				}
			}

		private:
			std::unordered_map<T, std::function<void(void)>>	m_functor_map_;
	};
}