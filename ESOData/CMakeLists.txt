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

set(database_sources
	include/ESOData/Database/AssetReference.h
	include/ESOData/Database/CompiledDef.h
	include/ESOData/Database/DatabaseAddressing.h
	include/ESOData/Database/DatabaseManager.h
	include/ESOData/Database/DefFile.h
	include/ESOData/Database/DefFileIndex.h
	include/ESOData/Database/ESODatabase.h
	include/ESOData/Database/ESODatabaseDef.h
	include/ESOData/Database/ESODatabaseParsingContext.h
	include/ESOData/Database/ESODatabaseRecord.h
	include/ESOData/Database/ForeignKey.h
	include/ESOData/Database/PolymorphicReference.h
	Database/AssetReference.cpp
	Database/CompiledDef.cpp
	Database/DatabaseAddressing.cpp
	Database/DatabaseManager.cpp
	Database/DefFile.cpp
	Database/DefFileIndex.cpp
	Database/ESODatabase.cpp
	Database/ESODatabaseDef.cpp
	Database/ESODatabaseParsingContext.cpp
	Database/ESODatabaseRecord.cpp
)

set(depot_sources
	include/ESOData/Depot/ESODepot.h
	include/ESOData/Depot/IDepotLoadingCallback.h
	Depot/ESODepot.cpp
)

set(directives_sources
	include/ESOData/Directives/DatabaseDirectiveFile.h
	include/ESOData/Directives/DirectiveFile.h
	include/ESOData/Directives/FilenameHarvestingDirectiveFile.h
	include/ESOData/Directives/FilesystemDirectiveFile.h
	include/ESOData/Directives/SupportedVersionsDirectiveFile.h
	Directives/DatabaseDirectiveFile.cpp
	Directives/DirectiveFile.cpp
	Directives/FilenameHarvestingDirectiveFile.cpp
	Directives/FilesystemDirectiveFile.cpp
	Directives/SupportedVersionsDirectiveFile.cpp
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

set(granny2_sources
	include/ESOData/Granny2/ESOGraphicsTypes.h
	include/ESOData/Granny2/Granny2TypeHelpers.h
	Granny2/ESOGraphicsTypes.cpp
	Granny2/Granny2TypeHelpers.cpp
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

set(world_sources
	include/ESOData/World/FixtureFile.h
	include/ESOData/World/WorldAddressing.h
	include/ESOData/World/WorldTableOfContents.h
	include/ESOData/World/WorldTerrain.h
	World/FixtureFile.cpp
	World/WorldAddressing.cpp
	World/WorldTableOfContents.cpp
	World/WorldTerrain.cpp
)

add_library(ESOData STATIC 
	${cryptography_sources}
	${database_sources}
	${depot_sources}
	${directives_sources}
	${filesystem_sources}
	${granny2_sources}
	${io_sources}
	${serialization_sources}
	${world_sources}
	Oodle/oodle.h
	Oodle/oodle.cpp
)

target_include_directories(ESOData PUBLIC include)
target_link_libraries(ESOData PRIVATE zlib bcrypt crypt32 snappy granny)
target_link_libraries(ESOData PUBLIC archiveparse)
target_compile_definitions(ESOData PRIVATE -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -D_VC_EXTRALEAN -DNOMINMAX)

source_group(Cryptography FILES ${cryptography_sources})
source_group(Database FILES ${database_sources})
source_group(Depot FILES ${depot_sources})
source_group(Directives FILES ${directives_sources})
source_group(Filesystem FILES ${filesystem_sources})
source_group(Granny2 FILES ${granny2_sources})
source_group(IO FILES ${io_sources})
source_group(Serialization FILES ${serialization_sources})
source_group(World FILES ${world_sources})

set_target_properties(ESOData PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED TRUE
)
