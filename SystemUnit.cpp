#pragma hdrstop
#include "SystemUnit.h"
#pragma package(smart_init)

UnicodeString GetInfoAboutJob(HANDLE hProcess)
{
	BOOL isProcessInJob;
	if (IsProcessInJob(hProcess, nullptr, &isProcessInJob))
	{
		return isProcessInJob ? "Выполняется" : "Приостановлен";
	}
	else
	{
		return "Error checking process virtualization.";
	}
}

UnicodeString GetPriority(HANDLE hProcess)
{
	UnicodeString priority;
    switch (GetPriorityClass(hProcess))
    {
        case HIGH_PRIORITY_CLASS:
            priority = "Высокий";
            break;
        case IDLE_PRIORITY_CLASS:
            priority = "Низкий";
            break;
        case NORMAL_PRIORITY_CLASS:
            priority = "Средний";
            break;
        case REALTIME_PRIORITY_CLASS:
			priority = "Реального времени";
            break;
        case ABOVE_NORMAL_PRIORITY_CLASS:
            priority = "Выше среднего";
            break;
        case BELOW_NORMAL_PRIORITY_CLASS:
            priority = "Ниже среднего";
			break;
        default:
			priority = "Неизвестно";
	}
    return priority;
}

UnicodeString GetProcessCreationTime(HANDLE hProcess)
{
	FILETIME creationTime, exitTime, kernelTime, userTime;
	try {
		if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime))
		{
			SYSTEMTIME creationSystemTime;
			FileTimeToSystemTime(&creationTime, &creationSystemTime);
			creationSystemTime.wHour += 3; // Коррекция времени на 3 часа (по МСК)
			return FormatDateTime(L"dd.mm.yyyy hh:nn:ss", SystemTimeToDateTime(creationSystemTime));
		}
	} catch (...) {
		return "N/A";
	}
}

UnicodeString GetProcessPath(HANDLE hProcess)
{
	TCHAR processPath[MAX_PATH];
	if (GetModuleFileNameEx(hProcess, nullptr, processPath, sizeof(processPath) / sizeof(TCHAR)))
	{
		return processPath;
	}
	else
	{
        return "N/A";
	}
}

UnicodeString GetProcessBitness(HANDLE hProcess)
{
    BOOL isWow64;
	if (IsWow64Process(hProcess, &isWow64))
	{
        return isWow64 ? "32-битный" : "64-битный";
	}
	else
	{
		return "N/A";
    }
}

UnicodeString GetCpuUsage(HANDLE hProcess) {
	FILETIME creationTime, exitTime, kernelTime, userTime;
	if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime))
	{
        ULONGLONG totalTime = kernelTime.dwHighDateTime + kernelTime.dwLowDateTime + userTime.dwHighDateTime + userTime.dwLowDateTime;
		double cpuUsage = (totalTime * 100.0) / (10000000.0 * GetTickCount());
        return FormatFloat("0.00000000%", cpuUsage);
	}
	else
	{
        return "N/A";
	}
}

UnicodeString GetMemoryUsage(HANDLE hProcess) {
    PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
	{
        double memoryUsageMB = pmc.WorkingSetSize / (1024.0 * 1024.0);
        return FormatFloat("0.000 MB", memoryUsageMB);
	}
	else
	{
        return "N/A";
    }
}

UnicodeString GetProcessorArchitecture(SYSTEM_INFO &systemInfo)
{
	switch (systemInfo.wProcessorArchitecture)
    {
		case PROCESSOR_ARCHITECTURE_INTEL:
            return L"Intel (x86)";
        case PROCESSOR_ARCHITECTURE_AMD64:
			return L"AMD64 (x64)";
        case PROCESSOR_ARCHITECTURE_ARM:
			return L"ARM";
		case PROCESSOR_ARCHITECTURE_IA64:
			return L"IA64";
        default:
			return L"Unknown";
	}
}
