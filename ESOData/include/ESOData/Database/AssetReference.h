#ifndef ESODATA_DATABASE_ASSET_REFERENCE_H
#define ESODATA_DATABASE_ASSET_REFERENCE_H

#include <stdint.h>

namespace esodata {
	class SerializationStream;

	class AssetReference {
	public:
		AssetReference();
		~AssetReference();

		AssetReference(const AssetReference& other);
		AssetReference &operator =(const AssetReference& other);

		inline bool isNull() const {
			return m_value == 0;
		}

		inline operator bool() const {
			return m_value != 0;
		}

		inline uint32_t id() const {
			return m_value;
		}

	private:
		friend SerializationStream& operator <<(SerializationStream& stream, const AssetReference& value);
		friend SerializationStream& operator >>(SerializationStream& stream, AssetReference& value);

		uint32_t m_value;
	};

	SerializationStream& operator <<(SerializationStream& stream, const AssetReference& value);
	SerializationStream& operator >>(SerializationStream& stream, AssetReference& value);
}

#endif
