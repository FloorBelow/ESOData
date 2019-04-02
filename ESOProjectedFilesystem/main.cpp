#include <CLI11.hpp>

#include <archiveparse/EncodingUtilities.h>
#include <archiveparse/WindowsError.h>

#include <Windows.h>
#include <ShlObj.h>
#include <comdef.h>

#include "ProjectedFilesystem.h"

struct SCMHandleDeleter {
	inline void operator()(SC_HANDLE handle) const {
		CloseServiceHandle(handle);
	}
};

extern "C" {
	extern unsigned char __ImageBase;
}

using SCMHandle = std::unique_ptr<std::remove_pointer<SC_HANDLE>::type, SCMHandleDeleter>;

bool NotInService;
SERVICE_STATUS_HANDLE ServiceControlHandle;
SERVICE_STATUS ServiceStatus;
ProjectedFilesystem FilesystemInstance;

DWORD CALLBACK ServiceControlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) {
	(void)dwEventType;
	(void)lpEventData;
	(void)lpContext;

	switch (dwControl) {
	case SERVICE_CONTROL_INTERROGATE:
		return NO_ERROR;

	case SERVICE_CONTROL_STOP:
		FilesystemInstance.stop();

		return NO_ERROR;
	}

	return ERROR_CALL_NOT_IMPLEMENTED;
}

void ExitService(int Code) {
	if (!NotInService) {
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		if (Code != 0) {
			ServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
			ServiceStatus.dwServiceSpecificExitCode = 1;
		}
		else {
			ServiceStatus.dwWin32ExitCode = ERROR_SUCCESS;
		}
		if (!SetServiceStatus(ServiceControlHandle, &ServiceStatus))
			throw archiveparse::WindowsError();
	}

	exit(0);
}

