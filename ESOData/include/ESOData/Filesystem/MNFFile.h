#ifndef ESODATA_FILESYSTEM_MNF_FILE_H
#define ESODATA_FILESYSTEM_MNF_FILE_H

#include <stdint.h>

#include <ESOData/Serialization/SizedSegment.h>
#include <ESOData/Serialization/HashTable.h>

#include <ESOData/Filesystem/ManifestFileEntry.h>
#include <ESOData/Filesystem/FileSignature.h>

namespace esodata {
	class SerializationStream;

	struct MNFFile;

	struct MNFFileBody {
		mutable const MNFFile *outer;

		FileSignature signature;
		HashTable<uint64_t, ManifestFileEntry> files;
	};

	struct MNFFile {
		enum : uint32_t {
			Signature = 0x3253454D, // Little-endian 'MES2'
		};

		enum : uint32_t {
			DirectorySignaturePresent = 1,
			FileSignaturesPresentPossibility1 = 2,
			FileSignaturesPresentPossibility2 = 4,
		};

		uint16_t version;

		uint8_t dataFileCountOld;
		uint32_t dataFileCountNew;

		size_t dataFileCount() const;
		void setDataFileCount(size_t dataFileCount);

		uint32_t fileFlags;

		SizedSegment<MNFFileBody, ByteswapMode::Enable> body;

		bool hasDirectorySignature() const;
		bool hasFileSignatures() const;

	private:
		bool hasNewDataFileCount() const;

		friend SerializationStream &operator <<(SerializationStream &stream, const MNFFile &file);
		friend SerializationStream &operator >>(SerializationStream &stream, MNFFile &file);
	};

	SerializationStream &operator <<(SerializationStream &stream, const MNFFile &file);
	SerializationStream &operator >>(SerializationStream &stream, MNFFile &file);

	SerializationStream &operator <<(SerializationStream &stream, const MNFFileBody &file);
	SerializationStream &operator >>(SerializationStream &stream, MNFFileBody &file);
}

#endif
