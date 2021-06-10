#include <ESOData/Filesystem/Filesystem.h>

#include <ESOData/World/WorldAddressing.h>
#include <ESOData/World/WorldTableOfContents.h>
#include <ESOData/World/WorldTerrain.h>
#include <ESOData/World/FixtureFile.h>

#include "../ESOData/Oodle/oodle.h"

using namespace esodata;

int main(int argc, char *argv[]) {
	if (!LoadOodleLib()) printf("loading oodle failed\n");

	Filesystem fs;


	fs.addManifest("F:\\Games\\ESO\\The Elder Scrolls Online\\game\\client\\game.mnf", false);
	fs.addManifest("F:\\Games\\ESO\\The Elder Scrolls Online\\depot\\eso.mnf", false);
	fs.addManifest("F:\\Games\\ESO\\The Elder Scrolls Online\\vo_en\\esoaudioen.mnf", false);

	fs.loadFileTable(0x8000000100000000ULL);
	fs.loadFileTable(0x0000000000ffffffULL);

	auto world = 0x1a;
	auto toc = WorldTableOfContents::readFromFilesystem(fs, getWorldTableOfContentsFileID(world));

	printf("world size: %ux%u\n", toc->worldWidth, toc->worldHeight);

	const auto &layers = toc->layers;
	for (auto begin = layers.begin(), it = begin, end = layers.end(); it != end; it++) {
		auto index = static_cast<uint32_t>(it - begin);

		const auto &layer = *it;

		unsigned int cellsX = toc->worldWidth / layer.layerSize;
		if (toc->worldWidth % layer.layerSize)
			cellsX++;

		unsigned int cellsY = toc->worldHeight / layer.layerSize;
		if (toc->worldHeight % layer.layerSize)
			cellsY++;

		printf("Layer %u: size %u name %s extension %s, %ux%u cells\n",
			index,
			layer.layerSize,
			layer.layerName.c_str(),
			layer.layerExtension.c_str(),
			cellsX, cellsY);

		for (unsigned int cellY = 0; cellY < cellsY; cellY++) {
			for (unsigned int cellX = 0; cellX < cellsX; cellX++) {
				auto cellKey = getWorldCellFileID(world, index, cellX, cellY);

				if (layer.layerName == "terrain") {
					auto terrain = WorldTerrain::readFromFilesystem(fs, cellKey);

					if (!terrain) {
						continue;
					}

					printf("  terrain cell %u, %u\n", cellX, cellY);
				}
				else if (layer.layerName == "fixtures") {
					auto fixtures = FixtureFile::readFromFilesystem(fs, cellKey);

					if (!fixtures) {
						continue;
					}

					printf("  fixture cell %u, %u\n", cellX, cellY);

					for (const auto &item : fixtures->placedObjects) {
						printf("    model %08X\n", item.model);
					}
				}
				else {

					std::vector<unsigned char> cellData;
					if (!fs.tryReadFileByKey(cellKey, cellData))
						continue;

					printf("  unk cell %u, %u\n", cellX, cellY);

				}
		
			}
		}
	}

	__debugbreak();
}
