#ifndef ESODATA_DEPOT_ESODEPOT_H
#define ESODATA_DEPOT_ESODEPOT_H

#include <filesystem>

#include <ESOData/Directives/SupportedVersionsDirectiveFile.h>
#include <ESOData/Directives/FilesystemDirectiveFile.h>
#include <ESOData/Directives/FilenameHarvestingDirectiveFile.h>

#include <ESOData/Filesystem/Filesystem.h>

#include <ESOData/Database/ESODatabase.h>
#include <ESOData/Database/DatabaseManager.h>

namespace esodata {
	enum class ValidateDepotResult {
		Succeeded,
		DoesNotExist,
		UnsupportedVersion
	};

	class IDepotLoadingCallback;

	class ESODepot {
	public:
		ESODepot();
		~ESODepot();

		ESODepot(const ESODepot& other) = delete;
		ESODepot &operator =(const ESODepot& other) = delete;

		void loadDirectives(const std::filesystem::path& path, bool includingDatabase = false);

		inline const std::vector<std::string>& supportedVersions() const {
			return m_supportedVersions.supportedVersions;
		}

		inline const std::vector<std::string>& prefixesForFilenameHarvesting() const {
			return m_filenameHarvesting.prefixes;
		}

		inline const std::filesystem::path& depotPath() const {
			return m_depotPath;
		}

		inline void setDepotPath(const std::filesystem::path& depotPath) {
			m_depotPath = depotPath;
		}

		inline void setDepotPath(std::filesystem::path&& depotPath) {
			m_depotPath = std::move(depotPath);
		}

		inline const std::string& depotBuild() const {
			return m_depotBuild;
		}

		inline const std::string& depotBuildDate() const {
			return m_depotBuildDate;
		}

		inline const std::string& depotClientVersion() const {
			return m_depotClientVersion;
		}

		inline const Filesystem* filesystem() const {
			return &m_fs;
		}

		inline const ESODatabase* database() const {
			return &m_database;
		}

		esodata::ValidateDepotResult validateDepot();

		unsigned int getExpectedNumberOfLoadingSteps() const;
		bool load(IDepotLoadingCallback* callback = nullptr);

	private:
		bool queryDepotVersion();

		SupportedVersionsDirectiveFile m_supportedVersions;
		FilesystemDirectiveFile m_filesystem;
		FilenameHarvestingDirectiveFile m_filenameHarvesting;

		std::filesystem::path m_depotPath;
		std::string m_depotBuild;
		std::string m_depotBuildDate;
		std::string m_depotClientVersion;

		Filesystem m_fs;
		ESODatabase m_database;
		DatabaseManager m_dbManager;
	};
}

#endif
