#ifndef ESODATA_DIRECTIVES_FILESYSTEM_DIRECTIVE_FILE_H
#define ESODATA_DIRECTIVES_FILESYSTEM_DIRECTIVE_FILE_H

#include <ESOData/Directives/DirectiveFile.h>

#include <string>

namespace esodata {
	class FilesystemDirectiveFile final : public DirectiveFile {
	public:
		FilesystemDirectiveFile();
		~FilesystemDirectiveFile();

		std::vector<std::string> manifests;
		std::vector<uint64_t> fileTables;

	private:
		void processLine(std::vector<std::string>& tokens) override;
	};
}

#endif
