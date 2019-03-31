#ifndef ESODATA_CRYPTOGRAPHY_CNG_SECRET_H
#define ESODATA_CRYPTOGRAPHY_CNG_SECRET_H

#include <Windows.h>
#include <bcrypt.h>

#include <vector>

namespace esodata {
	class CNGKey;

	class CNGSecret {
	public:
		CNGSecret() noexcept;
		CNGSecret(CNGKey &privateKey, CNGKey &publicKey);
		~CNGSecret() noexcept;

		CNGSecret(const CNGSecret &other) = delete;
		CNGSecret &operator =(const CNGSecret &other) = delete;

		CNGSecret(CNGSecret &&other) noexcept;
		CNGSecret &operator =(CNGSecret &&other) noexcept;

		void swap(CNGSecret &other) noexcept;

		inline operator BCRYPT_SECRET_HANDLE() {
			return m_handle;
		}

		void derive(LPCWSTR kdf, const BCryptBufferDesc *parameters, std::vector<uint8_t> &derivedKey, ULONG flags);

	private:
		BCRYPT_SECRET_HANDLE m_handle;
	};
}

#endif
