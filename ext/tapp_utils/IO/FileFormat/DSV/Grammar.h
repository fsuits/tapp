// Copyright 2019, IBM Corporation
// 
// This source code is licensed under the Apache License, Version 2.0 found in
// the LICENSE.md file in the root directory of this source tree.

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