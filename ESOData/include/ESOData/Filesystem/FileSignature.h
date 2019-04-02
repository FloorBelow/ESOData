#ifndef ESODATA_FILESYSTEM_FILE_SIGNATURE_H
#define ESODATA_FILESYSTEM_FILE_SIGNATURE_H

#include <ESOData/Serialization/SizedVector.h>

namespace esodata {
	struct FileSignature {
		uint32_t unknown;
		std::vector<uint8_t> publicKey;
		std::vector<uint8_t> signature;
	};

	SerializationStream &operator <<(SerializationStream &stream, const FileSignature &signature);
	SerializationStream &operator >>(SerializationStream &stream, FileSignature &signature);
}

#endif
