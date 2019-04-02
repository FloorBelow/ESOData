#include "ProjectedFilesystem.h"

#include <archiveparse/EncodingUtilities.h>
#include <archiveparse/WindowsError.h>

#include <objbase.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>

#include <ESOData/Serialization/Hash.h>

size_t std::hash<GUID>::operator()(const GUID &guid) const {
	return static_cast<size_t>(esodata::hashData64(reinterpret_cast<const unsigned char *>(&guid), sizeof(guid)));
}

bool ProjectedFilesystem::parseByIDName(const std::wstring &initialPath, uint64_t &firstID, uint64_t &lastID) {
	std::wstring path(initialPath);
	while (!path.empty() && path.back() == L'\\')
		path.erase(path.size() - 1, 1);

	size_t namePos = 0;

	std::wstring thisName;

	int position = -2;

	firstID = 0;
	lastID = ~firstID;

	while (true) {
		size_t nextPart = path.find(L'\\', namePos);

		if (nextPart == std::string::npos) {
			thisName = path.substr(namePos);
		}
		else {
			thisName = path.substr(namePos, nextPart - namePos);
		}

		if (position == -2) {
			if (!thisName.empty())
				return false;
		}
		else if (position == -1) {
			if (PrjFileNameCompare(thisName.c_str(), L"by-id") != 0)
				return false;
		}
		else {
			if (position == 8)
				return false;

			static const std::wregex nameRegex(L"^[0-9a-fA-F]{2}$");
			if (!std::regex_match(thisName, nameRegex))
				return false;

			auto byte = std::stoul(thisName, nullptr, 16);

			uint64_t mask;
			if (position == 7)
				mask = 0;
			else
				mask = ~0ULL >> ((position + 1) * 8);

			firstID = (firstID & mask) | (static_cast<uint64_t>(byte) << ((7 - position) * 8));
			lastID = (lastID & mask) | (static_cast<uint64_t>(byte) << ((7 - position) * 8));
		}

		position++;

		if (nextPart == std::string::npos)
			break;
		
		namePos = nextPart + 1;
	}

	return position >= 0;
}

ProjectedFilesystem::ProjectedFilesystem() : m_run(true) {
	m_rootInode.fileKey = static_cast<uint64_t>(-1);
	m_rootInode.info.IsDirectory = TRUE;
	m_rootInode.info.FileSize = 0;
	m_rootInode.info.CreationTime.QuadPart = 0;
	m_rootInode.info.LastAccessTime.QuadPart = 0;
	m_rootInode.info.LastWriteTime.QuadPart = 0;
	m_rootInode.info.ChangeTime.QuadPart = 0;
	m_rootInode.info.FileAttributes = 0;
}

ProjectedFilesystem::~ProjectedFilesystem() = default;

std::wstring ProjectedFilesystem::buildNameForID(uint64_t key, uint64_t mask) const {
	std::wstringstream nameBuf;

	nameBuf << "\\by-id";

	for (size_t byte = 0; byte < 8; byte++) {
		if (mask & (0xFF00000000000000ULL >> (byte * 8))) {
			nameBuf << "\\";
			nameBuf << std::hex;
			nameBuf.width(2);
			nameBuf.fill('0');
			nameBuf << ((key >> (56 - byte * 8)) & 0xFF);
		}
	}

	return nameBuf.str();
}


