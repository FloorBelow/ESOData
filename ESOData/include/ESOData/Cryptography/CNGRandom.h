#ifndef ESODATA_CRYPTOGRAPHY_CNG_RANDOM_H
#define ESODATA_CRYPTOGRAPHY_CNG_RANDOM_H

#include <Windows.h>
#include <bcrypt.h>

namespace esodata {

	class CNGAlgorithmProvider;

	struct CNGRandom {
		static void generateBytes(CNGAlgorithmProvider &algorithm, unsigned char *buffer, size_t bufferSize, ULONG flags);
	};
}

#endif
