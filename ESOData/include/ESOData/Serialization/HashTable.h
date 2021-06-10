#ifndef ESODATA_SERIALIZATION_HASH_TABLE_H
#define ESODATA_SERIALIZATION_HASH_TABLE_H

#include <ESOData/Serialization/SerializationStream.h>
#include <ESOData/Serialization/DeflatedSegment.h>
#include <ESOData/Serialization/Hash.h>

#include <vector>
#include <algorithm>
#include <stdexcept>

namespace esodata {
	
	template<typename Key, typename Value>
	struct HashTableType3Data {
		std::vector<uint32_t> hashTable;
		std::vector<Key> keys;
		std::vector<Value> values;
	};

	template<typename Key, typename Value>
	SerializationStream &operator <<(SerializationStream &stream, const HashTableType3Data<Key, Value> &data) {
		stream << static_cast<uint32_t>(4);
		stream << static_cast<uint32_t>(data.hashTable.size());
		stream << static_cast<uint32_t>(data.keys.size());
		stream << static_cast<uint32_t>(data.values.size());

		if(!data.hashTable.empty())
			stream << makeDeflatedSegment<std::vector<uint32_t>, ByteswapMode::Disable>(data.hashTable);

		if(!data.keys.empty())
			stream << makeDeflatedSegment<std::vector<Key>, ByteswapMode::Disable>(data.keys);

		if(!data.values.empty())
			stream << makeDeflatedSegment<std::vector<Value>, ByteswapMode::Disable>(data.values);

		return stream;
	}

	template<typename Key, typename Value>
	SerializationStream &operator >>(SerializationStream &stream, HashTableType3Data<Key, Value> &data) {
		uint32_t countLength;
		stream >> countLength;

		if (countLength != 4)
			throw std::logic_error("unexpected countLength in hash table");

		uint32_t hashTableCount;
		uint32_t keyCount;
		uint32_t valueCount;

		stream >> hashTableCount >> keyCount >> valueCount;

		data.hashTable.resize(hashTableCount);

		auto pairCount = std::max(keyCount, valueCount);
		data.keys.resize(pairCount);
		data.values.resize(pairCount);

		if(hashTableCount != 0)
			stream >> makeDeflatedSegment<std::vector<uint32_t>, ByteswapMode::Disable>(data.hashTable);

		if(pairCount != 0)
			stream >> makeDeflatedSegment<std::vector<Key>, ByteswapMode::Disable>(data.keys);

		if(pairCount != 0)
			stream >> makeDeflatedSegment<std::vector<Value>, ByteswapMode::Disable>(data.values);

		return stream;
	}

	template<typename Key, typename Value>
	class HashTable {
	public:
		class Iterator {
		public:
			Iterator(const HashTable<Key, Value> &table, size_t index) : m_table(table), m_index(index) {
				findNextValidIndex();
			}

			bool operator ==(const typename HashTable<Key, Value>::Iterator &other) const {
				return m_index == other.m_index;
			}

			bool operator !=(const typename HashTable<Key, Value>::Iterator &other) const {
				return m_index != other.m_index;
			}

			Iterator operator++(int) {
				Iterator old(*this);

				++(*this);

				return old;
			}

			Iterator &operator++() {
				m_index++;
				findNextValidIndex();

				return *this;
			}

			std::pair<const Key &, const Value &> operator *() const {
				uint32_t entry = m_table.type3Data.hashTable[m_index];
				size_t pairIndex = entry & 0x3FFFFFFFU;

				return std::make_pair<const Key &, const Value &>(m_table.type3Data.keys[pairIndex], m_table.type3Data.values[pairIndex]);
			}

			std::pair<const Key &, Value &> operator *() {
				uint32_t entry = m_table.type3Data.hashTable[m_index];
				size_t pairIndex = entry & 0x3FFFFFFFU;

				return std::pair<const Key &, Value &>(m_table.type3Data.keys[pairIndex], const_cast<HashTable<Key, Value> &>(m_table).type3Data.values[pairIndex]);
			}

		private:
			void findNextValidIndex() {
				while (m_index < m_table.type3Data.hashTable.size()) {
					uint32_t entry = m_table.type3Data.hashTable[m_index];
					uint32_t index = entry & 0x3FFFFFFF;
					if ((entry & 0xC0000000U) == 0x80000000U) {
						break;
					}
					m_index++;
				}
			}

			const HashTable<Key, Value> &m_table;
			size_t m_index;
		};

		Iterator begin() const {
			return Iterator(*this, 0);
		}

		Iterator end() const {
			return Iterator(*this, type3Data.hashTable.size());
		}

		Iterator find(const Key &key) const {
			size_t hashSize = type3Data.hashTable.size();

			if (hashSize == 0)
				return end();

			size_t bucket = hashData64(reinterpret_cast<const unsigned char *>(&key), sizeof(key)) % hashSize;

			for (size_t chain = 0; chain < hashSize; chain++) {
				size_t elementIndex = (bucket + chain) % hashSize;
				uint32_t element = type3Data.hashTable[elementIndex];
				if ((element & 0xC0000000) == 0x00000000)
					break;

				if ((element & 0xC0000000) == 0x80000000) {
					size_t pairIndex = element & 0x3FFFFFFF;
					if (memcmp(&key, &type3Data.keys[pairIndex], sizeof(key)) == 0) {
						return Iterator(*this, elementIndex);
					}
				}
			}


			return end();
		}

	private:		
		HashTableType3Data<Key, Value> type3Data;

		friend SerializationStream &operator >> (SerializationStream &serializer, HashTable<Key, Value> &hashTable) {
			uint16_t tableType;
			serializer >> tableType;

			switch (tableType) {
			case 3:
				serializer >> hashTable.type3Data;
				break;
			default:
				throw std::runtime_error("unsupported hash table type");
			}

			return serializer;
		}

		friend SerializationStream &operator << (SerializationStream &serializer, const HashTable<Key, Value> &hashTable) {
			serializer << (uint16_t)3;
			serializer << hashTable.type3Data;

			return serializer;
		}
	};
}

#endif
