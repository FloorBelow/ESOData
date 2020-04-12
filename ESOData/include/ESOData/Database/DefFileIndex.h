#ifndef ESODATA_DATABASE_DEF_FILE_INDEX_H
#define ESODATA_DATABASE_DEF_FILE_INDEX_H

#include <memory>
#include <vector>

namespace esodata {
	class Filesystem;
	class SerializationStream;

	struct DefFileIndex {
		struct LookupRecord {
			uint32_t index;
			uint32_t offset;

			friend SerializationStream& operator <<(SerializationStream& stream, const DefFileIndex::LookupRecord& value);
			friend SerializationStream& operator >>(SerializationStream& stream, DefFileIndex::LookupRecord& value);
		};

		static constexpr uint32_t Magic = 0xFBFBECECU;

		uint32_t version;
		bool unk1;
		bool unk2;
		uint32_t unk3;
		int32_t unk4;
		uint32_t highestKey;
		std::vector<LookupRecord> lookupRecords;

		friend SerializationStream& operator <<(SerializationStream& stream, const DefFileIndex& value);
		friend SerializationStream& operator >>(SerializationStream& stream, DefFileIndex& value);

		static std::unique_ptr<DefFileIndex> readFromFilesystem(const Filesystem& filesystem, uint64_t fileId);
		static std::unique_ptr<DefFileIndex> readFromData(const std::vector<unsigned char>& data);
	};

	SerializationStream& operator <<(SerializationStream& stream, const DefFileIndex& value);
	SerializationStream& operator >>(SerializationStream& stream, DefFileIndex& value);

	SerializationStream& operator <<(SerializationStream& stream, const DefFileIndex::LookupRecord& value);
	SerializationStream& operator >>(SerializationStream& stream, DefFileIndex::LookupRecord& value);

}

#endif
