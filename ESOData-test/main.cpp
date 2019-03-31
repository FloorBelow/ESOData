#include <ESOData/Filesystem/Filesystem.h>

int main(int argc, char *argv[]) {
	esodata::Filesystem fs;

	fs.addManifest("C:\\Program Files (x86)\\Zenimax Online\\The Elder Scrolls Online\\game\\client\\game.mnf");
	fs.addManifest("C:\\Program Files (x86)\\Zenimax Online\\The Elder Scrolls Online\\depot\\eso.mnf");
	fs.addManifest("C:\\Program Files (x86)\\Zenimax Online\\The Elder Scrolls Online\\vo_en\\esoaudioen.mnf");

	fs.loadFileTable(0x8000000100000000ULL);

	__debugbreak();
}
