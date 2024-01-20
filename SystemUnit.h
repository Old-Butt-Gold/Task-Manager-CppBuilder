#ifndef SystemUnitH
#define SystemUnitH

#include <Windows.h>
#include "psapi.h"

UnicodeString GetInfoAboutJob(HANDLE hProcess);
UnicodeString GetPriority(HANDLE hProcess);
UnicodeString GetProcessCreationTime(HANDLE hProcess);
UnicodeString GetProcessPath(HANDLE hProcess);
UnicodeString GetProcessBitness(HANDLE hProcess);
UnicodeString GetCpuUsage(HANDLE hProcess);
UnicodeString GetMemoryUsage(HANDLE hProcess);
UnicodeString GetProcessorArchitecture(SYSTEM_INFO &systemInfo);

#endif

