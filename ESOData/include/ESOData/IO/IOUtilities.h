#ifndef ESODATA_IO_IOUTILITIES_H
#define ESODATA_IO_IOUTILITIES_H

#include <vector>
#include <string>

namespace esodata {
	std::vector<unsigned char> readWholeFile(const std::string &filename);
}

#endif
