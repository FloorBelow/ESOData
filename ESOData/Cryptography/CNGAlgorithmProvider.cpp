#include <ESOData/Cryptography/CNGAlgorithmProvider.h>

#include <comdef.h>

#include <algorithm>

namespace esodata {

	CNGAlgorithmProvider::CNGAlgorithmProvider() noexcept : m_handle(nullptr) {

	}

	CNGAlgorithmProvider::CNGAlgorithmProvider(LPCWSTR algorithmId, LPCWSTR implementation, ULONG dwFlags) : m_handle(nullptr) {
		auto status = HRESULT_FROM_NT(BCryptOpenAlgorithmProvider(&m_handle, algorithmId, implementation, dwFlags));
		if (FAILED(status))
			_com_raise_error(status);
	}

	CNGAlgorithmProvider::~CNGAlgorithmProvider() noexcept {
		if (m_handle)
			BCryptCloseAlgorithmProvider(m_handle, 0);
	}

	CNGAlgorithmProvider::CNGAlgorithmProvider(CNGAlgorithmProvider &&other) noexcept : m_handle(nullptr) {
		swap(other);
	}

	CNGAlgorithmProvider &CNGAlgorithmProvider::operator =(CNGAlgorithmProvider &&other) noexcept {
		swap(other);

		return *this;
	}

	void CNGAlgorithmProvider::swap(CNGAlgorithmProvider &other) noexcept {
		std::swap(m_handle, other.m_handle);
	}

	void CNGAlgorithmProvider::setProperty(LPCWSTR property, const unsigned char *data, size_t dataSize, ULONG flags) {
		auto result = HRESULT_FROM_NT(BCryptSetProperty(
			m_handle, property, const_cast<PUCHAR>(data), static_cast<ULONG>(dataSize), flags
		));

		if (FAILED(result))
			_com_raise_error(result);
	}
}
