#ifndef ESODATA_FILESYSTEM_FILE_TABLE_H
#define ESODATA_FILESYSTEM_FILE_TABLE_H

#include <ESOData/Serialization/HashTable.h>
#include <ESOData/Serialization/SizedVector.h>

#include <array>

namespace esodata {	
	struct FileTableEntry {
		uint32_t localFileKey;
		uint32_t nameOffset;
		uint64_t nameHash;
	};
	
	SerializationStream &operator <<(SerializationStream &stream, const FileTableEntry &entry);
	SerializationStream &operator >>(SerializationStream &stream, FileTableEntry &entry);

	struct FileTableAdditionalData {
		std::array<uint32_t, 11> unknown1;
	};

	SerializationStream &operator <<(SerializationStream &stream, const FileTableAdditionalData &entry);
	SerializationStream &operator >>(SerializationStream &stream, FileTableAdditionalData &entry);

	struct FileTable {
		uint16_t unknown1;
		uint32_t unknown2;
		uint32_t unknown3;
		uint32_t recordCount;
		HashTable<uint64_t, uint32_t> nameHashToLocalId;
		HashTable<uint32_t, FileTableEntry> entries;
		HashTable<uint32_t, FileTableAdditionalData> additionalData;
		SizedVector<uint32_t, char> nameHeap;

		uint64_t globalIdPrefix;
	};

	SerializationStream &operator <<(SerializationStream &stream, const FileTable &table);
	SerializationStream &operator >>(SerializationStream &stream, FileTable &table);
}

#endif
