#ifndef ESODATA_CRYPTOGRAPHY_CNG_HASH_H
#define ESODATA_CRYPTOGRAPHY_CNG_HASH_H

#include <Windows.h>
#include <bcrypt.h>

#include <vector>

namespace esodata {
	class CNGAlgorithmProvider;

	class CNGHash {
	public:
		CNGHash() noexcept;
		CNGHash(CNGAlgorithmProvider &algorithm, const unsigned char *secret, size_t secretLength, ULONG flags);
		~CNGHash() noexcept;

		CNGHash(const CNGHash &other) = delete;
		CNGHash &operator =(const CNGHash &other) = delete;

		CNGHash(CNGHash &&other) noexcept;
		CNGHash &operator =(CNGHash &&other) noexcept;

		inline operator BCRYPT_HASH_HANDLE() {
			return m_handle;
		}

		void swap(CNGHash &other) noexcept;

		void hashData(const unsigned char *data, size_t dataSize);
		void finish(std::vector<uint8_t> &digest);
		void finish(uint8_t *digest, size_t digestSize);

	private:
		BCRYPT_HASH_HANDLE m_handle;
		std::vector<uint8_t> m_hashObject;
	};
}

#endif
