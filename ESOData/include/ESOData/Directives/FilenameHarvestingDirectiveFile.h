#ifndef ESODATA_FILENAME_HARVESTING_DIRECTIVE_FILE_H
#define ESODATA_FILENAME_HARVESTING_DIRECTIVE_FILE_H

#include <ESOData/Directives/DirectiveFile.h>

namespace esodata {
	class FilenameHarvestingDirectiveFile final : public DirectiveFile {
	public:
		FilenameHarvestingDirectiveFile();
		~FilenameHarvestingDirectiveFile();

		std::vector<std::string> prefixes;

	protected:
		void processLine(std::vector<std::string>& tokens) override;
	};
}

#endif
