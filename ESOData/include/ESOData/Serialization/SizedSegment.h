#ifndef ESODATA_SERIALIZATION_SIZED_SEGMENT_H
#define ESODATA_SERIALIZATION_SIZED_SEGMENT_H

#include <ESOData/Serialization/InputSerializationStream.h>
#include <ESOData/Serialization/OutputSerializationStream.h>

namespace esodata {
	enum class ByteswapMode {
		Keep,
		Enable,
		Disable
	};

	template<typename T, ByteswapMode Mode = ByteswapMode::Keep>
	struct SizedSegment {
		T data;
	};

	template<typename T, ByteswapMode Mode>
	SerializationStream &operator <<(SerializationStream &stream, const SizedSegment<T, Mode> &data) {
		auto position = stream.getCurrentPosition();

		stream.getRegionForWrite(sizeof(uint32_t));

		OutputSerializationStream nestedStream(&stream);

		if constexpr (Mode == ByteswapMode::Keep) {
			nestedStream.setSwapEndian(stream.swapEndian());
		}
		else if constexpr (Mode == ByteswapMode::Enable) {
			nestedStream.setSwapEndian(true);
		}
		else {
			nestedStream.setSwapEndian(false);
		}

		nestedStream << data.data;

		auto length = nestedStream.getCurrentPosition();

		auto savedPosition = stream.getCurrentPosition();

		stream.setCurrentPosition(position);
		stream << static_cast<uint32_t>(length);
		stream.setCurrentPosition(savedPosition);

		return stream;
	}

	template<typename T, ByteswapMode Mode>
	SerializationStream &operator >>(SerializationStream &stream, SizedSegment<T, Mode> &data) {
		uint32_t length;
		stream >> length;

		auto dataPtr = stream.getRegionForRead(length);
		InputSerializationStream nestedStream(dataPtr, dataPtr + length);

		if constexpr (Mode == ByteswapMode::Keep) {
			nestedStream.setSwapEndian(stream.swapEndian());
		}
		else if constexpr (Mode == ByteswapMode::Enable) {
			nestedStream.setSwapEndian(true);
		}
		else {
			nestedStream.setSwapEndian(false);
		}

		nestedStream >> data.data;

		return stream;
	}
}

#endif
