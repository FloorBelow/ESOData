#ifndef ESODATA_DATABASE_DEF_FILE_H
#define ESODATA_DATABASE_DEF_FILE_H

#include <vector>

#include <stdint.h>

namespace esodata {
	class SerializationStream;

	struct DefFileHeader {
		static constexpr uint32_t FlagsPresentMagic = 0xFAFAEBEBU;

		uint32_t flags;
		uint32_t itemCount;
		uint32_t version;

		void readFromData(const std::vector<unsigned char>& data, size_t &offset);

		friend SerializationStream& operator <<(SerializationStream& stream, const DefFileHeader& value);
		friend SerializationStream& operator >>(SerializationStream& stream, DefFileHeader& value);
	};

	struct DefFileRow {
		std::vector<unsigned char> recordData;

		void readFromData(const std::vector<unsigned char>& data, size_t& offset);

		friend SerializationStream& operator <<(SerializationStream& stream, const DefFileRow& value);
		friend SerializationStream& operator >>(SerializationStream& stream, DefFileRow& value);
	};

	SerializationStream& operator <<(SerializationStream& stream, const DefFileHeader& value);
	SerializationStream& operator >>(SerializationStream& stream, DefFileHeader& value);

	SerializationStream& operator <<(SerializationStream& stream, const DefFileRow& value);
	SerializationStream& operator >>(SerializationStream& stream, DefFileRow& value);
}

#endif
