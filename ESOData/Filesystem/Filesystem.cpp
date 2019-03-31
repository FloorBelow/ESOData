#include <ESOData/Filesystem/Filesystem.h>
#include <ESOData/Filesystem/Archive.h>
#include <ESOData/Filesystem/FileTable.h>

#include <sstream>

namespace esodata {
	Filesystem::Filesystem() = default;

	Filesystem::~Filesystem() = default;

	void Filesystem::addManifest(const std::string &filename) {
		m_archives.emplace_back(std::make_unique<Archive>(filename));
	}
	
	std::vector<unsigned char> Filesystem::readFileByKey(uint64_t key) {
		std::vector<unsigned char> data;
		for (const auto &archive : m_archives) {
			if (archive->readFileByKey(key, data)) {
				return data;
			}
		}

		std::stringstream sstream;
		sstream << "File not found: " << std::hex << key;
		throw std::runtime_error(sstream.str());
	}

	void Filesystem::loadFileTable(uint64_t fileTableKey) {
		auto fileTableData = readFileByKey(fileTableKey);

		InputSerializationStream stream(fileTableData.data(), fileTableData.data() + fileTableData.size());
		auto fileTable = std::make_unique<FileTable>();
		stream >> *fileTable;

		m_fileTables.emplace_back(std::move(fileTable));
	}

	void Filesystem::enumerateFiles(std::function<void(uint64_t, size_t size)> &&enumerator) {
		for (const auto &archive : m_archives) {
			archive->enumerateFiles(std::move(enumerator));
		}
	}
}