void ProjectedFilesystem::initialize(const std::string &rootDirectory, const std::vector<std::string> &archives, const std::vector<uint64_t> &fileTables) {
	m_rootDirectory = archiveparse::utf8ToWide(rootDirectory);

	auto instanceFile = std::filesystem::path(rootDirectory) / "virtualizationInstanceId";

	GUID virtualizationInstanceGuid;

	bool recreate = true;

	std::fstream stream;
	stream.open(instanceFile, std::ios::in);
	if (stream) {
		stream.read(reinterpret_cast<char *>(&virtualizationInstanceGuid), sizeof(virtualizationInstanceGuid));
		if (stream)
			recreate = false;
		stream.close();
	}

	stream.clear();

	if (recreate) {
		stream.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
		struct FileGuard {
			bool success;
			std::filesystem::path instanceFile;

			FileGuard(const std::filesystem::path &instanceFile) : instanceFile(instanceFile), success(false) {

			}

			~FileGuard() {
				if (!success)
					std::filesystem::remove(instanceFile);
			}
		} fileGuard(instanceFile);

		stream.open(instanceFile, std::ios::out | std::ios::trunc | std::ios::binary);

		CoCreateGuid(&virtualizationInstanceGuid);

		stream.write(reinterpret_cast<const char *>(&virtualizationInstanceGuid), sizeof(virtualizationInstanceGuid));
		stream.close();

		auto hr = PrjMarkDirectoryAsPlaceholder(m_rootDirectory.c_str(), nullptr, nullptr, &virtualizationInstanceGuid);
		if (FAILED(hr))
			throw archiveparse::WindowsError(HRESULT_CODE(hr));

		fileGuard.success = true;
	}

	ZeroMemory(&m_callbacks, sizeof(m_callbacks));
	m_callbacks.StartDirectoryEnumerationCallback = startDirectoryEnumeration;
	m_callbacks.EndDirectoryEnumerationCallback = endDirectoryEnumeration;
	m_callbacks.GetDirectoryEnumerationCallback = getDirectoryEnumeration;
	m_callbacks.GetPlaceholderInfoCallback = getPlaceholderInfo;
	m_callbacks.GetFileDataCallback = getFileData;

	for (const auto &archive : archives) {
		m_fs.addManifest(archive);
	}

	for (auto fileTable : fileTables) {
		m_fs.loadFileTable(fileTable);
	}

	m_fs.enumerateFiles([this](uint64_t key, size_t size) {
		m_files.emplace(key, size);
	});
}

void ProjectedFilesystem::run() {
	auto threads = std::thread::hardware_concurrency();

	PRJ_STARTVIRTUALIZING_OPTIONS options;
	options.Flags = PRJ_FLAG_NONE;
	options.PoolThreadCount = threads;
	options.ConcurrentThreadCount = threads;
	options.NotificationMappings = nullptr;
	options.NotificationMappingsCount = 0;

	auto hr = PrjStartVirtualizing(m_rootDirectory.c_str(), &m_callbacks, this, &options, &m_context);
	if (FAILED(hr))
		throw archiveparse::WindowsError(HRESULT_CODE(hr));

	{
		std::unique_lock<std::mutex> lock(m_runMutex);
		while (m_run)
			m_runCondvar.wait(lock);
	}

	PrjStopVirtualizing(m_context);
}

void ProjectedFilesystem::stop() {
	{
		std::unique_lock<std::mutex> lock(m_runMutex);

		m_run = false;
		m_runCondvar.notify_all();
	}
}

Inode *ProjectedFilesystem::createInode(const std::wstring &name) const {
	Inode *parentInode, *thisInode;
	std::wstring thisName, thisNameWithPath;

	bool result;

	do {
		result = pathToInode(name, parentInode, thisName, thisInode, thisNameWithPath);

		if (!result) {
			auto directoryInode = std::make_unique<Inode>();
			directoryInode->name = thisName;
			directoryInode->fullCanonicalPath = thisNameWithPath;
			directoryInode->fileKey = static_cast<uint64_t>(-1);
			directoryInode->info.IsDirectory = TRUE;
			directoryInode->info.FileSize = 0;
			directoryInode->info.CreationTime.QuadPart = 0;
			directoryInode->info.LastAccessTime.QuadPart = 0;
			directoryInode->info.LastWriteTime.QuadPart = 0;
			directoryInode->info.ChangeTime.QuadPart = 0;
			directoryInode->info.FileAttributes = 0;
			parentInode->children.insert(std::make_pair(thisName, std::move(directoryInode)));
		}
	} while (!result);

	if (!thisInode) {
		auto fileInode = std::make_unique<Inode>();
		fileInode->fullCanonicalPath = thisNameWithPath;;
		fileInode->info.IsDirectory = FALSE;
		fileInode->info.FileSize = 0;
		fileInode->info.CreationTime.QuadPart = 0;
		fileInode->info.LastAccessTime.QuadPart = 0;
		fileInode->info.LastWriteTime.QuadPart = 0;
		fileInode->info.ChangeTime.QuadPart = 0;
		fileInode->info.FileAttributes = 0;
		fileInode->name = thisName;
		thisInode = fileInode.get();
		parentInode->children.insert(std::make_pair(thisName, std::move(fileInode)));
	}

	return thisInode;
}

