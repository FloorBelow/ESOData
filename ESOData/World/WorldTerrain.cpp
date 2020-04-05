#include <ESOData/World/WorldTerrain.h>
#include <ESOData/Filesystem/Filesystem.h>

#include <ESOData/Serialization/InputSerializationStream.h>
#include <ESOData/Serialization/SizedVector.h>

namespace esodata {
	std::unique_ptr<WorldTerrain> WorldTerrain::readFromFilesystem(const Filesystem& filesystem, uint64_t fileId) {
		std::vector<unsigned char> data;

		if (!filesystem.tryReadFileByKey(fileId, data))
			return nullptr;

		return readFromData(data);
	}

	std::unique_ptr<WorldTerrain> WorldTerrain::readFromData(const std::vector<unsigned char> &data) {
		InputSerializationStream serializer(data.data(), data.data() + data.size());

		auto instance = std::make_unique<WorldTerrain>();
		serializer >> *instance;

		if (instance->magic1 != ExpectedMagic || instance->magic2 != ExpectedMagic) {
			throw std::runtime_error("Invalid WorldTerrain magic");
		}

		instance->entries.reserve(instance->directory.size());

		for (const auto &entry : instance->directory) {
			if (entry.recordLength == 0)
				continue;


			WorldTerrainEntry entryData;

			InputSerializationStream dataSerializer(
				data.data() + entry.recordOffset,
				data.data() + entry.recordOffset + entry.recordLength
			);

			dataSerializer >> entryData;

			instance->entries.emplace(entry.recordType, std::move(entryData));
		}

		return instance;
	}


	SerializationStream &operator <<(SerializationStream &stream, const WorldTerrain &obj) {
		return stream << obj.magic1 << obj.unknown2 << obj.magic2 << makeSizedVector<uint8_t>(obj.directory) << obj.textures << obj.unknown6 << obj.unknown7 << obj.unknown8;
	}

	SerializationStream &operator >>(SerializationStream &stream, WorldTerrain &obj) {
		stream >> obj.magic1;

		if (obj.magic1 != WorldTerrain::ExpectedMagic) {
			throw std::runtime_error("Invalid WorldTerrain magic");
		}

		stream >> obj.unknown2 >> obj.magic2;

		if (obj.magic2 != WorldTerrain::ExpectedMagic) {
			throw std::runtime_error("Invalid WorldTerrain magic");
		}

		return stream >> makeSizedVector<uint8_t>(obj.directory) >> obj.textures >> obj.unknown6 >> obj.unknown7 >> obj.unknown8;
	}

	SerializationStream &operator <<(SerializationStream &serializer, const WorldTerrainEntry &object) {
		serializer << object.magic1 << object.width << object.height << object.stride;

		for (unsigned int row = 0; row < object.height; row++) {
			uint16_t length = object.stride;
			serializer << length;
			serializer.writeData(object.textureData.data() + row * length, length);
		}

		serializer << object.magic2;

		return serializer;
	}

	SerializationStream &operator >>(SerializationStream &serializer, WorldTerrainEntry &object) {
		serializer >> object.magic1;

		if (object.magic1 != WorldTerrainEntry::ExpectedMagic)
			throw std::logic_error("Invalid terrain entry magic");

		serializer >> object.width >> object.height >> object.stride;

		object.textureData.resize(object.stride * object.height);

		for (unsigned int row = 0; row < object.height; row++) {
			uint16_t length;

			serializer >> length;

			if (length > object.stride)
				throw std::runtime_error("Row length exceeds stride");

			serializer.readData(object.textureData.data() + row * length, length);
		}

		serializer >> object.magic2;

		if (object.magic2 != WorldTerrainEntry::ExpectedMagic)
			throw std::logic_error("Invalid terrain entry magic");

		return serializer;
	}

	SerializationStream &operator <<(SerializationStream &stream, const WorldTerrainTexture &obj) {
		return stream << obj.attributes << obj.texture;
	}

	SerializationStream &operator >>(SerializationStream &stream, WorldTerrainTexture &obj) {
		return stream >> obj.attributes >> obj.texture;
	}

	SerializationStream &operator <<(SerializationStream &stream, const WorldTerrainDirectoryEntry &obj) {
		return stream << obj.recordType << obj.recordOffset << obj.recordLength;
	}

	SerializationStream &operator >>(SerializationStream &stream, WorldTerrainDirectoryEntry &obj) {
		return stream >> obj.recordType >> obj.recordOffset >> obj.recordLength;
	}
}
