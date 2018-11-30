#pragma once

#include <string>
#include <vector>

namespace TAPP::MassSpectrometry::Isotope
{
	inline std::vector<std::pair<char, double>> CreateAveragineTable(void)
	{
		std::vector<std::pair<char, double>> averagine(5);
		averagine[0] = { 'C', 4.9384 / 111.1254 };
		averagine[1] = { 'H', 7.7583 / 111.1254 };
		averagine[2] = { 'N', 1.3577 / 111.1254 };
		averagine[3] = { 'O', 1.4773 / 111.1254 };
		averagine[4] = { 'S', 0.0417 / 111.1254 };
		return averagine;
	}

	inline std::string Averagine(std::vector<std::pair<char, double>>& averagine_table, double mass)
	{
		std::string result("");
		for (std::pair<char, double>& atom : averagine_table)
		{
			result += atom.first + std::to_string((int)std::round(atom.second * mass));
		}
		return result;
	}
}
