#ifndef ESODATA_DATABASE_FOREIGN_KEY_H
#define ESODATA_DATABASE_FOREIGN_KEY_H

#include <ESOData/Serialization/SerializationStream.h>

namespace esodata {
	template<typename T>
	class ForeignKey;

	template<typename T>
	SerializationStream& operator <<(SerializationStream& stream, const ForeignKey<T>& value);

	template<typename T>
	SerializationStream& operator >>(SerializationStream& stream, ForeignKey<T>& value);
	
	template<typename T>
	class ForeignKey {
	public:
		ForeignKey() = default;
		~ForeignKey() = default;

		ForeignKey(const ForeignKey& other) = default;
		ForeignKey &operator =(const ForeignKey& other) = default;

	private:
		friend SerializationStream& operator <<<T>(SerializationStream& stream, const ForeignKey<T>& value);
		friend SerializationStream& operator >><T>(SerializationStream& stream, ForeignKey<T>& value);

		uint32_t m_value;
	};

	template<typename T>
	SerializationStream& operator <<(SerializationStream& stream, const ForeignKey<T>& value) {
		return stream << value.m_value;
	}

	template<typename T>
	SerializationStream& operator >>(SerializationStream& stream, ForeignKey<T>& value) {
		return stream >> value.m_value;
	}
}

#endif
