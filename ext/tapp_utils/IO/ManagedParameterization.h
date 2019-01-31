// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

#pragma once

#include <map>
#include <string>
#include <vector>

namespace TAPP::IO
{
	/*	ManagedParameterization
		
		A simple class for system call parameter initialization. Thread unsafe.
	*/
	class ManagedParameterization
	{
		public:
			/*** Constructors / Destructors *********************************************/

			ManagedParameterization(void);

			/*** Public Functions *******************************************************/

			void AddParameter(bool required, bool accepts_value, std::string category, std::string description, std::string name);

			std::string GenerateParameterizationList(void);

			std::map<std::string, std::string> ParseInput(int argc, char* argv[]);

		private:
			/*** Structures *************************************************************/

			struct ParameterEntry
			{
				bool accepts_value;
				std::string category;
				std::string description;
			};

			/*** Private Variables ******************************************************/

			std::map<std::string, ParameterEntry>	m_parameter_map_;
			std::vector<std::string>				m_required_parameters_;

			/*** Private Functions ******************************************************/

			void ValidateParameters_(const std::map<std::string, std::string>& parameter_value_map);
	};
}