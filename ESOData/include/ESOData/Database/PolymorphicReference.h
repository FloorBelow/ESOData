#ifndef ESODATA_DATABASE_POLYMORPHIC_REFERENCE_H
#define ESODATA_DATABASE_POLYMORPHIC_REFERENCE_H

#include <ESOData/Serialization/SerializationStream.h>

namespace esodata {
	template<typename T>
	class PolymorphicReference;

	template<typename T>
	SerializationStream& operator <<(SerializationStream& stream, const PolymorphicReference<T>& value);
	
	template<typename T>
	SerializationStream& operator >>(SerializationStream& stream, PolymorphicReference<T>& value);

	template<typename T>
	class PolymorphicReference {
	public:
		PolymorphicReference() = default;
		~PolymorphicReference() = default;

		PolymorphicReference(const PolymorphicReference& other) = default;
		PolymorphicReference& operator =(const PolymorphicReference& other) = default;

	private:
		friend SerializationStream& operator <<<T>(SerializationStream& stream, const PolymorphicReference<T>& value);
		friend SerializationStream& operator >><T>(SerializationStream& stream, PolymorphicReference<T>& value);

		T m_selector;
		uint32_t m_value;
	};

	template<typename T>
	SerializationStream& operator <<(SerializationStream& stream, const PolymorphicReference<T>& value) {
		return stream << value.m_selector << value.m_value;
	}

	template<typename T>
	SerializationStream& operator >>(SerializationStream& stream, PolymorphicReference<T>& value) {
		return stream >> value.m_selector >> value.m_value;
	}
}

#endif
