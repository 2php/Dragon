#include <map>
#include <iostream>
#include "utils/flags.hpp"

std::map<std::string, NEW_FUNC> d_global_flag_parsers;
std::map<std::string, std::string> d_global_flag_helpers;

std::map<std::string, NEW_FUNC>& FlagsFactory::getFlagParsers(){
	static ParsersFactory* d_global_flag_parsers = new ParsersFactory();
	return *d_global_flag_parsers;
}
std::map<std::string, std::string>& FlagsFactory::getFlagHelpers(){
	static HelpersFactory* d_global_flag_helpers = new HelpersFactory();
	return *d_global_flag_helpers;
}

void ParseCommandLineFlags(int* pargc, char*** pargv){
	if (*pargc <= 1) return;
	char** argv = *pargv;
	int write_head = 1;
	for (int i = 1; i < *pargc; i++) {
		std::string arg(argv[i]);
		if (arg == "--help") {
			std::cout << "Arguments: " << std::endl;
			for (const auto& help_msg : FlagsFactory::getFlagHelpers()) {
				std::cout << "    " << help_msg.first << ": " << help_msg.second
					<< std::endl;
			}
			exit(0);
		}
		//	if the arg does not start with "--", we will ignore it.
		if (arg[0] != '-' || arg[1] != '-') {
			argv[write_head++] = argv[i];
			continue;
		}
		std::string key, val;
		int prefix_idx = arg.find('=');
		//	we will basically use the value after the "=".
		key = arg.substr(2, prefix_idx - 2);
		val = arg.substr(prefix_idx + 1, std::string::npos);
		FlagsFactory::getFlagParsers()[key](val);
	}
	*pargc = write_head;
}

template <> void DragonFlagParser::parse<std::string>(const std::string& content, std::string* value) {
	*value = content;
}

template <> void DragonFlagParser::parse<int>(const std::string& content, int* value) {
	*value = std::atoi(content.c_str());
}

template <> void DragonFlagParser::parse<double>(const std::string& content, double* value) {
	*value = std::atof(content.c_str());
}

template <> void DragonFlagParser::parse<bool>(const std::string& content, bool* value) {
	if (content == "false" || content == "False" || content == "FALSE" || content == "0") *value = false;
	else if (content == "true" || content == "True" || content == "TRUE" || content == "1") *value = true;
	else *value = false;
}
