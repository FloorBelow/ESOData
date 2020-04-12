#include <ESOData/Database/DefFileIndex.h>

#include <ESOData/Filesystem/Filesystem.h>

#include <ESOData/Serialization/InputSerializationStream.h>
#include <ESOData/Serialization/SizedVector.h>

namespace esodata {
	std::unique_ptr<DefFileIndex> DefFileIndex::readFromFilesystem(const Filesystem& filesystem, uint64_t fileId) {
		return readFromData(filesystem.readFileByKey(fileId));
	}

	std::unique_ptr<DefFileIndex> DefFileIndex::readFromData(const std::vector<unsigned char>& data) {
		InputSerializationStream stream(data.data(), data.data() + data.size());
		stream.setSwapEndian(true);

		auto index = std::make_unique<DefFileIndex>();
		stream >> *index;

		return index;
	}

	SerializationStream& operator <<(SerializationStream& stream, const DefFileIndex& value) {
		stream << DefFileIndex::Magic;
		stream << value.version;
		if (value.version >= 1) {
			stream << value.unk1;

			if (value.version >= 2) {
				stream << value.unk2;
			}

			if (value.version >= 3) {
				stream << value.unk3;
			}

			if (value.version >= 4) {
				stream << value.unk4;
			}
			
			stream << value.highestKey;
			stream << makeSizedVector<uint32_t>(value.lookupRecords);
		}

		return stream;
	}

	SerializationStream& operator >>(SerializationStream& stream, DefFileIndex& value) {
		uint32_t magic;
		stream >> magic;

		if (magic != DefFileIndex::Magic)
			throw std::runtime_error("bad def index magic");

		value.unk1 = false;
		value.unk2 = false;
		value.unk3 = 3;
		value.unk4 = -2;

		stream >> value.version;
		if (value.version >= 1) {
			stream >> value.unk1;


			if (value.version >= 2) {
				stream >> value.unk2;
			}

			if (value.version >= 3) {
				stream >> value.unk3;
			}

			if (value.version >= 4) {
				stream >> value.unk4;
			}

			stream >> value.highestKey;
			stream >> makeSizedVector<uint32_t>(value.lookupRecords);
		}
		return stream;
	}

	SerializationStream& operator <<(SerializationStream& stream, const DefFileIndex::LookupRecord& value) {
		return stream << value.index << value.offset;
	}

	SerializationStream& operator >>(SerializationStream& stream, DefFileIndex::LookupRecord& value) {
		return stream >> value.index >> value.offset;
	}
}
