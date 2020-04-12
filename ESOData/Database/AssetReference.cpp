#include <ESOData/Database/AssetReference.h>
#include <ESOData/Serialization/SerializationStream.h>

namespace esodata {
	AssetReference::AssetReference() = default;
	AssetReference::~AssetReference() = default;

	AssetReference::AssetReference(const AssetReference& other) = default;
	AssetReference& AssetReference::operator =(const AssetReference& other) = default;

	SerializationStream& operator <<(SerializationStream& stream, const AssetReference& value) {
		return stream << value.m_value;
	}

	SerializationStream& operator >>(SerializationStream& stream, AssetReference& value) {
		return stream >> value.m_value;
	}
}
