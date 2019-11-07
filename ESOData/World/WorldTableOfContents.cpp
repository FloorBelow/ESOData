#include <ESOData/World/WorldTableOfContents.h>

#include <ESOData/Filesystem/Filesystem.h>

#include <ESOData/Serialization/InputSerializationStream.h>

#include <stdexcept>

namespace esodata {
	std::unique_ptr<WorldTableOfContents> WorldTableOfContents::readFromFilesystem(const Filesystem &filesystem, uint64_t fileId) {
		std::vector<unsigned char> data;
		if (!filesystem.tryReadFileByKey(fileId, data))
			return{};

		InputSerializationStream serializer(data.data(), data.data() + data.size());

		auto instance = std::make_unique<WorldTableOfContents>();
		serializer >> *instance;

		return instance;
	}

	SerializationStream &operator <<(SerializationStream &stream, const WorldTableOfContentsLayer &layer) {
		return stream << layer.layerSize << layer.layerName << layer.layerExtension;
	}

	SerializationStream &operator >>(SerializationStream &stream, WorldTableOfContentsLayer &layer) {
		return stream >> layer.layerSize >> layer.layerName >> layer.layerExtension;
	}

	SerializationStream &operator <<(SerializationStream &stream, const WorldTableOfContents &toc) {
		stream.setSwapEndian(false);
		return stream << toc.version << toc.worldHeight << toc.worldWidth << toc.unknown4 << makeSizedVector<uint8_t>(toc.layers);
	}

	SerializationStream &operator >>(SerializationStream &stream, WorldTableOfContents &toc) {
		stream.setSwapEndian(false);
		stream >> toc.version;

		if (toc.version != WorldTableOfContents::ExpectedVersion)
			throw std::runtime_error("Invalid WorldTableOfContents version");

		return stream >> toc.worldHeight >> toc.worldWidth >> toc.unknown4 >> makeSizedVector<uint8_t>(toc.layers);
	}
}