bool ProjectedFilesystem::pathToInode(const std::wstring &initialPath, Inode *&parentInode, std::wstring &thisName, Inode *&thisInode, std::wstring &thisNameWithPath) const {
	std::wstring path(initialPath);
	while (!path.empty() && path.back() == L'\\')
		path.erase(path.size() - 1, 1);

	parentInode = nullptr;
	thisInode = nullptr;

	size_t namePos = 0;

	while (true) {
		size_t nextPart = path.find(L'\\', namePos);

		if (nextPart == std::string::npos) {
			thisName = path.substr(namePos);
			thisNameWithPath = path;
		}
		else {
			thisName = path.substr(namePos, nextPart - namePos);
			thisNameWithPath = path.substr(0, nextPart);
		}

		if (parentInode == nullptr) {
			if (!thisName.empty())
				return false;

			thisInode = &m_rootInode;
		}
		else {
			auto it = parentInode->children.find(thisName);
			if (it == parentInode->children.end()) {
				thisInode = false;
			}
			else {
				thisInode = it->second.get();
			}
		}

		if (nextPart == std::string::npos)
			break;

		if (thisInode == nullptr)
			return false;

		parentInode = thisInode;

		namePos = nextPart + 1;
	}

	return true;
}

const Inode *ProjectedFilesystem::resolve(const std::wstring &name) const {
	Inode *parentInode, *thisInode;
	std::wstring thisName, thisNameWithPath;

	if (!pathToInode(name, parentInode, thisName, thisInode, thisNameWithPath))
		return nullptr;

	return thisInode;
}


HRESULT CALLBACK ProjectedFilesystem::startDirectoryEnumeration(const PRJ_CALLBACK_DATA *callbackData, const GUID *enumerationGuid) {
	auto this_ = static_cast<ProjectedFilesystem *>(callbackData->InstanceContext);
	
	auto enumeration = std::make_unique<DirectoryEnumeration>();
	enumeration->path = std::wstring(L"\\") + callbackData->FilePathName;

	enumeration->byID = this_->parseByIDName(enumeration->path, enumeration->firstID, enumeration->lastID);

	if (enumeration->byID) {
		if(enumeration->firstID == enumeration->lastID)
			return HRESULT_FROM_WIN32(ERROR_DIRECTORY);
	}
	else {
		enumeration->inode = this_->resolve(enumeration->path);
		enumeration->scanInProgress = false;

		if (!enumeration->inode)
			return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

		if (!enumeration->inode->info.IsDirectory)
			return HRESULT_FROM_WIN32(ERROR_DIRECTORY);
	}

	auto enumerationPtr = enumeration.get();

	{
		std::unique_lock lock(this_->m_directoryEnumerationMutex);
		this_->m_directoryEnumerations.emplace(*enumerationGuid, std::move(enumeration));
	}

	return S_OK;
}

HRESULT CALLBACK ProjectedFilesystem::endDirectoryEnumeration(const PRJ_CALLBACK_DATA *callbackData, const GUID *enumerationGuid) {
	auto this_ = static_cast<ProjectedFilesystem *>(callbackData->InstanceContext);
	{
		std::unique_lock lock(this_->m_directoryEnumerationMutex);
		if (this_->m_directoryEnumerations.erase(*enumerationGuid) == 0)
			return HRESULT_FROM_WIN32(ERROR_GEN_FAILURE);
	}

	return S_OK;
}

