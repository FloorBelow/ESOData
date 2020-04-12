#ifndef ESODATA_DATABASE_DATABASE_MANAGER_H
#define ESODATA_DATABASE_DATABASE_MANAGER_H

#include <memory>
#include <unordered_map>
#include <ESOData/Database/DefFile.h>

namespace esodata {
	class Filesystem;
	class CompiledDef;

	class DatabaseManager {
	public:
		explicit DatabaseManager(const esodata::Filesystem* fs);
		~DatabaseManager();

		DatabaseManager(const DatabaseManager& other) = delete;
		DatabaseManager &operator =(const DatabaseManager& other) = delete;

		static DatabaseManager* instance();

		template<typename DefType>
		typename std::enable_if<std::is_base_of<CompiledDef, DefType>::value, std::shared_ptr<DefType>>::type fetch(uint32_t index) {
			if (index == 0)
				return nullptr;

			std::shared_ptr<CompiledDef> instance = std::make_shared<DefType>();

			return std::static_pointer_cast<DefType>(fetch(DefType::DefIndex, DefType::DefVersion, index, instance));
		}

		void clear();

	private:
		static DatabaseManager* m_instance;

		struct LoadedTable {
			std::unordered_map<uint32_t, uint32_t> lookup;
			std::vector<uint8_t> data;
			DefFileHeader header;
		};

		std::shared_ptr<CompiledDef> fetch(uint32_t index, uint32_t version, uint32_t id, std::shared_ptr<CompiledDef>& instance);
		
		const esodata::Filesystem* m_fs;
		std::unordered_map<uint64_t, std::shared_ptr<CompiledDef>> m_loadedDefs;
		std::unordered_map<uint32_t, LoadedTable> m_loadedTables;
	};
}

#endif
