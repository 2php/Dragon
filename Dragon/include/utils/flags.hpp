# ifndef FLAGS_HPP
# define FLAGS_HPP

#include <string>
#include <map>


class DragonFlagParser {
public:
	DragonFlagParser() {}
	template <typename T>
	void parse(const std::string& content, T* value);
};

typedef DragonFlagParser* (*NEW_FUNC)(const std::string&);
typedef std::map<std::string, NEW_FUNC> ParsersFactory;
typedef std::map<std::string, std::string> HelpersFactory;

class FlagsFactory{
public:
	static std::map<std::string, NEW_FUNC>& getFlagParsers();
	static std::map<std::string, std::string>& getFlagHelpers();
};



void ParseCommandLineFlags(int* pargc, char*** pargv);

class FlagsRegister{
public:
	FlagsRegister(std::string name, std::string help_str, NEW_FUNC new_pointer){
		FlagsFactory::getFlagParsers()[name] = new_pointer;
		FlagsFactory::getFlagHelpers()[name] = help_str;
	}
};

#define DEFINE_typed_var(type, name, default_value, help_str)	\
	type FLAGS_##name = default_value;	\
	class DragonFlagParser_##name : public DragonFlagParser {	\
	public:	\
		DragonFlagParser_##name(const std::string& content){ \
		 DragonFlagParser::parse<type>(content, &FLAGS_##name); \
	}};	\
	DragonFlagParser* FlagCreator_##name(const std::string& content){ \
		return new DragonFlagParser_##name(content); \
	} \
	static FlagsRegister g_flag_register_##name(#name,#help_str,FlagCreator_##name);

#define DEFINE_int32(name, default_value, help_str)  DEFINE_typed_var(int, name, default_value, help_str)
#define DEFINE_double(name, default_value, help_str)  DEFINE_typed_var(double, name, default_value, help_str)
#define DEFINE_bool(name, default_value, help_str)    DEFINE_typed_var(bool, name, default_value, help_str)
#define DEFINE_string(name, default_value, help_str)  DEFINE_typed_var(std::string, name, default_value, help_str)

# endif