void ProjectedFilesystem::byIdSplit(uint64_t pos, uint64_t first, uint64_t last, uint8_t &byte, uint64_t &next) {
	auto mask = last - first;
	switch (mask) {
	case 0xFFFFFFFFFFFFFFFFULL:
		byte = static_cast<uint8_t>(pos >> (7 * 8));
		next = pos + (1ULL << (7 * 8)) & ~(static_cast<uint64_t>(-1) >> (1 * 8));
		break;

	case 0x00FFFFFFFFFFFFFFULL:
		byte = static_cast<uint8_t>(pos >> (6 * 8));
		next = pos + (1ULL << (6 * 8)) & ~(static_cast<uint64_t>(-1) >> (2 * 8));
		break;

	case 0x0000FFFFFFFFFFFFULL:
		byte = static_cast<uint8_t>(pos >> (5 * 8));
		next = pos + (1ULL << (5 * 8)) & ~(static_cast<uint64_t>(-1) >> (3 * 8));
		break;

	case 0x000000FFFFFFFFFFULL:
		byte = static_cast<uint8_t>(pos >> (4 * 8));
		next = pos + (1ULL << (4 * 8)) & ~(static_cast<uint64_t>(-1) >> (4 * 8));
		break;

	case 0x00000000FFFFFFFFULL:
		byte = static_cast<uint8_t>(pos >> (3 * 8));
		next = pos + (1ULL << (3 * 8)) & ~(static_cast<uint64_t>(-1) >> (5 * 8));
		break;

	case 0x0000000000FFFFFFULL:
		byte = static_cast<uint8_t>(pos >> (2 * 8));
		next = pos + (1ULL << (2 * 8)) & ~(static_cast<uint64_t>(-1) >> (6 * 8));
		break;

	case 0x000000000000FFFFULL:
		byte = static_cast<uint8_t>(pos >> (1 * 8));
		next = pos + (1ULL << (1 * 8)) & ~(static_cast<uint64_t>(-1) >> (7 * 8));
		break;

	case 0x00000000000000FFULL:
		byte = static_cast<uint8_t>(pos);
		next = pos + 1;
		break;

	default:
		throw std::logic_error("bad mask");
	}
}

HRESULT CALLBACK ProjectedFilesystem::getDirectoryEnumeration(const PRJ_CALLBACK_DATA *callbackData, const GUID *enumerationGuid, PCWSTR searchExpression, PRJ_DIR_ENTRY_BUFFER_HANDLE dirEntryBufferHandle) {
	auto this_ = static_cast<ProjectedFilesystem *>(callbackData->InstanceContext);

	DirectoryEnumeration *enumeration;

	{
		std::unique_lock lock(this_->m_directoryEnumerationMutex);
		auto it = this_->m_directoryEnumerations.find(*enumerationGuid);
		if(it == this_->m_directoryEnumerations.end())
			return HRESULT_FROM_WIN32(ERROR_GEN_FAILURE);

		enumeration = it->second.get();
	}

	if (!enumeration->scanInProgress || (callbackData->Flags & PRJ_CB_DATA_FLAG_ENUM_RESTART_SCAN)) {
		enumeration->scanInProgress = true;
		if (searchExpression && *searchExpression) {
			enumeration->filter = true;
			enumeration->pattern = searchExpression;
		}
		else {
			enumeration->filter = false;
		}
		if (enumeration->byID) {
			enumeration->posID = enumeration->firstID;
		}
		else {
			enumeration->iterator = enumeration->inode->children.begin();
		}
	}
	
	bool writtenEntry = false;

	if (enumeration->byID) {
		auto enumerationLimit = this_->m_files.upper_bound(enumeration->lastID);

		do {
			auto found = this_->m_files.lower_bound(enumeration->posID);
			if (found == enumerationLimit)
				break;

			uint8_t byte;
			uint64_t next;

			byIdSplit(found->first, enumeration->firstID, enumeration->lastID, byte, next);

			std::wstringstream name;
			name.width(2);
			name.fill('0');
			name << std::hex << byte;

			bool match = true;

			if (enumeration->filter) {
				match = PrjFileNameMatch(name.str().c_str(), enumeration->pattern.c_str());
			}

			if (match) {
				PRJ_FILE_BASIC_INFO info;
				info.CreationTime.QuadPart = 0;
				info.LastAccessTime.QuadPart = 0;
				info.LastWriteTime.QuadPart = 0;
				info.ChangeTime.QuadPart = 0;
				info.FileAttributes = 0;

				if ((enumeration->lastID - enumeration->firstID) == 0xFF) {
					info.IsDirectory = FALSE;
					info.FileSize = found->second;
				}
				else {
					info.IsDirectory = TRUE;
					info.FileSize = 0;
				}

				auto hr = PrjFillDirEntryBuffer(
					name.str().c_str(),
					&info,
					dirEntryBufferHandle
				);

				if (FAILED(hr)) {
					if (writtenEntry)
						break;
					else
						return hr;
				}
				else {
					writtenEntry = true;
				}
			}

			enumeration->posID = next;

		} while (enumeration->posID > enumeration->firstID && enumeration->posID <= enumeration->lastID);
	}
	else {

		while (enumeration->iterator != enumeration->inode->children.end()) {
			auto &entry = *enumeration->iterator;

			bool match = true;

			auto name = entry.second->name;

			if (enumeration->filter) {
				match = PrjFileNameMatch(name.c_str(), enumeration->pattern.c_str());
			}

			if (match) {
				auto hr = PrjFillDirEntryBuffer(
					name.c_str(),
					&entry.second->info,
					dirEntryBufferHandle
				);

				if (FAILED(hr)) {
					if (writtenEntry)
						break;
					else
						return hr;
				}
				else {
					writtenEntry = true;
				}
			}

			enumeration->iterator++;
		}
	}

	return S_OK;
}

