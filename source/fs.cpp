#include "3dsware/fs.h"
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

namespace Hardware {

bool FS::ensureDirectory(const std::string& path) {
	if (path.empty()) return false;
	
	std::string currentPath;
	size_t start = 0;
	
	if (path.compare(0, 6, "sdmc:/") == 0) {
		currentPath = "sdmc:/";
		start = 6;
	} else if (path.compare(0, 1, "/") == 0) {
		currentPath = "/";
		start = 1;
	}
	
	while (start < path.length()) {
		size_t pos = path.find('/', start);
		if (pos == std::string::npos) {
			pos = path.length();
		}
		
		currentPath += path.substr(start, pos - start);
		
		struct stat st;
		if (stat(currentPath.c_str(), &st) != 0) {
			if (mkdir(currentPath.c_str(), 0777) != 0) {
				return false;
			}
		} else if (!S_ISDIR(st.st_mode)) {
			return false; // Esiste ma non è una directory
		}
		
		currentPath += "/";
		start = pos + 1;
	}
	
	return true;
}

bool FS::fileExists(const std::string& path) {
	struct stat st;
	if (stat(path.c_str(), &st) != 0) return false;
	return S_ISREG(st.st_mode);
}

size_t FS::fileSize(const std::string& path) {
	struct stat st;
	if (stat(path.c_str(), &st) != 0) return 0;
	return st.st_size;
}

bool FS::removeFile(const std::string& path) {
	if (!fileExists(path)) return true;
	return unlink(path.c_str()) == 0;
}

} // namespace Hardware
