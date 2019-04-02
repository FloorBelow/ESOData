#ifndef SERIALIZATION__WORLD_ADDRESSING__H
#define SERIALIZATION__WORLD_ADDRESSING__H

#include <stdint.h>

namespace esodata {
	uint64_t getWorldCellFileID(unsigned int worldID, unsigned int layerID, unsigned int cellX, unsigned int cellY);
	uint64_t getWorldTableOfContentsFileID(unsigned int worldID);
	uint64_t getWorldLooseFileID(unsigned int worldID, unsigned int fileID);
}

#endif
