#ifndef WORLD__WORLD_TABLE_OF_CONTENTS__H
#define WORLD__WORLD_TABLE_OF_CONTENTS__H

#include <ESOData/Serialization/SizedVector.h>

#include <string>
#include <memory>

namespace esodata {
	class Filesystem;

	struct WorldTableOfContentsLayer {
		uint16_t layerSize;
		std::string layerName;
		std::string layerExtension;
	};

	SerializationStream &operator <<(SerializationStream &stream, const WorldTableOfContentsLayer &layer);
	SerializationStream &operator >>(SerializationStream &stream, WorldTableOfContentsLayer &layer);

	struct WorldTableOfContents {
		enum : uint32_t {
			ExpectedVersion = 1
		};

		uint32_t version;
		uint32_t worldWidth;
		uint32_t worldHeight;
		uint16_t unknown4;
		std::vector<WorldTableOfContentsLayer> layers;

		static std::unique_ptr<WorldTableOfContents> readFromFilesystem(const Filesystem &filesystem, uint64_t fileId);
	};

	SerializationStream &operator <<(SerializationStream &stream, const WorldTableOfContents &toc);
	SerializationStream &operator >>(SerializationStream &stream, WorldTableOfContents &toc);
}

#endif
