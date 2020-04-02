#include <ESOData/Depot/ESODepot.h>
#include <ESOData/Depot/IDepotLoadingCallback.h>

#include <fstream>

namespace esodata {
	ESODepot::ESODepot() : m_database(&m_fs) {

	}

	ESODepot::~ESODepot() = default;	

	void ESODepot::loadDirectives(const std::filesystem::path& path, bool includingDatabase) {
		m_supportedVersions.parseFile(path / "SupportedVersions.dir");
		m_filesystem.parseFile(path / "Filesystem.dir");
		m_filenameHarvesting.parseFile(path / "FilenameHarvesting.dir");

		if (includingDatabase) {
			m_database.loadDirectives(path / "Database");
		}
	}

	ValidateDepotResult ESODepot::validateDepot() {
		if (!queryDepotVersion())
			return ValidateDepotResult::DoesNotExist;

		if (std::find(m_supportedVersions.supportedVersions.begin(), m_supportedVersions.supportedVersions.end(), m_depotClientVersion) == m_supportedVersions.supportedVersions.end()) {
			return ValidateDepotResult::UnsupportedVersion;
		}
		else {
			return ValidateDepotResult::Succeeded;
		}
	}

	bool ESODepot::queryDepotVersion() {
		try {
			std::ifstream stream;
			stream.exceptions(std::ios::failbit | std::ios::eofbit | std::ios::badbit);
			stream.open(m_depotPath / "depot" / "_databuild" / "databuild.stamp");

			std::getline(stream, m_depotBuild);
			std::getline(stream, m_depotBuildDate);

			stream.exceptions(std::ios::failbit | std::ios::badbit);
			std::getline(stream, m_depotClientVersion);

			return true;
		}
		catch (const std::exception&) {
			return false;
		}
	}
	
	unsigned int ESODepot::getExpectedNumberOfLoadingSteps() const {
		return static_cast<unsigned int>(m_filesystem.manifests.size() + m_filesystem.fileTables.size() + m_database.defs().size());
	}

	bool ESODepot::load(IDepotLoadingCallback* callback) {
		unsigned int progress = 0;

		if (callback) {
			if (!callback->loadingStepsDone(1))
				return false;
		}

		for (const auto& manifest : m_filesystem.manifests) {
			m_fs.addManifest(m_depotPath / manifest, false);

			++progress;
			if (callback) {
				if (!callback->loadingStepsDone(1))
					return false;
			}
		}

		for (auto fileTable : m_filesystem.fileTables) {
			m_fs.loadFileTable(fileTable);

			++progress;
			if (callback) {
				if (!callback->loadingStepsDone(1))
					return false;
			}
		}
		
		for (auto& def : m_database.defs()) {
			def.loadDef();

			++progress;
			if (callback) {
				if (!callback->loadingStepsDone(1))
					return false;
			}
		}

		return true;
	}
}
