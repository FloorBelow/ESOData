#include <ESOData/Filesystem/FileTable.h>

#include <array>

namespace esodata {
	static const std::array<unsigned char, 5> FileTableExpectedSignature{ 'Z', 'O', 'S', 'F', 'T' };

	SerializationStream &operator <<(SerializationStream &stream, const FileTableEntry &entry) {
		return stream << entry.localFileKey << entry.nameOffset << entry.nameHash;		
	}

	SerializationStream &operator >>(SerializationStream &stream, FileTableEntry &entry) {
		return stream >> entry.localFileKey >> entry.nameOffset >> entry.nameHash;
	}

	SerializationStream &operator <<(SerializationStream &stream, const FileTableAdditionalData &entry) {
		return stream << entry.unknown1;
	}

	SerializationStream &operator >>(SerializationStream &stream, FileTableAdditionalData &entry) {
		return stream >> entry.unknown1;
	}

	SerializationStream &operator <<(SerializationStream &stream, const FileTable &table) {
		return
			stream
			<< FileTableExpectedSignature
			<< table.unknown1
			<< table.unknown2
			<< table.unknown3
			<< table.recordCount
			<< table.nameHashToLocalId
			<< table.entries
			<< table.additionalData
			<< table.nameHeap
			<< FileTableExpectedSignature;
	}

	SerializationStream &operator >>(SerializationStream &stream, FileTable &table) {
		std::array<unsigned char, 5> signature;
		stream >> signature;

		if (signature != FileTableExpectedSignature)
			throw std::logic_error("bad ZOSFT signature");

		stream
			>> table.unknown1
			>> table.unknown2
			>> table.unknown3
			>> table.recordCount
			>> table.nameHashToLocalId
			>> table.entries
			>> table.additionalData
			>> table.nameHeap;

		stream >> signature;

		if (signature != FileTableExpectedSignature)
			throw std::logic_error("bad ZOSFT signature");

		return stream;
	}
}
