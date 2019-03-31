#include <ESOData/Cryptography/CNGHash.h>
#include <ESOData/Cryptography/CNGAlgorithmProvider.h>

#include <comdef.h>

#include <algorithm>

namespace esodata {
	CNGHash::CNGHash() noexcept : m_handle(nullptr) {

	}

	CNGHash::CNGHash(CNGAlgorithmProvider &algorithm, const unsigned char *secret, size_t secretLength, ULONG flags) : m_handle(nullptr) {
		DWORD objectLength;
		ULONG objectLengthLength;

		auto status = HRESULT_FROM_NT(BCryptGetProperty(
			algorithm,
			BCRYPT_OBJECT_LENGTH,
			reinterpret_cast<PUCHAR>(&objectLength),
			sizeof(objectLength),
			&objectLengthLength,
			0));

		if (FAILED(status))
			_com_raise_error(status);

		m_hashObject.resize(objectLength);

		status = HRESULT_FROM_NT(BCryptCreateHash(
			algorithm,
			&m_handle,
			m_hashObject.data(),
			static_cast<ULONG>(m_hashObject.size()),
			const_cast<PUCHAR>(secret),
			static_cast<ULONG>(secretLength),
			flags));

		if (FAILED(status))
			_com_raise_error(status);
	}

	CNGHash::~CNGHash() noexcept {
		if (m_handle)
			BCryptDestroyHash(m_handle);
	}

	CNGHash::CNGHash(CNGHash &&other) noexcept : m_handle(nullptr) {
		swap(other);
	}

	CNGHash &CNGHash::operator =(CNGHash &&other) noexcept {
		swap(other);

		return *this;
	}

	void CNGHash::swap(CNGHash &other) noexcept {
		std::swap(m_handle, other.m_handle);
		m_hashObject.swap(other.m_hashObject);
	}

	void CNGHash::hashData(const unsigned char *data, size_t dataSize) {
		auto status = HRESULT_FROM_NT(BCryptHashData(m_handle, const_cast<PUCHAR>(data), static_cast<ULONG>(dataSize), 0));
		if (FAILED(status))
			_com_raise_error(status);
	}

	void CNGHash::finish(std::vector<uint8_t> &digest) {
		DWORD hashLength;
		ULONG hashLengthLength;

		auto status = HRESULT_FROM_NT(BCryptGetProperty(
			m_handle,
			BCRYPT_HASH_LENGTH,
			reinterpret_cast<PUCHAR>(&hashLength),
			sizeof(hashLength),
			&hashLengthLength,
			0));

		if (FAILED(status))
			_com_raise_error(status);

		digest.resize(hashLength);

		status = HRESULT_FROM_NT(BCryptFinishHash(
			m_handle, digest.data(), static_cast<ULONG>(digest.size()), 0
		));

		if (FAILED(status))
			_com_raise_error(status);
	}

	void CNGHash::finish(uint8_t *digest, size_t digestSize) {
		auto status = HRESULT_FROM_NT(BCryptFinishHash(m_handle, digest, static_cast<ULONG>(digestSize), 0));

		if (FAILED(status))
			_com_raise_error(status);

	}
}
