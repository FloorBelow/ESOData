#ifndef ESODATA_FILESYSTEM_ARCHIVE_H
#define ESODATA_FILESYSTEM_ARCHIVE_H

#include <vector>
#include <functional>

#include <ESOData/Filesystem/MNFFile.h>

#include <archiveparse/WindowsHandle.h>

#include <Windows.h>

namespace esodata {
	class Archive {
	public:
		explicit Archive(const std::string &manifestFilename, bool needPreciseSizes);
		~Archive();

		Archive(const Archive &other) = delete;
		Archive &operator =(const Archive &other) = delete;

		bool readFileByKey(uint64_t key, std::vector<unsigned char> &data);
		void enumerateFiles(std::function<void(uint64_t key, size_t size)> &&enumerator);

	private:
		MNFFile m_manifest;
		std::vector<archiveparse::WindowsHandle> m_files;
	};
}

#endif

