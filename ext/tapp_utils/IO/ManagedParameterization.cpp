#include "IO/ManagedParameterization.h"

#include <cmath>
#include <stdexcept>
	
namespace TAPP::IO
{
	/*** Constructors / Destructors *********************************************/

	ManagedParameterization::ManagedParameterization(void)
	{
	}

	/*** Public Functions *******************************************************/

	void ManagedParameterization::AddParameter(bool required, bool accepts_value, std::string category, std::string description, std::string name)
	{
		auto parameter_iterator = m_parameter_map_.insert({ name, { accepts_value, category, description } });
		
		if (!parameter_iterator.second)
		{
			throw std::runtime_error("Unable to add parameter: " + name);
		}
		else if (required)
		{
			m_required_parameters_.push_back(parameter_iterator.first->first);
		}
	}

	std::string ManagedParameterization::GenerateParameterizationList(void)
	{
		unsigned int largest_parameter = 0;
		for (auto parameter : m_parameter_map_)
		{
			if (parameter.first.size() > largest_parameter)
			{
				largest_parameter = parameter.first.size();
			}
		}

		std::map<std::string, std::string> category_map;

		for (auto parameter : m_parameter_map_)
		{
			auto category_entry = category_map.insert({ parameter.second.category, std::string(parameter.second.category + ":\n") });

			category_entry.first->second += "  -" + parameter.first + "  ";

			// Adds the tabs to split the name and description.
			for (size_t whitespace = parameter.first.size(); whitespace < largest_parameter; ++whitespace)
			{
				category_entry.first->second += " ";
			}

			category_entry.first->second += parameter.second.description + "\n";
		}

		std::string parameter_list("");
		for (auto category : category_map)
		{
			parameter_list += category.second + "\n";
		}

		return parameter_list;
	}

	std::map<std::string, std::string> ManagedParameterization::ParseInput(int argc, char* argv[])
	{
		// Maps the parameter names to their string inputs.
		std::map<std::string, std::string> parameter_value_map;
		for (auto parameter : m_parameter_map_)
		{
			parameter_value_map.insert({ parameter.first, std::string() });
		}

		for (int param = 0; param < argc; ++param)
		{
			// Prepares the parameter name.
			std::string parameter_name(argv[param]);
			size_t param_start = parameter_name.find_first_of('-');
			if (param_start != std::string::npos)
			{
				parameter_name = parameter_name.substr(param_start + 1);

				// Attempts to find the parameter in the map.
				auto parameter = m_parameter_map_.find(parameter_name);
				if (parameter != m_parameter_map_.end())
				{
					// If the parameter requires a value.
					if (parameter->second.accepts_value)
					{
						++param;
						parameter_value_map.find(parameter->first)->second = std::string(argv[param]);
					}
					else
					{
						parameter_value_map.find(parameter->first)->second = "1";
					}
				}
			}
		}

		// Checks if all the required parameters have been entered.
		ValidateParameters_(parameter_value_map);

		return parameter_value_map;
	}

	/*** Private Functions ******************************************************/

	void ManagedParameterization::ValidateParameters_(const std::map<std::string, std::string>& parameter_value_map)
	{
		std::string error_message("");

		// Loops through all the required parameters to see if they have been initialized.
		for (std::string parameter : m_required_parameters_)
		{
			if (parameter_value_map.find(parameter)->second.empty())
			{
				error_message += "The \"" + parameter + "\" parameter is mandatory for this programs execution.\n";
			}
		}

		if (!error_message.empty())
		{
			throw std::runtime_error(error_message);
		}
	}
}
