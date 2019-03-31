#ifndef ESODATA_CRYPTOGRAPHY_CNG_ALGORITHM_PROVIDER_H
#define ESODATA_CRYPTOGRAPHY_CNG_ALGORITHM_PROVIDER_H
#include <Windows.h>
#include <bcrypt.h>


namespace esodata {
	class CNGAlgorithmProvider {
	public:
		CNGAlgorithmProvider() noexcept;
		CNGAlgorithmProvider(LPCWSTR algorithmId, LPCWSTR implementation, ULONG dwFlags);
		~CNGAlgorithmProvider() noexcept;

		CNGAlgorithmProvider(const CNGAlgorithmProvider &other) = delete;
		CNGAlgorithmProvider &operator =(const CNGAlgorithmProvider &other) = delete;

		CNGAlgorithmProvider(CNGAlgorithmProvider &&other) noexcept;
		CNGAlgorithmProvider &operator =(CNGAlgorithmProvider &&other) noexcept;

		void swap(CNGAlgorithmProvider &other) noexcept;

		inline operator BCRYPT_ALG_HANDLE() const {
			return m_handle;
		}

		void setProperty(LPCWSTR property, const unsigned char *data, size_t dataSize, ULONG flags);

	private:
		BCRYPT_ALG_HANDLE m_handle;
	};
}

#endif
