#include <ESOData/World/FixtureFile.h>

#include <ESOData/Filesystem/Filesystem.h>

#include <ESOData/Serialization/InputSerializationStream.h>
#include <ESOData/Serialization/SizedVector.h>

#include <stdexcept>

namespace esodata {

	std::unique_ptr<FixtureFile> FixtureFile::readFromFilesystem(const Filesystem &filesystem, uint64_t fileId) {
		std::vector<unsigned char> data;

		if (!filesystem.tryReadFileByKey(fileId, data))
			return nullptr;

		InputSerializationStream serializer(data.data(), data.data() + data.size());

		auto instance = std::make_unique<FixtureFile>();
		serializer >> *instance;

		return instance;
	}

	SerializationStream &operator <<(SerializationStream &stream, const FixtureFile &obj) {
		return stream << obj.version
			<< makeSizedVector<uint32_t>(obj.placedObjects)
			<< makeSizedVector<uint32_t>(obj.lightSources)
			<< makeSizedVector<uint32_t>(obj.unknownObjects)
			<< makeSizedVector<uint32_t>(obj.objectGroups)
			<< obj.unknown5
			<< obj.unknown6
			<< obj.unknown7
			<< obj.unknown8;
	}

	SerializationStream &operator >>(SerializationStream &stream, FixtureFile &obj) {
		stream >> obj.version;
		if(obj.version != FixtureFile::ExpectedVersion)
			throw std::runtime_error("Invalid FixtureFile version");

		return stream
			>> makeSizedVector<uint32_t>(obj.placedObjects)
			>> makeSizedVector<uint32_t>(obj.lightSources)
			>> makeSizedVector<uint32_t>(obj.unknownObjects)
			>> makeSizedVector<uint32_t>(obj.objectGroups)
			>> obj.unknown5
			>> obj.unknown6
			>> obj.unknown7
			>> obj.unknown8;
	}

	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileRTree &obj) {
		return stream
			<< obj.signature
			<< obj.unknown2
			<< obj.unknown3
			<< obj.unknown4
			<< obj.unknown5
			<< obj.unknown6
			<< obj.unknown7
			<< obj.rootNode;
	}

	SerializationStream &operator >>(SerializationStream &stream, FixtureFileRTree &obj) {
		stream >> obj.signature;
		if (obj.signature != FixtureFileRTree::ExpectedSignature)
			throw std::runtime_error("bad R-Tree signature");

		stream
			>> obj.unknown2
			>> obj.unknown3
			>> obj.unknown4
			>> obj.unknown5
			>> obj.unknown6
			>> obj.unknown7;

		if(obj.unknown2 != FixtureFileRTree::ExpectedUnknown2 ||
			obj.unknown3 != FixtureFileRTree::ExpectedUnknown3 ||
			obj.unknown4 != FixtureFileRTree::ExpectedUnknown4 ||
			obj.unknown5 != FixtureFileRTree::ExpectedUnknown5 ||
			obj.unknown6 != FixtureFileRTree::ExpectedUnknown6 ||
			obj.unknown7 != FixtureFileRTree::ExpectedUnknown7)
			throw std::runtime_error("bad R-Tree dimensions");

		return stream >> obj.rootNode;
	}

	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileRTreeItemChild &obj) {
		return stream << static_cast<const FixtureFileRTreeBaseChild &>(obj) << obj.itemIndex;
	}

	SerializationStream &operator >>(SerializationStream &stream, FixtureFileRTreeItemChild &obj) {
		return stream >> static_cast<FixtureFileRTreeBaseChild &>(obj) >> obj.itemIndex;
	}

	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileRTreeNodeChild &obj) {
		return stream << static_cast<const FixtureFileRTreeBaseChild &>(obj) << obj.node;
	}

	SerializationStream &operator >>(SerializationStream &stream, FixtureFileRTreeNodeChild &obj) {
		return stream >> static_cast<FixtureFileRTreeBaseChild &>(obj) >> obj.node;
	}

	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileRTreeBaseChild &obj) {
		return stream << obj.boundingBox;
	}

	SerializationStream &operator >>(SerializationStream &stream, FixtureFileRTreeBaseChild &obj) {
		return stream >> obj.boundingBox;
	}

	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileRTreeNode &obj) {
		stream << obj.nodeLevelsBelow;

		if (obj.isLeafNode()) {
			stream << makeSizedVector<uint32_t>(obj.itemChildren);
		}
		else {
			stream << makeSizedVector<uint32_t>(obj.nodeChildren);
		}

		return stream;
	}

	SerializationStream &operator >>(SerializationStream &stream, FixtureFileRTreeNode &obj) {
		stream >> obj.nodeLevelsBelow;

		if (obj.isLeafNode()) {
			stream >> makeSizedVector<uint32_t>(obj.itemChildren);
		}
		else {
			stream >> makeSizedVector<uint32_t>(obj.nodeChildren);
		}

		return stream;
	}

	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileRTreeBoundingBox &obj) {
		return stream << obj.min << obj.max;
	}

	SerializationStream &operator >>(SerializationStream &stream, FixtureFileRTreeBoundingBox &obj) {
		return stream >> obj.min >> obj.max;
	}

	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileObjectGroup &obj) {
		return stream <<
			obj.groupId <<
			makeSizedVector<uint32_t>(obj.furnitureID);
	}

	SerializationStream &operator >>(SerializationStream &stream, FixtureFileObjectGroup &obj) {
		return stream >>
			obj.groupId >>
			makeSizedVector<uint32_t>(obj.furnitureID);
	}

	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileUnknownObject &obj) {
		return stream
			<< static_cast<const FixtureFileBaseObject &>(obj)
			<< obj.unknown14
			<< obj.unknown15
			<< obj.unknown16
			<< obj.unknown17
			<< obj.unknown18
			<< obj.unknown19
			<< obj.unknown20
			<< obj.unknown21
			<< obj.unknown22;
	}

	SerializationStream &operator >>(SerializationStream &stream, FixtureFileUnknownObject &obj) {
		return stream
			>> static_cast<FixtureFileBaseObject &>(obj)
			>> obj.unknown14
			>> obj.unknown15
			>> obj.unknown16
			>> obj.unknown17
			>> obj.unknown18
			>> obj.unknown19
			>> obj.unknown20
			>> obj.unknown21
			>> obj.unknown22;
	}

	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileLightSource &obj) {
		return stream
			<< static_cast<const FixtureFileBaseObject &>(obj)
			<< obj.unknown14
			<< obj.unknown15
			<< obj.unknown16
			<< obj.unknown17
			<< obj.unknown18
			<< obj.unknown19
			<< obj.unknown20
			<< obj.unknown21
			<< obj.unknown22
			<< obj.unknown23
			<< obj.unknown24
			<< obj.unknown25
			<< obj.unknown26
			<< obj.unknown27
			<< obj.unknown28
			<< obj.unknown29
			<< obj.unknown30
			<< obj.unknown31
			<< obj.unknown32
			<< obj.unknown33
			<< obj.texture1
			<< obj.unknown35
			<< obj.unknown36
			<< obj.unknown37
			<< obj.unknown38
			<< obj.texture2
			<< obj.unknown40
			<< obj.unknown41
			<< obj.unknown42
			<< obj.unknown43
			<< obj.unknown44
			<< obj.unknown45
			<< obj.unknown46
			<< obj.unknown47
			<< obj.unknown48
			<< obj.unknown49
			<< obj.unknown50
			<< obj.unknown51
			<< obj.unknown52
			<< obj.unknown53
			<< obj.unknown54
			<< obj.unknown55
			<< obj.unknown56
			<< obj.unknown57
			<< obj.unknown58
			<< obj.unknown59
			<< obj.unknown60
			<< obj.unknown61
			<< obj.unknown62
			<< obj.unknown63
			<< obj.unknown64
			<< obj.unknown65
			<< obj.unknown66
			<< obj.unknown67
			<< obj.unknown68
			<< obj.unknown69
			<< obj.unknown70
			<< obj.unknown71
			<< obj.unknown72
			<< obj.unknown73
			<< obj.unknown74
			<< obj.unknown75
			<< obj.unknown76
			<< obj.unknown77
			<< obj.unknown78;
	}

	SerializationStream &operator >>(SerializationStream &stream, FixtureFileLightSource &obj) {
		return stream
			>> static_cast<FixtureFileBaseObject &>(obj)
			>> obj.unknown14
			>> obj.unknown15
			>> obj.unknown16
			>> obj.unknown17
			>> obj.unknown18
			>> obj.unknown19
			>> obj.unknown20
			>> obj.unknown21
			>> obj.unknown22
			>> obj.unknown23
			>> obj.unknown24
			>> obj.unknown25
			>> obj.unknown26
			>> obj.unknown27
			>> obj.unknown28
			>> obj.unknown29
			>> obj.unknown30
			>> obj.unknown31
			>> obj.unknown32
			>> obj.unknown33
			>> obj.texture1
			>> obj.unknown35
			>> obj.unknown36
			>> obj.unknown37
			>> obj.unknown38
			>> obj.texture2
			>> obj.unknown40
			>> obj.unknown41
			>> obj.unknown42
			>> obj.unknown43
			>> obj.unknown44
			>> obj.unknown45
			>> obj.unknown46
			>> obj.unknown47
			>> obj.unknown48
			>> obj.unknown49
			>> obj.unknown50
			>> obj.unknown51
			>> obj.unknown52
			>> obj.unknown53
			>> obj.unknown54
			>> obj.unknown55
			>> obj.unknown56
			>> obj.unknown57
			>> obj.unknown58
			>> obj.unknown59
			>> obj.unknown60
			>> obj.unknown61
			>> obj.unknown62
			>> obj.unknown63
			>> obj.unknown64
			>> obj.unknown65
			>> obj.unknown66
			>> obj.unknown67
			>> obj.unknown68
			>> obj.unknown69
			>> obj.unknown70
			>> obj.unknown71
			>> obj.unknown72
			>> obj.unknown73
			>> obj.unknown74
			>> obj.unknown75
			>> obj.unknown76
			>> obj.unknown77
			>> obj.unknown78;
	}

	SerializationStream &operator <<(SerializationStream &stream, const FixtureFilePlacedObject &obj) {
		return stream
			<< static_cast<const FixtureFileBaseObject &>(obj)
			<< obj.unknown14
			<< obj.room
			<< obj.unknown16
			<< obj.unknown17
			<< obj.model
			<< obj.clickable
			<< obj.unknown20
			<< obj.unknown21
			<< obj.unknown22
			<< obj.unknown23
			<< obj.unknown24;
	}

	SerializationStream &operator >>(SerializationStream &stream, FixtureFilePlacedObject &obj) {
		return stream
			>> static_cast<FixtureFileBaseObject &>(obj)
			>> obj.unknown14
			>> obj.room
			>> obj.unknown16
			>> obj.unknown17
			>> obj.model
			>> obj.clickable
			>> obj.unknown20
			>> obj.unknown21
			>> obj.unknown22
			>> obj.unknown23
			>> obj.unknown24;
	}

	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileBaseObject &obj) {
		return stream
			<< obj.fixtureID
			<< obj.flags
			<< obj.itemGroupID
			<< obj.rotation
			<< obj.translation
			<< obj.worldOffsetX
			<< obj.unknown12
			<< obj.worldOffsetY;
	}

	SerializationStream &operator >>(SerializationStream &stream, FixtureFileBaseObject &obj) {
		return stream
			>> obj.fixtureID
			>> obj.flags
			>> obj.itemGroupID
			>> obj.rotation
			>> obj.translation
			>> obj.worldOffsetX
			>> obj.unknown12
			>> obj.worldOffsetY;
	}

	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileVector &obj) {
		return stream << obj.x << obj.y << obj.z;
	}

	SerializationStream &operator >>(SerializationStream &stream, FixtureFileVector &obj) {
		return stream >> obj.x >> obj.y >> obj.z;
	}

}
