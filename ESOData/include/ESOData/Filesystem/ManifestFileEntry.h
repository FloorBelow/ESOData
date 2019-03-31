#ifndef ESODATA_FILESYSTEM_MANIFEST_FILE_ENTRY_H
#define ESODATA_FILESYSTEM_MANIFEST_FILE_ENTRY_H

#include <stdint.h>

namespace esodata {
	class SerializationStream;

	enum class FileCompressionType : uint8_t {
		None,
		Deflate,
		Snappy
	};

	struct ManifestFileEntry {
		uint32_t uncompressedSize;
		uint32_t compressedSize;
		uint32_t fileCRC32;
		uint32_t fileOffset;
		FileCompressionType compressionType;
		uint8_t archiveIndex;
		uint16_t unknown;

		size_t cachedSize;
	};

	SerializationStream &operator <<(SerializationStream &stream, const ManifestFileEntry &entry);
	SerializationStream &operator >>(SerializationStream &stream, ManifestFileEntry &entry);
}

#endif
