#ifndef ESODATA_WORLD_FIXTURE_FILE_H
#define ESODATA_WORLD_FIXTURE_FILE_H

#include <ESOData/Serialization/SerializationStream.h>

#include <vector>
#include <memory>

namespace esodata {
	class Filesystem;

	struct FixtureFileVector {
		float x;
		float y;
		float z;
	};
	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileVector &obj);
	SerializationStream &operator >>(SerializationStream &stream, FixtureFileVector &obj);

	struct FixtureFileBaseObject {
		enum uint32_t {
			FlagInItemGroup = (1U << 0),
			FlagUnknown = (1U << 16)
		};


		uint64_t fixtureID;
		uint32_t flags;
		uint32_t itemGroupID;
		FixtureFileVector rotation;
		FixtureFileVector translation;
		uint32_t worldOffsetX;
		uint32_t unknown12;
		uint32_t worldOffsetY;
	};
	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileBaseObject &obj);
	SerializationStream &operator >>(SerializationStream &stream, FixtureFileBaseObject &obj);

	struct FixtureFilePlacedObject : public FixtureFileBaseObject {

		uint32_t unknown14;
		uint32_t room; // Room ID
		uint32_t unknown16;
		uint32_t unknown17;
		uint32_t model; // Asset reference
		uint32_t clickable; // Clickable ID, unconfirmed
		uint32_t unknown20;
		uint32_t unknown21;
		uint32_t unknown22;
		uint32_t unknown23;
		uint32_t unknown24;
	};
	SerializationStream &operator <<(SerializationStream &stream, const FixtureFilePlacedObject &obj);
	SerializationStream &operator >>(SerializationStream &stream, FixtureFilePlacedObject &obj);


	struct FixtureFileLightSource : public FixtureFileBaseObject {

		uint32_t unknown14;
		uint32_t unknown15;
		float unknown16;
		float unknown17;
		float unknown18;
		float unknown19;
		float unknown20;
		float unknown21;
		float unknown22;
		float unknown23;
		float unknown24;
		float unknown25;			   
		float unknown26;
		float unknown27;
		float unknown28;
		float unknown29;
		uint32_t unknown30;
		uint32_t unknown31;
		float unknown32;
		float unknown33;
		uint32_t texture1; // Asset ID
		uint32_t unknown35;
		uint32_t unknown36;
		float unknown37;
		float unknown38;
		uint32_t texture2; // Asset ID
		uint32_t unknown40;
		uint32_t unknown41;
		uint32_t unknown42;
		uint32_t unknown43;
		float unknown44;
		float unknown45;
		float unknown46;
		float unknown47;
		float unknown48;
		float unknown49;
		float unknown50;
		float unknown51;
		float unknown52;
		float unknown53;
		float unknown54;
		float unknown55;
		float unknown56;
		float unknown57;
		float unknown58;
		float unknown59;
		float unknown60;
		float unknown61;
		float unknown62;
		float unknown63;
		float unknown64;
		float unknown65;
		float unknown66;
		float unknown67;
		float unknown68;
		float unknown69;
		float unknown70;
		float unknown71;
		float unknown72;
		float unknown73;
		float unknown74;
		float unknown75;
		uint32_t unknown76;
		uint32_t unknown77;
		uint32_t unknown78;
	};
	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileLightSource &obj);
	SerializationStream &operator >>(SerializationStream &stream, FixtureFileLightSource &obj);

	struct FixtureFileUnknownObject : public FixtureFileBaseObject {

		uint32_t unknown14;
		uint32_t unknown15;
		float unknown16;
		uint32_t unknown17;
		float unknown18;
		float unknown19;
		float unknown20;
		uint32_t unknown21;
		uint32_t unknown22;
	};
	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileUnknownObject &obj);
	SerializationStream &operator >>(SerializationStream &stream, FixtureFileUnknownObject &obj);


	struct FixtureFileObjectGroup {

		uint32_t groupId;
		std::vector<uint64_t> furnitureID;
	};
	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileObjectGroup &obj);
	SerializationStream &operator >>(SerializationStream &stream, FixtureFileObjectGroup &obj);

	struct  FixtureFileRTreeBoundingBox {

		FixtureFileVector min;
		FixtureFileVector max;

	};
	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileRTreeBoundingBox &obj);
	SerializationStream &operator >>(SerializationStream &stream, FixtureFileRTreeBoundingBox &obj);

	struct FixtureFileRTreeNodeChild;
	struct FixtureFileRTreeItemChild;

	struct  FixtureFileRTreeNode {

		uint32_t nodeLevelsBelow;

		inline bool isLeafNode() const {
			return nodeLevelsBelow == 0;
		}

		//SERIALIZED(condition = !object.isLeafNode())
		std::vector<FixtureFileRTreeNodeChild> nodeChildren;

		//SERIALIZED(condition = object.isLeafNode())
		std::vector<FixtureFileRTreeItemChild> itemChildren;

	};
	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileRTreeNode &obj);
	SerializationStream &operator >>(SerializationStream &stream, FixtureFileRTreeNode &obj);

	struct  FixtureFileRTreeBaseChild {
		FixtureFileRTreeBoundingBox boundingBox;
	};
	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileRTreeBaseChild &obj);
	SerializationStream &operator >>(SerializationStream &stream, FixtureFileRTreeBaseChild &obj);

	struct  FixtureFileRTreeNodeChild : public FixtureFileRTreeBaseChild {
		FixtureFileRTreeNode node;
	};
	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileRTreeNodeChild &obj);
	SerializationStream &operator >>(SerializationStream &stream, FixtureFileRTreeNodeChild &obj);

	struct  FixtureFileRTreeItemChild : public FixtureFileRTreeBaseChild {
		uint32_t itemIndex;
	};

	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileRTreeItemChild &obj);
	SerializationStream &operator >>(SerializationStream &stream, FixtureFileRTreeItemChild &obj);

	struct FixtureFileRTree {
		enum uint32_t {
			ExpectedSignature = 0x45525452,
			ExpectedUnknown2 = 4,
			ExpectedUnknown3 = 3,
			ExpectedUnknown4 = 4,
			ExpectedUnknown5 = 4,
			ExpectedUnknown6 = 8,
			ExpectedUnknown7 = 4,
		};

		uint32_t signature;
		uint32_t unknown2;
		uint32_t unknown3;
		uint32_t unknown4;
		uint32_t unknown5;
		uint32_t unknown6;
		uint32_t unknown7;

		FixtureFileRTreeNode rootNode;
	};

	SerializationStream &operator <<(SerializationStream &stream, const FixtureFileRTree &obj);
	SerializationStream &operator >>(SerializationStream &stream, FixtureFileRTree &obj);

	struct FixtureFile {
		enum uint32_t {
			ExpectedVersion = 23
		};

		uint32_t version;
		std::vector<FixtureFilePlacedObject> placedObjects;
		std::vector<FixtureFileLightSource> lightSources;
		std::vector<FixtureFileUnknownObject> unknownObjects;
		std::vector<FixtureFileObjectGroup> objectGroups;
		FixtureFileRTree unknown5;
		FixtureFileRTree unknown6;
		FixtureFileRTree unknown7;
		FixtureFileRTree unknown8;

		static std::unique_ptr<FixtureFile> readFromFilesystem(const Filesystem &filesystem, uint64_t fileId);

	};

	SerializationStream &operator <<(SerializationStream &stream, const FixtureFile &obj);
	SerializationStream &operator >>(SerializationStream &stream, FixtureFile &obj);
}

#endif

