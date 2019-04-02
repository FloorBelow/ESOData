#ifndef PROJECTED_FILESYSTEM_H
#define PROJECTED_FILESYSTEM_H

#include <mutex>
#include <condition_variable>
#include <vector>
#include <string>

#include <Windows.h>
#include <projectedfslib.h>

#include <ESOData/Filesystem/Filesystem.h>

#include "Inode.h"

#include <shared_mutex>
#include <unordered_map>

struct DirectoryEnumeration {
	std::wstring path;
	const Inode *inode;
	bool scanInProgress;
	bool filter;
	bool byID;
	uint64_t firstID;
	uint64_t lastID;
	uint64_t posID;
	std::wstring pattern;
	std::map<std::wstring, std::unique_ptr<Inode>>::const_iterator iterator;
};

template<>
struct std::hash<GUID> {
	size_t operator()(const GUID &guid) const;
};

class ProjectedFilesystem {
public:
	ProjectedFilesystem();
	~ProjectedFilesystem();

	ProjectedFilesystem(const ProjectedFilesystem &other) = delete;
	ProjectedFilesystem &operator =(const ProjectedFilesystem &other) = delete;

	void initialize(const std::string &rootDirectory, const std::vector<std::string> &archives, const std::vector<uint64_t> &fileTables);

	void run();
	void stop();

private:
	static HRESULT CALLBACK startDirectoryEnumeration(const PRJ_CALLBACK_DATA *callbackData, const GUID *enumerationGuid);
	static HRESULT CALLBACK endDirectoryEnumeration(const PRJ_CALLBACK_DATA *callbackData, const GUID *enumerationGuid);
	static HRESULT CALLBACK getDirectoryEnumeration(const PRJ_CALLBACK_DATA *callbackData, const GUID *enumerationGuid, PCWSTR searchExpression, PRJ_DIR_ENTRY_BUFFER_HANDLE dirEntryBufferHandle);
	static HRESULT CALLBACK getPlaceholderInfo(const PRJ_CALLBACK_DATA *callbackData);
	static HRESULT CALLBACK getFileData(const PRJ_CALLBACK_DATA *callbackData, UINT64 byteOffset, UINT32 length);

	bool pathToInode(const std::wstring &name, Inode *&parentInode, std::wstring &thisName, Inode *&thisInode, std::wstring &thisNameWithPath) const;
	Inode *createInode(const std::wstring &name) const;

	const Inode *resolve(const std::wstring &name) const;

	std::wstring buildNameForID(uint64_t id, uint64_t mask) const;

	bool parseByIDName(const std::wstring &name, uint64_t &firstID, uint64_t &lastID);
	static void byIdSplit(uint64_t pos, uint64_t first, uint64_t last, uint8_t &byte, uint64_t &next);

	std::mutex m_runMutex;
	bool m_run;
	std::condition_variable m_runCondvar;
	std::wstring m_rootDirectory;
	PRJ_CALLBACKS m_callbacks;
	PRJ_NAMESPACE_VIRTUALIZATION_CONTEXT m_context;
	esodata::Filesystem m_fs;
	mutable Inode m_rootInode;
	std::map<uint64_t, size_t> m_files;

	std::shared_mutex m_directoryEnumerationMutex;
	std::unordered_map<GUID, std::unique_ptr<DirectoryEnumeration>> m_directoryEnumerations;

};

#endif
