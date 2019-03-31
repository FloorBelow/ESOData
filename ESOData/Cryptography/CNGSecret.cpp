#include <ESOData/Cryptography/CNGSecret.h>
#include <ESOData/Cryptography/CNGKey.h>

#include <comdef.h>

#include <algorithm>

namespace esodata {
	CNGSecret::CNGSecret() noexcept : m_handle(nullptr) {

	}

	CNGSecret::CNGSecret(CNGKey &privateKey, CNGKey &publicKey) : m_handle(nullptr) {
		auto result = HRESULT_FROM_NT(BCryptSecretAgreement(privateKey, publicKey, &m_handle, 0));
		if (FAILED(result))
			_com_raise_error(result);
	}

	CNGSecret::~CNGSecret() noexcept {
		if (m_handle) {
			BCryptDestroySecret(m_handle);
		}
	}

	CNGSecret::CNGSecret(CNGSecret &&other) noexcept : m_handle(nullptr) {
		swap(other);
	}

	CNGSecret &CNGSecret::operator =(CNGSecret &&other) noexcept {
		swap(other);

		return *this;
	}

	void CNGSecret::swap(CNGSecret &other) noexcept {
		std::swap(m_handle, other.m_handle);
	}

	void CNGSecret::derive(LPCWSTR kdf, const BCryptBufferDesc *parameters, std::vector<uint8_t> &derivedKey, ULONG flags) {
		ULONG outLength = 0;

		auto status = HRESULT_FROM_NT(BCryptDeriveKey(m_handle,
			kdf, const_cast<BCryptBufferDesc *>(parameters), nullptr, 0, &outLength, flags));
		if (FAILED(status))
			_com_raise_error(status);

		derivedKey.resize(outLength);

		status = HRESULT_FROM_NT(BCryptDeriveKey(m_handle,
			kdf, const_cast<BCryptBufferDesc *>(parameters),
			derivedKey.data(), static_cast<ULONG>(derivedKey.size()), &outLength, flags));
		if (FAILED(status))
			_com_raise_error(status);

		derivedKey.resize(outLength);
	}
}
