#include <ESOData/Filesystem/DataFileHeader.h>

#include <ESOData/Serialization/SerializationStream.h>

#include <stdexcept>

namespace esodata {

	SerializationStream &operator <<(SerializationStream &stream, const DataFileHeader &header) {
		stream << DataFileHeader::Signature;
		stream << header.version;
		stream << header.unknown;
		stream << header.headerSize;
		return stream;
	}

	SerializationStream &operator >>(SerializationStream &stream, DataFileHeader &header) {
		uint32_t signature;

		stream >> signature;
		if (signature != DataFileHeader::Signature)
			throw std::runtime_error("Bad DAT file signature");

		stream >> header.version;
		stream >> header.unknown;
		stream >> header.headerSize;
		return stream;
	}
}
