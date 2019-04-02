#ifndef ESODATA_SERIALIZATION_DEFLATED_SEGMENT_H
#define ESODATA_SERIALIZATION_DEFLATED_SEGMENT_H

#include <ESOData/Serialization/SerializationStream.h>
#include <ESOData/Serialization/InputSerializationStream.h>
#include <ESOData/Serialization/OutputSerializationStream.h>
#include <ESOData/Serialization/SizedSegment.h>

namespace esodata {
	class SerializationStream;

	std::vector<unsigned char> zlibCompress(const unsigned char *inputData, size_t inputLength);
	void zlibUncompress(const unsigned char *inputData, size_t inputLength, unsigned char *outputData, size_t outputLength);

	template<typename T, ByteswapMode Mode = ByteswapMode::Keep>
	struct DeflatedSegment {
		DeflatedSegment(T &data) : data(data) {

		}

		T &data;
	};

	template<typename T, ByteswapMode Mode = ByteswapMode::Keep>
	DeflatedSegment<T, Mode> makeDeflatedSegment(const T &val) {
		return DeflatedSegment<T, Mode>(const_cast<T &>(val));
	}

	template<typename T, ByteswapMode Mode>
	SerializationStream &operator <<(SerializationStream &stream, const DeflatedSegment<T, Mode> &segment) {
		OutputSerializationStream nestedStream;

		if constexpr (Mode == ByteswapMode::Keep) {
			nestedStream.setSwapEndian(stream.swapEndian());
		}
		else if constexpr (Mode == ByteswapMode::Enable) {
			nestedStream.setSwapEndian(true);
		}
		else {
			nestedStream.setSwapEndian(false);
		}

		nestedStream << segment.data;

		auto &&uncompressedData = std::move(nestedStream.data());

		auto compressedData = zlibCompress(uncompressedData.data(), uncompressedData.size());

		uint32_t uncompressedLength = static_cast<uint32_t>(uncompressedData.size());
		uint32_t compressedLength = static_cast<uint32_t>(compressedData.size());

		stream << uncompressedLength << compressedLength << compressedData;

		return stream;
	}

	template<typename T, ByteswapMode Mode>
	SerializationStream &operator >>(SerializationStream &stream, DeflatedSegment<T, Mode> &segment) {
		uint32_t uncompressedLength;
		uint32_t compressedLength;

		stream >> uncompressedLength >> compressedLength;

		auto compressedData = stream.getRegionForRead(compressedLength);
		std::vector<unsigned char> uncompressedData(uncompressedLength);

		zlibUncompress(compressedData, compressedLength, uncompressedData.data(), uncompressedData.size());

		InputSerializationStream nestedStream(uncompressedData.data(), uncompressedData.data() + uncompressedData.size());

		if constexpr (Mode == ByteswapMode::Keep) {
			nestedStream.setSwapEndian(stream.swapEndian());
		}
		else if constexpr (Mode == ByteswapMode::Enable) {
			nestedStream.setSwapEndian(true);
		}
		else {
			nestedStream.setSwapEndian(false);
		}

		nestedStream >> segment.data;

		return stream;
	}
}

#endif
