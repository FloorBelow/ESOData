#include "ProjectedFilesystem.h"

#include <archiveparse/EncodingUtilities.h>
#include <archiveparse/WindowsError.h>

#include <objbase.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include <ESOData/Serialization/Hash.h>

size_t std::hash<GUID>::operator()(const GUID &guid) const {
	return static_cast<size_t>(esodata::hashData64(reinterpret_cast<const unsigned char *>(&guid), sizeof(guid)));
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

std::wstring ProjectedFilesystem::buildNameForID(uint64_t key) const {
	std::wstringstream nameBuf;

	nameBuf << "\\by-id\\";
	nameBuf << std::hex;
	nameBuf.width(2);
	nameBuf.fill('0');
	nameBuf << ((key >> 56) & 0xFF) << "\\";
	nameBuf.width(2);
	nameBuf.fill('0');
	nameBuf << ((key >> 48) & 0xFF) << "\\";
	nameBuf.width(2);
	nameBuf.fill('0');
	nameBuf << ((key >> 40) & 0xFF) << "\\";
	nameBuf.width(2);
	nameBuf.fill('0');
	nameBuf << ((key >> 32) & 0xFF) << "\\";
	nameBuf.width(2);
	nameBuf.fill('0');
	nameBuf << ((key >> 24) & 0xFF) << "\\";
	nameBuf.width(2);
	nameBuf.fill('0');
	nameBuf << ((key >> 16) & 0xFF) << "\\";
	nameBuf.width(2);
	nameBuf.fill('0');
	nameBuf << ((key >> 8) & 0xFF) << "\\";
	nameBuf.width(2);
	nameBuf.fill('0');
	nameBuf << (key & 0xFF);

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
		auto inode = createInode(buildNameForID(key));
		inode->fileKey = key;
		inode->info.FileSize = size;
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
	enumeration->inode = this_->resolve(enumeration->path);
	enumeration->scanInProgress = false;

	if (!enumeration->inode)
		return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

	if (!enumeration->inode->info.IsDirectory)
		return HRESULT_FROM_WIN32(ERROR_DIRECTORY);

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
		enumeration->iterator = enumeration->inode->children.begin();
	}
	
	bool writtenEntry = false;

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

	return S_OK;
}

HRESULT CALLBACK ProjectedFilesystem::getPlaceholderInfo(const PRJ_CALLBACK_DATA *callbackData) {
	printf("Request: %ls\n", callbackData->FilePathName);

	auto this_ = static_cast<ProjectedFilesystem *>(callbackData->InstanceContext);
	auto inode = this_->resolve(std::wstring(L"\\") + callbackData->FilePathName);
	if(inode == nullptr)
		return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

	PRJ_PLACEHOLDER_INFO placeholderInfo;
	ZeroMemory(&placeholderInfo, sizeof(placeholderInfo));
	placeholderInfo.FileBasicInfo = inode->info;

	auto path = inode->fullCanonicalPath.c_str();
	if (*path == L'\\')
		path++;

	auto hr = PrjWritePlaceholderInfo(
		this_->m_context,
		path,
		&placeholderInfo,
		sizeof(placeholderInfo)
	);

	return hr;
}

HRESULT CALLBACK ProjectedFilesystem::getFileData(const PRJ_CALLBACK_DATA *callbackData, UINT64 byteOffset, UINT32 length) {
	
	auto this_ = static_cast<ProjectedFilesystem *>(callbackData->InstanceContext);
	auto inode = this_->resolve(std::wstring(L"\\") + callbackData->FilePathName);
	if (inode == nullptr)
		return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

	if (inode->info.IsDirectory)
		return HRESULT_FROM_WIN32(ERROR_DIRECTORY);
	
	printf("GetFileData: %ls, %016llX, %08X\n", callbackData->FilePathName, byteOffset, length);

	try {
		auto data = this_->m_fs.readFileByKey(inode->fileKey);

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
