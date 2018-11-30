
//#include "stdafx.h"

#include <string>
using namespace std;

#include "StringTokenizer.h"


StringTokenizer::StringTokenizer(string aFileRecord, int aSeparator, bool skipNull) {
	fileRecord = aFileRecord;
	begin_val = 0; done = false;
	separator = aSeparator;
	skipNullStrings = skipNull;
	// Check for null string.
	if (skipNull&&fileRecord.length()==0)
		done = true;
}
/*	
StringTokenizer::StringTokenizer(string aFileRecord) {
	this(aFileRecord, default_separator, false);
}
*/	
string StringTokenizer::next() {
	string stringFieldValue;

	static const basic_string <char>::size_type npos = -1;

	basic_string<char>::size_type end_val;
	end_val = fileRecord.find_first_of(separator, begin_val);
	if (end_val != npos) {
		if (skipNullStrings) {
			while(end_val == begin_val) {
				begin_val++;
				end_val = fileRecord.find_first_of(separator, begin_val);
			}
		}
		stringFieldValue = fileRecord.substr(begin_val, end_val-begin_val);
		begin_val = end_val+1;
	} else {
		stringFieldValue = fileRecord.substr(begin_val);
		done = true;
	}
	return stringFieldValue;
}
