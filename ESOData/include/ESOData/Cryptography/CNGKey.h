#ifndef CRYPTO__CNG_KEY__H
#define CRYPTO__CNG_KEY__H

#include <Windows.h>
#include <wincrypt.h>
#include <bcrypt.h>

#include <vector>

namespace esodata {
	class CNGAlgorithmProvider;

	class CNGKey {
	public:
		CNGKey() noexcept;
		CNGKey(CNGAlgorithmProvider &provider, ULONG length, ULONG flags);
		CNGKey(CNGAlgorithmProvider &provider, LPCWSTR blobType, const unsigned char *data, size_t size, ULONG flags);
		CNGKey(CNGAlgorithmProvider &provider, CNGKey *importKey, LPCWSTR blobType, const unsigned char *data, size_t size, ULONG flags);
		CNGKey(DWORD encodingType, PCERT_PUBLIC_KEY_INFO keyInfo, DWORD flags, void *auxInfo);
		~CNGKey() noexcept;

		CNGKey(const CNGKey &other) = delete;
		CNGKey &operator =(const CNGKey &other) = delete;

		CNGKey(CNGKey &&other) noexcept;
		CNGKey &operator =(CNGKey &&other) noexcept;

		void swap(CNGKey &other) noexcept;

		inline operator BCRYPT_KEY_HANDLE() {
			return m_handle;
		}

		void setProperty(LPCWSTR property, const unsigned char *data, size_t dataSize, ULONG flags);

		void finalize();

		static CNGKey importDERPublicKey(const std::vector<unsigned char> &publicKeyData);
		std::vector<unsigned char> exportDERPublicKey();

		void exportKey(LPCWSTR blobType, std::vector<unsigned char> &blob, ULONG flags);

		void signHash(
			void *paddingInfo,
			const unsigned char *input, size_t inputSize,
			std::vector<uint8_t> &output,
			ULONG flags);

		bool verifySignature(
			void *paddingInfo,
			const unsigned char *input, size_t inputSize,
			const unsigned char *signature, size_t signatureSize,
			ULONG flags
		);

		void encrypt(
			const unsigned char *input, size_t inputSize,
			void *paddingInfo,
			const unsigned char *iv, size_t ivSize,
			unsigned char *output, size_t outputSize,
			ULONG flags);

	private:
		BCRYPT_KEY_HANDLE m_handle;
		std::vector<uint8_t> m_keyData;
	};
}

#endif