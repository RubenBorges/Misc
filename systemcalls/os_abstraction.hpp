#pragma once
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

#if defined(_WIN32) || defined(_WIN64)
#define _WIN32_WINNT 0x0501
#include <windows.h>
using os_uint = UINT;
using os_dword = DWORD;
using os_bool = BOOLEAN;
#define OS_SHUTDOWN_COMMAND "C:\\Windows\\System32\\shutdown.exe -s -t 0 -f -c"
os_dword reason = SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED;

inline void os_clear_screen() { system("cls"); }
inline int os_shutdown() { return system(OS_SHUTDOWN_COMMAND); }
inline int os_force_shutdown() { return ExitWindowsEx(EWX_POWEROFF | EWX_FORCEIFHUNG, reason); }
#else
#include <unistd.h>
#include <sys/reboot.h>
using os_uint = unsigned int;
using os_dword = unsigned int;
using os_bool = bool;
#define OS_SHUTDOWN_COMMAND "shutdown -h now"
inline void os_clear_screen() { std::cout << "\033[7;1H"; system("clear"); }
inline int os_shutdown() { return system(OS_SHUTDOWN_COMMAND); }
inline int os_force_shutdown() { return reboot(RB_POWER_OFF); }
#endif

// Example wrapper for privilege enabling (no-op on Linux)
inline bool os_enable_shutdown_privilege() {
#if defined(_WIN32) || defined(_WIN64)

bool EnablePrivilege(LPCTSTR privName)
{
	HANDLE hToken = nullptr;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		std::cerr << "OpenProcessToken failed: " << GetLastError() << "\n";
		return false;
	}

	TOKEN_PRIVILEGES tp{};
	LUID luid;
	if (!LookupPrivilegeValue(nullptr, privName, &luid))
	{
		std::cerr << "LookupPrivilegeValue failed: " << GetLastError() << "\n";
		CloseHandle(hToken);
		return false;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr))
	{
		std::cerr << "AdjustTokenPrivileges failed: " << GetLastError() << "\n";
		CloseHandle(hToken);
		return false;
	}
	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		std::cerr << "Privilege not assigned. Run as Administrator.\n";
		CloseHandle(hToken);
		return false;
	}

	CloseHandle(hToken);
	return true;
}

int ForceAction(os_uint flags, os_dword reason)
{
	if (!os_enable_shutdown_privilege(SE_SHUTDOWN_NAME))
		return 1;

	if (!ExitWindowsEx(flags, reason))
	{
		std::cerr << "ExitWindowsEx failed: " << GetLastError() << "\n";
		return 2;
	}
	return 0;
}
#else
    return true; // No privilege escalation needed for shutdown on most Linux
#endif
}
