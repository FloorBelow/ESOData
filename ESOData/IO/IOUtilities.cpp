#include <ESOData/IO/IOUtilities.h>

#include <archiveparse/EncodingUtilities.h>

#include <fstream>

namespace esodata {
	std::vector<unsigned char> readWholeFile(const std::string &filename) {
		std::ifstream stream;
		stream.exceptions(std::ios::failbit | std::ios::eofbit | std::ios::badbit);
		stream.open(archiveparse::utf8ToWide(filename).c_str(), std::ios::in | std::ios::binary);

		stream.seekg(0, std::ios::end);
		auto size = static_cast<size_t>(stream.tellg());
		stream.seekg(0);

		std::vector<unsigned char> data(size);
		stream.read(reinterpret_cast<char *>(data.data()), data.size());

		return data;
	}
}
