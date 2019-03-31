#include <ESOData/Cryptography/CNGKey.h>
#include <ESOData/Cryptography/CNGAlgorithmProvider.h>

#include <comdef.h>

#include <algorithm>

#define STATUS_INVALID_SIGNATURE         ((NTSTATUS)0xC000A000L)

namespace esodata {

	CNGKey::CNGKey() noexcept : m_handle(nullptr) {

	}

	CNGKey::CNGKey(CNGAlgorithmProvider &provider, ULONG length, ULONG flags) : m_handle(nullptr) {
		auto result = HRESULT_FROM_NT(BCryptGenerateKeyPair(provider, &m_handle, length, flags));
		if (FAILED(result))
			_com_raise_error(result);
	}

	CNGKey::CNGKey(CNGAlgorithmProvider &provider, CNGKey *importKey, LPCWSTR blobType, const unsigned char *data, size_t size, ULONG flags) {
		ULONG objectLength, objectLengthLength;

		auto result = HRESULT_FROM_NT(BCryptGetProperty(provider, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&objectLength), sizeof(objectLength), &objectLengthLength, 0));
		if (FAILED(result))
			_com_raise_error(result);

		m_keyData.resize(objectLength);

		result = HRESULT_FROM_NT(BCryptImportKey(
			provider, importKey ? static_cast<BCRYPT_KEY_HANDLE>(*importKey) : nullptr, blobType, &m_handle,
			m_keyData.data(), static_cast<ULONG>(m_keyData.size()), const_cast<PUCHAR>(data), static_cast<ULONG>(size), flags));
		if (FAILED(result))
			_com_raise_error(result);
	}

	CNGKey::CNGKey(DWORD encodingType, PCERT_PUBLIC_KEY_INFO keyInfo, DWORD flags, void *auxInfo) : m_handle(nullptr) {
		if (!CryptImportPublicKeyInfoEx2(encodingType, keyInfo, flags, auxInfo, &m_handle))
			_com_raise_error(HRESULT_FROM_WIN32(GetLastError()));
	}

	CNGKey::CNGKey(CNGAlgorithmProvider &provider, LPCWSTR blobType, const unsigned char *data, size_t size, ULONG flags) : m_handle(nullptr) {
		auto result = HRESULT_FROM_NT(BCryptImportKeyPair(provider,
			nullptr, blobType, &m_handle, const_cast<PUCHAR>(data), static_cast<ULONG>(size), flags));
		if (FAILED(result))
			_com_raise_error(result);
	}

	CNGKey::~CNGKey() noexcept {
		if (m_handle)
			BCryptDestroyKey(m_handle);
	}

	CNGKey::CNGKey(CNGKey &&other) noexcept : m_handle(nullptr) {
		swap(other);
	}

	CNGKey &CNGKey::operator =(CNGKey &&other) noexcept {
		swap(other);
		return *this;
	}

	void CNGKey::swap(CNGKey &other) noexcept {
		std::swap(m_handle, other.m_handle);
		m_keyData.swap(other.m_keyData);
	}

	void CNGKey::finalize() {
		auto result = HRESULT_FROM_NT(BCryptFinalizeKeyPair(m_handle, 0));
		if (FAILED(result))
			_com_raise_error(result);
	}

	std::vector<unsigned char> CNGKey::exportDERPublicKey() {
		DWORD size = 0;

		if (!CryptExportPublicKeyInfoFromBCryptKeyHandle(m_handle, X509_ASN_ENCODING, nullptr, 0, nullptr, nullptr, &size))
			_com_raise_error(HRESULT_FROM_WIN32(GetLastError()));

		std::vector<unsigned char> exportedKey(size);

		if (!CryptExportPublicKeyInfoFromBCryptKeyHandle(m_handle, X509_ASN_ENCODING, nullptr, 0, nullptr, (PCERT_PUBLIC_KEY_INFO)exportedKey.data(), &size))
			_com_raise_error(HRESULT_FROM_WIN32(GetLastError()));

		void *info = nullptr;
		DWORD infoSize = 0;

		if (!CryptEncodeObjectEx(X509_ASN_ENCODING, X509_PUBLIC_KEY_INFO, exportedKey.data(), CRYPT_ENCODE_ALLOC_FLAG, nullptr, &info, &infoSize)) {
			_com_raise_error(HRESULT_FROM_WIN32(GetLastError()));
		}

		struct InfoWatcher {
			InfoWatcher(void *info) : m_info(info) {}
			~InfoWatcher() {
				LocalFree(m_info);
			}
		private:
			void *m_info;
		} infoWatcher(info);

		return std::vector<unsigned char>((unsigned char *)info, (unsigned char *)info + infoSize);
	}


	CNGKey CNGKey::importDERPublicKey(const std::vector<unsigned char> &publicKeyData) {

		void *info;
		DWORD infoSize;

		if (!CryptDecodeObjectEx(
			X509_ASN_ENCODING, X509_PUBLIC_KEY_INFO, publicKeyData.data(), static_cast<ULONG>(publicKeyData.size()),
			CRYPT_DECODE_ALLOC_FLAG | CRYPT_DECODE_NOCOPY_FLAG | CRYPT_DECODE_SHARE_OID_STRING_FLAG,
			nullptr, &info, &infoSize))
			_com_raise_error(HRESULT_FROM_WIN32(GetLastError()));

		struct InfoWatcher {
			InfoWatcher(void *info) : m_info(info) {}
			~InfoWatcher() {
				LocalFree(m_info);
			}
		private:
			void *m_info;
		} infoWatcher(info);

		CERT_PUBLIC_KEY_INFO *publicKeyInfo = static_cast<CERT_PUBLIC_KEY_INFO *>(info);

		return CNGKey(X509_ASN_ENCODING, publicKeyInfo, 0, nullptr);
	}

	void CNGKey::signHash(
		void *paddingInfo,
		const unsigned char *input, size_t inputSize,
		std::vector<uint8_t> &output,
		ULONG flags) {

		ULONG outputSize = 0;

		auto result = HRESULT_FROM_NT(BCryptSignHash(
			m_handle, paddingInfo,
			const_cast<PUCHAR>(input), static_cast<ULONG>(inputSize),
			nullptr, 0, &outputSize, flags));
		if (FAILED(result))
			_com_raise_error(result);

		output.resize(outputSize);

		result = HRESULT_FROM_NT(BCryptSignHash(m_handle, paddingInfo,
			const_cast<PUCHAR>(input), static_cast<ULONG>(inputSize),
			output.data(), static_cast<ULONG>(output.size()), &outputSize, flags));
		if (FAILED(result))
			_com_raise_error(result);

		output.resize(outputSize);
	}

	bool CNGKey::verifySignature(
		void *paddingInfo,
		const unsigned char *input, size_t inputSize,
		const unsigned char *signature, size_t signatureSize,
		ULONG flags
	) {
		auto result = HRESULT_FROM_NT(BCryptVerifySignature(
			m_handle, paddingInfo,
			const_cast<PUCHAR>(input), static_cast<ULONG>(inputSize),
			const_cast<PUCHAR>(signature), static_cast<ULONG>(signatureSize),
			flags));

		if (result == HRESULT_FROM_NT(STATUS_INVALID_SIGNATURE))
			return false;

		if (FAILED(result))
			_com_raise_error(result);

		return true;
	}

	void CNGKey::setProperty(LPCWSTR property, const unsigned char *data, size_t dataSize, ULONG flags) {
		auto result = HRESULT_FROM_NT(BCryptSetProperty(
			m_handle, property, const_cast<PUCHAR>(data), static_cast<ULONG>(dataSize), flags
		));

		if (FAILED(result))
			_com_raise_error(result);
	}

	void CNGKey::exportKey(LPCWSTR blobType, std::vector<unsigned char> &blob, ULONG flags) {
		ULONG outputLength = 0;

		auto result = HRESULT_FROM_NT(BCryptExportKey(
			m_handle, nullptr, blobType, nullptr, 0, &outputLength, flags
		));

		if (FAILED(result))
			_com_raise_error(result);

		blob.resize(outputLength);

		result = HRESULT_FROM_NT(BCryptExportKey(
			m_handle, nullptr, blobType, blob.data(), static_cast<ULONG>(blob.size()), &outputLength, flags
		));

		blob.resize(outputLength);
	}

	void CNGKey::encrypt(
		const unsigned char *input, size_t inputSize,
		void *paddingInfo,
		const unsigned char *iv, size_t ivSize,
		unsigned char *output, size_t outputSize,
		ULONG flags) {

		ULONG outputRequired;

		auto hr = HRESULT_FROM_NT(BCryptEncrypt(
			m_handle,
			const_cast<PUCHAR>(input), static_cast<ULONG>(inputSize),
			paddingInfo,
			const_cast<PUCHAR>(iv), static_cast<ULONG>(ivSize),
			output, static_cast<ULONG>(outputSize), &outputRequired,
			flags));
		if (FAILED(hr))
			_com_raise_error(hr);
	}
}