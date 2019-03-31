#ifndef ESODATA_SERIALIZATION_HASH_H
#define ESODATA_SERIALIZATION_HASH_H

#include <stdint.h>

namespace esodata {
	uint32_t hashDataJenkins(const unsigned char *data, size_t dataSize);
	uint32_t hashDataDJB2(const unsigned char *data, size_t dataSize);
	uint64_t hashData64(const unsigned char *data, size_t dataSize);
}

#endif
