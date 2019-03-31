#ifndef INODE_H
#define INODE_H

#include <map>
#include <memory>
#include <string>

#include <Windows.h>
#include <projectedfslib.h>
#include <archiveparse/EncodingUtilities.h>

struct PJCompare {
	inline bool operator()(const std::wstring &a, const std::wstring &b) const {
		return PrjFileNameCompare(a.c_str(), b.c_str()) < 0;
	}
};

class Inode {
public:
	std::wstring name;
	std::wstring fullCanonicalPath;
	std::map<std::wstring, std::unique_ptr<Inode>, PJCompare> children;
	PRJ_FILE_BASIC_INFO info;

	uint64_t fileKey;
};

#endif

