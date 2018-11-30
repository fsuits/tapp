#ifndef _STRING_TOKENIZER_
#define _STRING_TOKENIZER_

#include <string>

class StringTokenizer {
	
	// Default separator is tab, but could be something else too, TODO.
	static const int default_separator = '\t';

	std::string fileRecord;
	std::basic_string<char>::size_type begin_val;
	bool done;
	bool skipNullStrings;
	int separator;

public:
	
	StringTokenizer(std::string aFileRecord, int aSeparator, bool skipNull);
	std::string next();
	bool isDone() { 
		return done; 
	}

};

#endif