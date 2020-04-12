#include <ESOData/Database/DatabaseAddressing.h>

namespace esodata {
	uint64_t getDefFileId(unsigned int index) {
		return 0x6000000000000000ULL | index;
	}

	uint64_t getDefFileIndexId(unsigned int index) {
		return 0x6000000100000000ULL | index;
	}
}
