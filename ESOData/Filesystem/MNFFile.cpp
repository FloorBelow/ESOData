#include <ESOData/Filesystem/MNFFile.h>

#include <ESOData/Serialization/SerializationStream.h>

#include <ESOData/Cryptography/CNGKey.h>
#include <ESOData/Cryptography/CNGAlgorithmProvider.h>
#include <ESOData/Cryptography/CNGHash.h>

#include <stdexcept>

namespace esodata {
	bool MNFFile::hasNewDataFileCount() const {
		return version >= 0x0102;
	}

	size_t MNFFile::dataFileCount() const {
		if (hasNewDataFileCount()) {
			return dataFileCountNew;
		}
		else {
			return dataFileCountOld;
		}
	}

	void MNFFile::setDataFileCount(size_t dataFileCount) {
		if (hasNewDataFileCount()) {
			if (dataFileCount > std::numeric_limits<decltype(dataFileCountNew)>::max()) {
				throw std::logic_error("too many data files");
			}

			dataFileCountNew = static_cast<decltype(dataFileCountNew)>(dataFileCount);
		}
		else {
			if (dataFileCount > std::numeric_limits<decltype(dataFileCountOld)>::max()) {
				throw std::logic_error("too many data files");
			}

			dataFileCountOld = static_cast<decltype(dataFileCountOld)>(dataFileCount);
		}
	}

	bool MNFFile::hasDirectorySignature() const {
		return (fileFlags & MNFFile::DirectorySignaturePresent) != 0;
	}

	bool MNFFile::hasFileSignatures() const {
		return (fileFlags & (MNFFile::FileSignaturesPresentPossibility1 | FileSignaturesPresentPossibility2)) != 0;
	}

	SerializationStream &operator <<(SerializationStream &stream, const MNFFile &file) {
		stream << MNFFile::Signature;
		stream << file.version;

		if (file.hasNewDataFileCount()) {
			stream << file.dataFileCountNew;
		}
		else {
			stream << file.dataFileCountOld;
		}
		
		stream << file.fileFlags;

		file.body.data.outer = &file;
		stream << file.body;

		return stream;
	}

	SerializationStream &operator >>(SerializationStream &stream, MNFFile &file) {
		uint32_t signature;

		stream >> signature;
		if (signature != MNFFile::Signature)
			throw std::runtime_error("Bad MNF file signature");

		stream >> file.version;

		if (file.hasNewDataFileCount()) {
			stream >> file.dataFileCountNew;
		}
		else {
			stream >> file.dataFileCountOld;
		}

		stream >> file.fileFlags;

		file.body.data.outer = &file;
		stream >> file.body;

		return stream;
	}

	SerializationStream &operator <<(SerializationStream &stream, const MNFFileBody &file) {
		if (file.outer->hasDirectorySignature()) {
			stream << file.signature;
		}

		stream << file.files;


		return stream;
	}

	SerializationStream &operator >>(SerializationStream &stream, MNFFileBody &file) {
		if (file.outer->hasDirectorySignature()) {
			stream >> file.signature;
		}

		auto beginPos = stream.getCurrentPosition();

		stream >> file.files;

		auto endPos = stream.getCurrentPosition();

		if (file.outer->hasDirectorySignature()) {
			stream.setCurrentPosition(beginPos);

			auto length = endPos - beginPos;
			auto data = stream.getRegionForRead(length);
			/* CryptImportPublicKeyInfoEx2 fails with E_INVALIDARG
			CNGKey key = CNGKey::importDERPublicKey(file.signature.publicKey.data);
			CNGAlgorithmProvider sha1Provider(BCRYPT_SHA1_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
			CNGHash hash(sha1Provider, nullptr, 0, 0);
			hash.hashData(data, length);
			std::vector<uint8_t> digest;
			hash.finish(digest);

			if (!key.verifySignature(nullptr, digest.data(), digest.size(), file.signature.signature.data.data(), file.signature.signature.data.size(), 0)) {
				throw std::runtime_error("Directory signature mismatch");
			}*/
		}

		return stream;
	}
}