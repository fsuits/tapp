#pragma once
namespace TAPP::Filetypes::DSV
{
	/// <summary>Holds the values that denote the record and value delimiters and encapsulation required to parse a Data Separated File.</summary>
	struct Grammar
	{
		char record_delimiter;
		char value_delimiter;
		char value_encapsulation;
	};
}