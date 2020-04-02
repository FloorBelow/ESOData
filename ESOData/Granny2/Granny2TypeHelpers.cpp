#include <ESOdata/Granny2/Granny2TypeHelpers.h>

#include <granny.h>

namespace esodata {
	void GrannyFileDeleter::operator()(granny_file* file) const {
		GrannyFreeFile(file);
	}
}

