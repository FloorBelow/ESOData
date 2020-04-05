#ifndef WORLD__WORLD_TERRAIN
#define WORLD__WORLD_TERRAIN

#include <array>
#include <memory>
#include <unordered_map>

namespace esodata {

	class Filesystem;
	class SerializationStream;

	struct WorldTerrainDirectoryEntry {
		uint8_t recordType;
		uint32_t recordOffset;
		uint32_t recordLength;
	};

	SerializationStream &operator <<(SerializationStream &stream, const WorldTerrainDirectoryEntry &obj);
	SerializationStream &operator >>(SerializationStream &stream, WorldTerrainDirectoryEntry &obj);

	struct WorldTerrainEntry {
		enum : uint32_t {
			ExpectedMagic = 0xBEEF0001
		};

		uint32_t magic1;
		uint32_t width;
		uint32_t height;
		uint32_t stride;
		std::vector<uint8_t> textureData;
		uint32_t magic2;
	};

	SerializationStream &operator <<(SerializationStream &stream, const WorldTerrainEntry &obj);
	SerializationStream &operator >>(SerializationStream &stream, WorldTerrainEntry &obj);


	struct WorldTerrainTexture {
		uint16_t attributes;
		uint16_t texture; // TerrainTexture
	};

	SerializationStream &operator <<(SerializationStream &stream, const WorldTerrainTexture &obj);
	SerializationStream &operator >>(SerializationStream &stream,  WorldTerrainTexture &obj);

	struct WorldTerrain {
		enum : uint32_t {
			ExpectedMagic = 0xBEEF000C
		};

		uint32_t magic1;
		bool unknown2;
		uint32_t magic2;
		std::vector<WorldTerrainDirectoryEntry> directory; // ByteLength
		std::array<WorldTerrainTexture, 16> textures;
		float unknown6;
		float unknown7;
		float unknown8;

		std::unordered_map<uint8_t, WorldTerrainEntry> entries;

		static std::unique_ptr<WorldTerrain> readFromFilesystem(const Filesystem &filesystem, uint64_t fileId);
		static std::unique_ptr<WorldTerrain> readFromData(const std::vector<unsigned char>& data);
	};

	SerializationStream &operator <<(SerializationStream &stream, const WorldTerrain &obj);
	SerializationStream &operator >>(SerializationStream &stream, WorldTerrain &obj);
}

#endif 

