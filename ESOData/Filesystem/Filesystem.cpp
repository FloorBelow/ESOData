#include <ESOData/Filesystem/Filesystem.h>
#include <ESOData/Filesystem/Archive.h>
#include <ESOData/Filesystem/FileTable.h>

#include <sstream>

namespace esodata {
	Filesystem::Filesystem() = default;

	Filesystem::~Filesystem() = default;

	void Filesystem::addManifest(const std::filesystem::path &filename, bool needPreciseSizes) {
		m_archives.emplace_back(std::make_unique<Archive>(filename, needPreciseSizes));
	}
	
	std::vector<unsigned char> Filesystem::readFileByKey(uint64_t key) const {
		std::vector<unsigned char> data;

		if (!tryReadFileByKey(key, data)) {
			std::stringstream sstream;
			sstream << "File not found: " << std::hex << key;
			throw std::runtime_error(sstream.str());
		}

		return data;
	}

	bool Filesystem::tryReadFileByKey(uint64_t key, std::vector<unsigned char> &data) const {
		for (const auto &archive : m_archives) {
			if (archive->readFileByKey(key, data)) {
				return true;
			}
		}

		return false;
	}

	void Filesystem::loadFileTable(uint64_t fileTableKey) {
		auto fileTableData = readFileByKey(fileTableKey);

		InputSerializationStream stream(fileTableData.data(), fileTableData.data() + fileTableData.size());
		auto fileTable = std::make_unique<FileTable>();
		stream >> *fileTable;

		fileTable->globalIdPrefix = fileTableKey & 0xFFFFFFFE00000000ULL;

		m_fileTables.emplace_back(std::move(fileTable));
	}

	void Filesystem::enumerateFileNames(std::function<void(const std::string &name, uint64_t key)> &&enumerator) {
		for (const auto &tablePtr : m_fileTables) {
			const auto &table = *tablePtr;

			for (const auto &entry : table.entries) {
				auto key = entry.second.localFileKey | table.globalIdPrefix;

				auto nameBegin = table.nameHeap.begin() + entry.second.nameOffset;
				auto nameEnd = std::find(nameBegin, table.nameHeap.end(), '\0');
				std::string name(nameBegin, nameEnd);
				
#if 0
				auto additionalIt = table.additionalData.find(entry.first);
				if (additionalIt != table.additionalData.end()) {
					const auto &additionalData = (*additionalIt).second;

					__debugbreak();
				}
#endif
				enumerator(name, key);
			}
		}
	}

	void Filesystem::enumerateFiles(std::function<void(uint64_t, size_t size)> &&enumerator) {
		for (const auto &archive : m_archives) {
			archive->enumerateFiles(std::move(enumerator));
		}
	}
}
