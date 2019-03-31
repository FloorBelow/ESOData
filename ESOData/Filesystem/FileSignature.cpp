#include <ESOData/Filesystem/FileSignature.h>

namespace esodata {

	SerializationStream &operator <<(SerializationStream &stream, const FileSignature &signature) {
		return stream << signature.unknown << signature.publicKey << signature.signature;
	}

	SerializationStream &operator >>(SerializationStream &stream, FileSignature &signature) {
		return stream >> signature.unknown >> signature.publicKey >> signature.signature;
	}
}