void CALLBACK ServiceMain(
	DWORD argc,
	LPWSTR *argv
) {
	CLI::App cliApp;

	std::vector<std::string> archives;
	std::vector<uint64_t> fileTables;
	std::string instanceName = "ESOProjectedFilesystem";
	bool serviceRegister, serviceUnregister;

	std::string rootDirectory;
	std::string virtualizationInstanceGuid;

	cliApp.set_config("--via");
	cliApp.add_option("--archive", archives, "Mount specified MNF file");
	cliApp.add_option("--filetable", fileTables, "Read specified file table");
	cliApp.add_option("--instance", instanceName, "Service name", true);
	cliApp.add_option("--root-directory", rootDirectory, "Root directory")->mandatory(true);

	cliApp.add_flag("--register", serviceRegister, "Register as service")->configurable(false);
	cliApp.add_flag("--unregister", serviceUnregister, "Unregister as service")->configurable(false);

	{
		std::vector<std::string> strings(argc);
		std::vector<const char *> stringPointers(argc);

		for (unsigned int index = 0; index < argc; index++) {
			strings[index] = archiveparse::wideToUtf8(argv[index]);
			stringPointers[index] = strings[index].c_str();
		}
		try {

			cliApp.parse(stringPointers.size(), stringPointers.data());
		}
		catch (const CLI::ParseError &e) {
			ExitProcess(cliApp.exit(e));
		}
	}

	ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	ServiceStatus.dwWin32ExitCode = NO_ERROR;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 5000;

	if (!NotInService) {
		ServiceControlHandle = RegisterServiceCtrlHandlerEx(archiveparse::utf8ToWide(instanceName).c_str(), ServiceControlHandler, nullptr);
		if (!ServiceControlHandle)
			throw archiveparse::WindowsError();
		
		if(!SetServiceStatus(ServiceControlHandle, &ServiceStatus))
			throw archiveparse::WindowsError();
	}

	try {

		if (serviceRegister) {
			try {
				std::stringstream pathName;

				auto mod = reinterpret_cast<HMODULE>(&__ImageBase);

				std::vector<wchar_t> fileName(_MAX_PATH);
				DWORD result;
				while (true) {
					result = GetModuleFileName(mod, fileName.data(), static_cast<DWORD>(fileName.size()));
					if (result == 0)
						throw std::runtime_error("GetModuleFileName failed");

					if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
						fileName.resize(fileName.size() * 2);
					}
					else {
						fileName.resize(result);
						break;
					}
				}

				pathName << "\"" << archiveparse::wideToUtf8(std::wstring(fileName.data(), fileName.size())) << "\"";

				struct WatchedPath {
					WatchedPath() : path(nullptr) {

					}

					~WatchedPath() {
						CoTaskMemFree(path);
					}

					PWSTR path;
				} watchedPath;

				auto hr = SHGetKnownFolderPath(
					FOLDERID_ProgramData,
					KF_FLAG_CREATE | KF_FLAG_INIT,
					nullptr,
					&watchedPath.path);
				if (FAILED(hr))
					_com_raise_error(hr);

				auto configFile = archiveparse::wideToUtf8(watchedPath.path) + "\\" + instanceName + ".ini";

				{
					std::ofstream stream;
					stream.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
					stream.open(archiveparse::utf8ToWide(configFile), std::ios::out | std::ios::trunc);
					stream << cliApp.config_to_str(true, true);
				}

				pathName << "  \"--via=" << configFile << "\"";

				auto scHandleRaw = OpenSCManager(nullptr, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CREATE_SERVICE);
				if (scHandleRaw == nullptr)
					throw archiveparse::WindowsError();

				SCMHandle scHandle(scHandleRaw);

				auto serviceHandleRaw = CreateService(scHandle.get(),
					archiveparse::utf8ToWide(instanceName).c_str(),
					archiveparse::utf8ToWide("ESO Projected Filesystem (instance " + instanceName + ")").c_str(),
					0,
					SERVICE_WIN32_OWN_PROCESS,
					SERVICE_DEMAND_START,
					SERVICE_ERROR_NORMAL,
					archiveparse::utf8ToWide(pathName.str()).c_str(),
					nullptr,
					nullptr,
					nullptr,
					nullptr,
					nullptr);

				if (serviceHandleRaw == nullptr)
					throw archiveparse::WindowsError();

				CloseServiceHandle(serviceHandleRaw);

			}
			catch (const std::exception &e) {
				fprintf(stderr, "Failed to register as service: %s\n", e.what());
				ExitService(1);
			}

			ExitService(0);
		}

		if (serviceUnregister) {
			try {
				auto scHandleRaw = OpenSCManager(nullptr, SERVICES_ACTIVE_DATABASE, 0);
				if (scHandleRaw == nullptr)
					throw archiveparse::WindowsError();

				SCMHandle scHandle(scHandleRaw);

				auto serviceHandleRaw = OpenService(scHandle.get(), archiveparse::utf8ToWide(instanceName).c_str(), DELETE);
				if (serviceHandleRaw == nullptr)
					throw archiveparse::WindowsError();

				SCMHandle serviceHandle(serviceHandleRaw);
				if (!DeleteService(serviceHandle.get()))
					throw archiveparse::WindowsError();
			}
			catch (const std::exception &e) {
				fprintf(stderr, "Failed to unregister as service: %s\n", e.what());
				ExitService(1);
			}

			ExitService(0);
		}

		FilesystemInstance.initialize(rootDirectory, archives, fileTables);

		if (!NotInService) {
			ServiceStatus.dwCurrentState = SERVICE_RUNNING;
			if (!SetServiceStatus(ServiceControlHandle, &ServiceStatus))
				throw archiveparse::WindowsError();
		}

		FilesystemInstance.run();

		ExitService(0);
	}
	catch (const std::exception &e) {
		fprintf(stderr, "Uncaught exception: %s\n", e.what());
		ExitService(1);
	}
};

int wmain(int argc, wchar_t *argv[]) {
	static const SERVICE_TABLE_ENTRY serviceTable[]{
		{ L"", ServiceMain },
		{ nullptr, nullptr }
	};

	if (!StartServiceCtrlDispatcher(serviceTable)) {
		auto error = GetLastError();

		if (error == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
			NotInService = true;
			ServiceMain(argc, argv);
			return 0;
		}

		try {
			throw archiveparse::WindowsError(error);
		}
		catch (const std::exception &e) {
			fprintf(stderr, "Failed to connect to SCM: %s\n", e.what());
			return 1;
		}
	}

	return 0;
}