HRESULT CALLBACK ProjectedFilesystem::getPlaceholderInfo(const PRJ_CALLBACK_DATA *callbackData) {
	auto this_ = static_cast<ProjectedFilesystem *>(callbackData->InstanceContext);
	auto path = std::wstring(L"\\") + callbackData->FilePathName;

	uint64_t firstID, lastID;

	auto byID = this_->parseByIDName(path, firstID, lastID);

	PRJ_PLACEHOLDER_INFO placeholderInfo;
	ZeroMemory(&placeholderInfo, sizeof(placeholderInfo));

	if (byID) {

		auto name = this_->buildNameForID(firstID, ~(lastID - firstID));

		if (firstID == lastID) {
			auto fileIt = this_->m_files.find(firstID);
			if(fileIt == this_->m_files.end())
				return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

			placeholderInfo.FileBasicInfo.IsDirectory = FALSE;
			placeholderInfo.FileBasicInfo.FileSize = fileIt->second;
		}
		else {
			auto lower = this_->m_files.lower_bound(firstID);
			auto upper = this_->m_files.upper_bound(lastID);
			if(lower == upper)
				return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

			placeholderInfo.FileBasicInfo.IsDirectory = TRUE;
			placeholderInfo.FileBasicInfo.FileSize = 0;

		}

		return PrjWritePlaceholderInfo(
			this_->m_context,
			path.c_str() + 1,
			&placeholderInfo,
			sizeof(placeholderInfo)
		);

	}
	else {
		auto inode = this_->resolve(path);
		if (inode == nullptr)
			return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

		placeholderInfo.FileBasicInfo = inode->info;

		auto path = inode->fullCanonicalPath.c_str();
		if (*path == L'\\')
			path++;

		return PrjWritePlaceholderInfo(
			this_->m_context,
			path,
			&placeholderInfo,
			sizeof(placeholderInfo)
		);
	}
}

HRESULT CALLBACK ProjectedFilesystem::getFileData(const PRJ_CALLBACK_DATA *callbackData, UINT64 byteOffset, UINT32 length) {
	uint64_t firstID, lastID;

	auto this_ = static_cast<ProjectedFilesystem *>(callbackData->InstanceContext);

	auto path = std::wstring(L"\\") + callbackData->FilePathName;

	auto byID = this_->parseByIDName(path, firstID, lastID);

	uint64_t id;

	if (byID) {

		if (firstID != lastID) {
			auto lower = this_->m_files.lower_bound(firstID);
			auto upper = this_->m_files.upper_bound(lastID);
			if (lower == upper)
				return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

			return HRESULT_FROM_WIN32(ERROR_DIRECTORY);
		}

		auto found = this_->m_files.find(firstID);
		if(found == this_->m_files.end())
			return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

		id = found->first;
	}
	else {
		auto inode = this_->resolve(path);
		if (inode == nullptr)
			return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

		if (inode->info.IsDirectory)
			return HRESULT_FROM_WIN32(ERROR_DIRECTORY);

		id = inode->fileKey;
	}

	try {
		auto data = this_->m_fs.readFileByKey(id);

		size_t realLength;
		if (byteOffset > data.size())
			realLength = 0;
		else {
			realLength = std::min<size_t>(length, data.size() - byteOffset);
		}

		auto buffer = PrjAllocateAlignedBuffer(callbackData->NamespaceVirtualizationContext, realLength);
		if (buffer == nullptr)
			return HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);

		memcpy(buffer, data.data() + byteOffset, realLength);

		auto hr = PrjWriteFileData(callbackData->NamespaceVirtualizationContext, &callbackData->DataStreamId, buffer, byteOffset, realLength);

		PrjFreeAlignedBuffer(buffer);

		return hr;
	}
	catch (const std::exception &e) {
		fprintf(stderr, "Exception reading %ls: %s\n", callbackData->FilePathName, e.what());
		return HRESULT_FROM_WIN32(ERROR_GEN_FAILURE);
	}
}
