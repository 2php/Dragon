#include <map>
#include <cstdlib>
#include "utils/logging.hpp"
#include "utils/flags.hpp"

DLogSeverity d_global_destination;
std::map<std::string, int> d_global_count, d_global_every;
void SetLogDestination(DLogSeverity type) { d_global_destination = type; }
std::string itos(int n) { std::stringstream ss; ss << n; return ss.str(); }
std::string genHashKey(const char* file, int line){
	std::string hash_key = std::string(file) + itos(line);
	return hash_key;
}
int everyNRegister(const char* file, int line, int severity, int n){
	std::string hash_key = genHashKey(file, line);
	if (!d_global_every.count(hash_key)) d_global_every[hash_key] = n;
	if (++d_global_count[hash_key] == d_global_every[hash_key]){
		d_global_count[hash_key] = 0;
		return severity;
	}else return -1;
}

DragonMessageLogger::DragonMessageLogger(const char* file, int line, int severity) :severity(severity){
	if (severity < d_global_destination || severity == PYTHON) return;
	std::string filename_only;
	stripBasename(file, &filename_only);
	stream_ << DRAGON_SEVERITIES[severity]
		<< " " << __TIME__
		<< " " << filename_only << ":" << line << "] ";
}

DragonMessageLogger::~DragonMessageLogger(){
	if (severity < d_global_destination) return;
	stream_ << "\n";
	std::cerr << stream_.str() << std::flush;
	if (severity == FATAL) {
		std::cerr << "*** Check failure stack trace : ***" << std::endl;
		abort();
	}
}

void DragonMessageLogger::stripBasename(const std::string &full_path, std::string* filename){
	size_t pos1 = full_path.rfind('/'), pos2 = full_path.rfind('\\'), pos = std::string::npos;
	if (pos1 != std::string::npos) pos = pos1;
	if (pos2 != std::string::npos) pos = pos2;
	if (pos != std::string::npos)  *filename = full_path.substr(pos + 1, std::string::npos);
	else  *filename = full_path;
}
