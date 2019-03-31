#ifndef ESODATA_FILESYSTEM_FILESYSTEM_H
#define ESODATA_FILESYSTEM_FILESYSTEM_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace esodata {
	class Archive;
	struct FileTable;

	class Filesystem {
	public:
		Filesystem();
		~Filesystem();

		Filesystem(const Filesystem &other) = delete;
		Filesystem &operator =(const Filesystem &other) = delete;

		void addManifest(const std::string &filename);

		void loadFileTable(uint64_t fileTableKey);

		std::vector<unsigned char> readFileByKey(uint64_t key);
		void enumerateFiles(std::function<void(uint64_t key, size_t size)> &&enumerator);

	private:
		std::vector<std::unique_ptr<Archive>> m_archives;
		std::vector<std::unique_ptr<FileTable>> m_fileTables;
	};
}

#endif
