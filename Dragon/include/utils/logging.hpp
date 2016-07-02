# ifndef LOGGING_HPP
# define LOGGING_HPP

#include <iostream>
#include <sstream>
enum DLogSeverity { INFO, WARNING, ERROR, FATAL, PYTHON};
static std::string DRAGON_SEVERITIES[] = { "INFO", "WARNING", "ERROR", "FATAL","PYTHON"};
void SetLogDestination(DLogSeverity type);
int everyNRegister(const char* file, int line, int severity, int n);
std::string itos(int n);

class DragonMessageLogger{
public:
	DragonMessageLogger(const char* file, int line, int severity);
	~DragonMessageLogger();
	void stripBasename(const std::string &full_path, std::string* filename);
	std::stringstream& stream() { return stream_; }
private:
	int severity;
	std::stringstream stream_;
};

#define DRAGON_FATAL_IF(condition) condition? DragonMessageLogger("-1", -1, -1).stream():DragonMessageLogger(__FILE__, __LINE__, FATAL).stream()
#define CHECK(condition)  DRAGON_FATAL_IF(condition)<< "Check failed: "#condition" "
#define CHECK_OP(val1,val2,op) DRAGON_FATAL_IF(val1 op val2) << "Check failed: " #val1 " " #op " " #val2 " (" #val1 " vs. " #val2 ") "
#define CHECK_EQ(val1,val2) CHECK_OP(val1,val2,==)
#define CHECK_NE(val1,val2) CHECK_OP(val1,val2,!=)
#define CHECK_GT(val1,val2) CHECK_OP(val1,val2,>)
#define CHECK_GE(val1,val2) CHECK_OP(val1,val2,>=)
#define CHECK_LT(val1,val2) CHECK_OP(val1,val2,<)
#define CHECK_LE(val1,val2) CHECK_OP(val1,val2,<=)
#define LOG(severity) DragonMessageLogger(__FILE__,__LINE__,severity).stream()
#define LOG_IF(severity,condition) if(condition) DragonMessageLogger(__FILE__,__LINE__,severity).stream()
#define LOG_EVERY_N(severity,n) DragonMessageLogger(__FILE__,__LINE__,everyNRegister(__FILE__,__LINE__,severity,n)).stream()

# endif