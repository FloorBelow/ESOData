#include <ESOData/Filesystem/FileSignature.h>

namespace esodata {

	SerializationStream &operator <<(SerializationStream &stream, const FileSignature &signature) {
		return stream << signature.unknown << makeSizedVector<uint32_t>(signature.publicKey) << makeSizedVector<uint32_t>(signature.signature);
	}

	SerializationStream &operator >>(SerializationStream &stream, FileSignature &signature) {
		return stream >> signature.unknown >> makeSizedVector<uint32_t>(signature.publicKey) >> makeSizedVector<uint32_t>(signature.signature);
	}
}
