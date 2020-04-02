#ifndef ESODATA_DIRECTIVES_SUPPORTED_VERSIONS_DIRECTIVE_FILE_H
#define ESODATA_DIRECTIVES_SUPPORTED_VERSIONS_DIRECTIVE_FILE_H

#include <ESOData/Directives/DirectiveFile.h>

#include <vector>
#include <string>

namespace esodata {
	class SupportedVersionsDirectiveFile final : public DirectiveFile {
	public:
		SupportedVersionsDirectiveFile();
		~SupportedVersionsDirectiveFile();

		std::vector<std::string> supportedVersions;

	protected:
		void processLine(std::vector<std::string>& tokens) override;
	};
}

#endif
