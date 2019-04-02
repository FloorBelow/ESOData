#include <ESOData/World/WorldAddressing.h>

namespace esodata {
	uint64_t getWorldCellFileID(unsigned int worldID, unsigned int layerID, unsigned int cellX, unsigned int cellY) {
		return 0x4000000000000000ULL |
			(static_cast<uint64_t>(worldID & 0x7FF) << 37) |
			(static_cast<uint64_t>(layerID & 0x1F) << 32) |
			((cellX & 0xFFFF) << 16) |
			(cellY & 0xFFFF);
	}

	uint64_t getWorldTableOfContentsFileID(unsigned int worldID) {
		return 0x4400000000000000ULL | worldID;
	}

	uint64_t getWorldLooseFileID(unsigned int worldID, unsigned int fileID) {
		return 0x4800000000000000ULL |
			(static_cast<uint64_t>(worldID & 0x7FF) << 37) |
			(static_cast<uint64_t>(fileID & 0x1F) << 32);
	}
}
