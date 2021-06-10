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

	//TODO FIX ORDER BASED ON VERSION?
	SerializationStream &operator >>(SerializationStream &stream, ManifestFileEntry &entry) {
		stream >> entry.uncompressedSize
			>> entry.compressedSize
			>> entry.fileCRC32
			>> entry.fileOffset
			>> entry.archiveIndex
			>> entry.compressionType
			>> entry.unknown;

		return stream;
	}
}
