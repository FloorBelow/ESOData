#include <ESOData/Filesystem/ManifestFileEntry.h>

#include <ESOData/Serialization/SerializationStream.h>

namespace esodata {

	SerializationStream &operator <<(SerializationStream &stream, const ManifestFileEntry &entry) {
		stream << entry.uncompressedSize
			<< entry.compressedSize
			<< entry.fileCRC32
			<< entry.fileOffset
			<< entry.compressionType
			<< entry.archiveIndex
			<< entry.unknown;

		return stream;
	}

	SerializationStream &operator >>(SerializationStream &stream, ManifestFileEntry &entry) {
		stream >> entry.uncompressedSize
			>> entry.compressedSize
			>> entry.fileCRC32
			>> entry.fileOffset
			>> entry.compressionType
			>> entry.archiveIndex
			>> entry.unknown;

		return stream;
	}
}
