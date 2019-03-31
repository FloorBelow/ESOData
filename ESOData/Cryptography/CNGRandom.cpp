#include <ESOData/Cryptography/CNGRandom.h>
#include <ESOData/Cryptography/CNGAlgorithmProvider.h>

#include <comdef.h>

namespace esodata {
	void CNGRandom::generateBytes(CNGAlgorithmProvider &algorithm, unsigned char *buffer, size_t bufferSize, ULONG flags) {
		auto result = HRESULT_FROM_NT(BCryptGenRandom(algorithm, buffer, static_cast<ULONG>(bufferSize), flags));
		if (FAILED(result))
			_com_raise_error(result);
	}
}
