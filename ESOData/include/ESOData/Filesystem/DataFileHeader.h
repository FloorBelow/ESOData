#ifndef ESODATA_FILESYSTEM_DATA_FILE_HEADER_H
#define ESODATA_FILESYSTEM_DATA_FILE_HEADER_H

#include <stdint.h>

namespace esodata {

	class SerializationStream;

	struct DataFileHeader {
		enum : uint32_t {
			Signature = 0x32534550, // Little-endian 'PES2'
			ExpectedVersion = 0x01
		};

		uint32_t signature;
		uint16_t version;
		uint32_t unknown;
		uint32_t headerSize;
	};

	SerializationStream &operator <<(SerializationStream &stream, const DataFileHeader &header);
	SerializationStream &operator >>(SerializationStream &stream, DataFileHeader &header);
}

#endif
