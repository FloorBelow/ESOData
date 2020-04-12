#include <stdio.h>

#include <fstream>

#include "ESODefCompiler.h"

int wmain(int argc, wchar_t** argv) {
	if (argc < 4) {
		fwprintf(stderr, L"Usage: %s <DATABASE DIRECTIVE DIRECTORY> <OUTPUT SOURCE FILE> <OUTPUT HEADER FILE>\n", argv[0]);
		return 1;
	}

	ESODefCompiler compiler(argv[1]);

	std::ofstream sourceFile;
	sourceFile.exceptions(std::ios::failbit | std::ios::eofbit | std::ios::badbit);
	sourceFile.open(argv[2], std::ios::out | std::ios::trunc);

	std::ofstream headerFile;
	headerFile.exceptions(std::ios::failbit | std::ios::eofbit | std::ios::badbit);
	headerFile.open(argv[3], std::ios::out | std::ios::trunc);
	
	compiler.generateSource(headerFile, sourceFile, std::filesystem::proximate(argv[3], std::filesystem::path(argv[2]).parent_path()).generic_u8string());

	return 0;
}