#ifndef ESODATA_GRANNY2_GRANNY2_TYPE_HELPERS_H
#define ESODATA_GRANNY2_GRANNY2_TYPE_HELPERS_H

#include <memory>

struct granny_file;

namespace esodata {
	struct GrannyFileDeleter {
		void operator()(granny_file* file) const;
	};

	using GrannyFile = std::unique_ptr<granny_file, GrannyFileDeleter>;
}

#endif
