#ifndef ESODATA_SERIALIZATION_SIZED_VECTOR_H
#define ESODATA_SERIALIZATION_SIZED_VECTOR_H

#include <ESOData/Serialization/SerializationStream.h>

namespace esodata {
	template<typename Size, typename Data>
	struct SizedVector {
		SizedVector(std::vector<Data> &data) : data(data) {

		}

		std::vector<Data> &data;
	};

	template<typename Size, typename Data>
	SerializationStream &operator <<(SerializationStream &stream, const SizedVector<Size, Data> &vector) {
		auto length = static_cast<Size>(vector.data.size());
		return stream << length << vector.data;
	}

	template<typename Size, typename Data>
	SerializationStream &operator >>(SerializationStream &stream, SizedVector<Size, Data> &vector) {
		Size length;
		stream >> length;
		vector.data.resize(length);
		return stream >> vector.data;
	}

	template<typename Size, typename Data>
	SizedVector<Size, Data> makeSizedVector(const std::vector<Data> &vector) {
		return SizedVector<Size, Data>(const_cast<std::vector<Data> &>(vector));
	}
}

#endif
