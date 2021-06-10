#include <Windows.h>
#include "oodle.h"
#include <stdio.h>
#include <string>
#include <libloaderapi.h>

namespace esodata {

	OodleLZ_Compress_Func*   g_OodleCompressFunc   = nullptr;
	OodleLZ_Decompress_Func* g_OodleDecompressFunc = nullptr;


	bool LoadOodleLib() 
	{
		HINSTANCE mod = LoadLibraryA("oo2core_8_win64.dll");

		if (mod == NULL) printf("Failed to load Oodle DLL!\n");

		g_OodleCompressFunc = (OodleLZ_Compress_Func *) GetProcAddress(mod, "OodleLZ_Compress");
		g_OodleDecompressFunc = (OodleLZ_Decompress_Func *) GetProcAddress(mod, "OodleLZ_Decompress");

		if (!g_OodleCompressFunc || !g_OodleDecompressFunc) printf("Failed to find Oodle compress/decompress functions in DLL!\n");

		return true;
	}

}