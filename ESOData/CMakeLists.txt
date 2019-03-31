set(cryptography_sources
	include/ESOData/Cryptography/CNGAlgorithmProvider.h
	include/ESOData/Cryptography/CNGHash.h
	include/ESOData/Cryptography/CNGKey.h
	include/ESOData/Cryptography/CNGRandom.h
	include/ESOData/Cryptography/CNGSecret.h
	Cryptography/CNGAlgorithmProvider.cpp
	Cryptography/CNGHash.cpp
	Cryptography/CNGKey.cpp
	Cryptography/CNGRandom.cpp
	Cryptography/CNGSecret.cpp
)

set(filesystem_sources
	include/ESOData/Filesystem/Archive.h
	include/ESOData/Filesystem/DataFileHeader.h
	include/ESOData/Filesystem/FileSignature.h
	include/ESOData/Filesystem/Filesystem.h
	include/ESOData/Filesystem/FileTable.h
	include/ESOData/Filesystem/ManifestFileEntry.h
	include/ESOData/Filesystem/MNFFile.h
	Filesystem/Archive.cpp
	Filesystem/DataFileHeader.cpp
	Filesystem/FileSignature.cpp
	Filesystem/Filesystem.cpp
	Filesystem/FileTable.cpp
	Filesystem/ManifestFileEntry.cpp
	Filesystem/MNFFile.cpp
)

set(io_sources
	include/ESOData/IO/IOUtilities.h
	IO/IOUtilities.cpp
)

set(serialization_sources
	include/ESOData/Serialization/DeflatedSegment.h
	include/ESOData/Serialization/Hash.h
	include/ESOData/Serialization/HashTable.h
	include/ESOData/Serialization/InputSerializationStream.h
	include/ESOData/Serialization/OutputSerializationStream.h
	include/ESOData/Serialization/SerializationStream.h
	include/ESOData/Serialization/SizedSegment.h
	include/ESOData/Serialization/SizedVector.h
	Serialization/DeflatedSegment.cpp
	Serialization/Hash.cpp
	Serialization/InputSerializationStream.cpp
	Serialization/OutputSerializationStream.cpp
	Serialization/SerializationStream.cpp
)

add_library(ESOData STATIC 
	${cryptography_sources}
	${filesystem_sources}
	${io_sources}
	${serialization_sources}
)

target_include_directories(ESOData PUBLIC include)
target_link_libraries(ESOData PRIVATE zlib bcrypt crypt32)
target_link_libraries(ESOData PUBLIC archiveparse)
target_compile_definitions(ESOData PRIVATE -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -D_VC_EXTRALEAN -DNOMINMAX)

source_group(Cryptography FILES ${cryptography_sources})
source_group(Filesystem FILES ${filesystem_sources})
source_group(IO FILES ${io_sources})
source_group(Serialization FILES ${serialization_sources})
