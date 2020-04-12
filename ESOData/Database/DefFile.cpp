#include <ESOData/Database/DefFile.h>

#include <ESOData/Serialization/InputSerializationStream.h>
#include <ESOData/Serialization/DeflatedSegment.h>

#include <stdexcept>

namespace esodata {
	void DefFileHeader::readFromData(const std::vector<unsigned char>& data, size_t &offset) {
		InputSerializationStream stream(data.data() + offset, data.data() + data.size());
		stream.setSwapEndian(true);

		stream >> *this;

		offset += stream.getCurrentPosition();
	}


	SerializationStream& operator <<(SerializationStream& stream, const DefFileHeader& value) {
		stream << DefFileHeader::FlagsPresentMagic;
		stream << value.flags;
		stream << value.itemCount;

		return stream;
	}

	SerializationStream& operator >>(SerializationStream& stream, DefFileHeader& value) {
		value.flags = 0;
		value.itemCount = 0;

		stream >> value.itemCount;

		if (value.itemCount == DefFileHeader::FlagsPresentMagic) {
			stream >> value.flags;
			stream >> value.itemCount;
		}

		stream >> value.version;

		return stream;
	}

	void DefFileRow::readFromData(const std::vector<unsigned char>& data, size_t& offset) {
		InputSerializationStream stream(data.data() + offset, data.data() + data.size());
		stream.setSwapEndian(true);

		stream >> *this;

		offset += stream.getCurrentPosition();
	}


	SerializationStream& operator <<(SerializationStream& stream, const DefFileRow& value) {
		return stream <<
			static_cast<uint32_t>(value.recordData.size()) <<
			esodata::makeDeflatedSegment(value.recordData);
	}

	SerializationStream& operator >>(SerializationStream& stream, DefFileRow& value) {
		uint32_t expectedLength;
		stream >> expectedLength;

		value.recordData.resize(expectedLength);
		return stream >> esodata::makeDeflatedSegment(value.recordData);
	}
}
