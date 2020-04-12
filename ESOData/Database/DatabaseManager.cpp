#include <ESOData/Database/DatabaseManager.h>
#include <ESOData/Database/DefFileIndex.h>
#include <ESOData/Database/DatabaseAddressing.h>
#include <ESOData/Database/CompiledDef.h>

#include <ESOData/Filesystem/Filesystem.h>

#include <ESOData/Serialization/InputSerializationStream.h>

#include <stdexcept>
#include <sstream>

namespace esodata {
    DatabaseManager* DatabaseManager::m_instance;

    DatabaseManager::DatabaseManager(const esodata::Filesystem* fs) : m_fs(fs) {
        if (m_instance != nullptr)
            throw std::runtime_error("Only one instance of DatabaseManager may be created");

        m_instance = this;
    }

    DatabaseManager::~DatabaseManager() {
        m_instance = nullptr;
    }

    DatabaseManager* DatabaseManager::instance() {
        if (m_instance == nullptr)
            throw std::runtime_error("DatabaseManager instance must be created first");

        return m_instance;
    }

    void DatabaseManager::clear() {
        m_loadedDefs.clear();
    }

    std::shared_ptr<CompiledDef> DatabaseManager::fetch(uint32_t index, uint32_t version, uint32_t id, std::shared_ptr<CompiledDef>& instance) {
        auto key = (static_cast<uint64_t>(index) << 32) | id;

        if (id == 0)
            return nullptr;

        auto existing = m_loadedDefs.find(key);
        if (existing != m_loadedDefs.end()) {
            return existing->second;
        }

        auto loadedTablePair = m_loadedTables.emplace(index, LoadedTable{});
        auto& loadedTable = loadedTablePair.first->second;

        if (loadedTablePair.second) {
            auto defIndex = DefFileIndex::readFromFilesystem(*m_fs, getDefFileIndexId(index));

            loadedTable.lookup.reserve(defIndex->lookupRecords.size());
            for (const auto& record : defIndex->lookupRecords) {
                loadedTable.lookup.emplace(record.index, record.offset);
            }

            size_t offset = 0;
            loadedTable.data = m_fs->readFileByKey(getDefFileId(index));
            loadedTable.header.readFromData(loadedTable.data, offset);

            if (loadedTable.header.flags != 0x13)
                throw std::runtime_error("this is not a client depot");
        }
  
        if (version != loadedTable.header.version) {
            std::stringstream error;
            error << "def " << index << " has unsupported version: expected " << version << ", got " << loadedTable.header.version;
            throw std::runtime_error(error.str());
        }

        auto entry = loadedTable.lookup.find(id);
        if (entry == loadedTable.lookup.end()) {
            instance = nullptr;
        }
        else {
            size_t offset = entry->second;

            esodata::DefFileRow row;
            row.readFromData(loadedTable.data, offset);

            esodata::InputSerializationStream contentStream(row.recordData.data(), row.recordData.data() + row.recordData.size());
            contentStream.setSwapEndian(true);

            instance->deserialize(contentStream);
        }

        m_loadedDefs.emplace(index, instance);

        return instance;
    }

}
