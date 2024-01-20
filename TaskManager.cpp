#include <vcl.h>
#pragma hdrstop
#include "TaskManager.h"
#include "windows.h"
#include "psapi.h"
#include "string"
#include <TlHelp32.h>
#include <stdio.h>
#include <conio.h>
#include <ShellAPI.h>
#include "SystemUnit.h"
#include "ShellExecute.h"
#pragma package(smart_init)
#pragma link "perfgrap"
#pragma link "trayicon"
#pragma resource "*.dfm"
#include <ShlObj.h>
#include <Registry.hpp>
#include <vector>

#define SystemProcessorTimes 8
#define MAX_PROCESSORS 32

TMainForm* MainForm;

typedef struct _SYSTEM_PROCESSOR_TIMES
{
	ULONGLONG IdleTime;
	ULONGLONG KernelTime;
	ULONGLONG UserTime;
	ULONGLONG DpcTime;
	ULONGLONG InterruptTime;
	ULONG     InterruptCount;
} SYSTEM_PROCESSORS_TIMES[MAX_PROCESSORS];

typedef struct Process_info
{
	bool IsActive;
	UnicodeString Job;
	int ProcessId;
	UnicodeString Priority;
	UnicodeString CreationTime;
	UnicodeString ProcessPath;
	UnicodeString CpuUsage;
	UnicodeString MemUsage;
	UnicodeString Bitness;
	int iconIndex;
} Processes_arr;

int nSortColumn;
bool isAscend;

int processToTerminate;

UnicodeString autoPath;

Process_info* processInfoArray;
int count_of_Processes_arr = 0;

typedef UINT __stdcall (*ZwQuerySystemInformation)(DWORD, void*, DWORD, DWORD*);
ZwQuerySystemInformation func;

unsigned int cpu_usage = 0;
unsigned int mem_load = 0;
HMODULE lib = NULL;
DWORD oldtime;
SYSTEM_PROCESSORS_TIMES CurrentSysProcTimes, PreviousSysProcTimes;

class TProcessUpdateThread : public TThread
{
	public:
		int TimeToSleep = 5000;
		bool Paused;
		__fastcall TProcessUpdateThread(bool CreateSuspended) : TThread(CreateSuspended) { }
	protected:
		void __fastcall Execute()
		{
			while (!Terminated)
			{
				while (Paused)
					Sleep(100);
				Synchronize(&UpdateInfoProcesses);
				Synchronize(&UpdateListView);
				Sleep(TimeToSleep);
			}
		}

		void __fastcall UpdateInfoProcesses()
		{
			MainForm->UpdateProcessesInfo();
		}

		void __fastcall UpdateListView()
		{
			MainForm->UpdateListInfo();
		}
};

auto ProcessUpdateThread = new TProcessUpdateThread(true);

__fastcall TMainForm::TMainForm(TComponent* Owner) : TForm(Owner) {}

void __fastcall TMainForm::PopulateMemoryInformation()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	MemLoadBar->Position = static_cast<unsigned int>(memInfo.dwMemoryLoad);
	MemLabel->Caption = UnicodeString(L"Загрузка RAM: ") + static_cast<unsigned int>(memInfo.dwMemoryLoad) + L"%";
	MemFreeLabel->Caption = UnicodeString(L"Свободно RAM: ") + static_cast<unsigned int>(memInfo.ullAvailPhys / (1024 * 1024)) + L" MB";
	MemAllLabel->Caption = UnicodeString(L"Всего RAM: ") + static_cast<unsigned int>(memInfo.ullTotalPhys / (1024 * 1024)) + L" MB";
	mem_load = memInfo.dwMemoryLoad;
}

int AddIconToImageList(TCHAR* processName) {
	SHFILEINFO shfi;
	ZeroMemory(&shfi, sizeof(shfi));
	if (SHGetFileInfo(processName, 0, &shfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON) != 0) {
		TIcon* icon = new TIcon;
		icon->Handle = shfi.hIcon;
		int iconIndex = MainForm->ImageList1->AddIcon(icon);
		DestroyIcon(shfi.hIcon);
		delete icon;
		return iconIndex;
	}
	return 0;
}

void AddProcessToList(Process_info &item, TCHAR* processName, DWORD processId, HANDLE hProcess)
{
	item.IsActive = true;
	item.ProcessPath = processName;
	item.iconIndex = MainForm->IconsCheckBox->Checked ? AddIconToImageList(processName) : -1;
	item.Job = GetInfoAboutJob(hProcess);
	item.ProcessId = processId;
	item.Priority = GetPriority(hProcess);
	item.CpuUsage = GetCpuUsage(hProcess);
	item.MemUsage =	GetMemoryUsage(hProcess);
	item.CreationTime = GetProcessCreationTime(hProcess);
	item.Bitness = GetProcessBitness(hProcess);
}

void __fastcall TMainForm::PopulateProcessList()
{
	DWORD processes[1024] = {};
	DWORD bytesReturned;
	if (!EnumProcesses(processes, sizeof(processes), &bytesReturned)) {
		ShowMessage("Ошибка при перечислении процессов.");
		return;
	}
	int processCount = bytesReturned / sizeof(DWORD);
	CountProcessesLabel->Caption = L"Количество процессов: " + IntToStr(processCount);
	count_of_Processes_arr = processCount;
	processInfoArray = new Process_info[processCount] {};
	for (int i = 0; i < processCount; i++)
	{
		DWORD processId = processes[i];
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, processId);
		TCHAR processName[MAX_PATH];
		DWORD processNameLength = sizeof(processName) / sizeof(TCHAR);
		if (QueryFullProcessImageName(hProcess, 0, processName, &processNameLength))
		{
			AddProcessToList(processInfoArray[i], processName, processId, hProcess);
			CloseHandle(hProcess);
		}
	}

}

void __fastcall TMainForm::UpdateProcessesInfo()
{
	delete[] processInfoArray;
	ImageList1->Clear();
	PopulateProcessList();
}

void __fastcall TMainForm::UpdateListInfo()
{
	LockWindowUpdate(ListProcesses->Handle);
	int count = ListProcesses->Items->Count;
	ListProcesses->Items->BeginUpdate();
	for (int i = 0; i < count_of_Processes_arr; i++)
	{
		if (processInfoArray[i].IsActive)
		{
			Process_info temp = processInfoArray[i];
			TListItem* listItem = ListProcesses->Items->Add();
			listItem->Caption = ExtractFileName(temp.ProcessPath);
			listItem->ImageIndex = temp.iconIndex;
			listItem->SubItems->Add(temp.Job);
			listItem->SubItems->Add(temp.ProcessId);
			listItem->SubItems->Add(temp.Priority);
			listItem->SubItems->Add(temp.CpuUsage);
			listItem->SubItems->Add(temp.MemUsage);
			listItem->SubItems->Add(temp.CreationTime);
			listItem->SubItems->Add(temp.ProcessPath);
			listItem->SubItems->Add(temp.Bitness);
		}
	}
	for (int i = count; i > -1; i--)
		ListProcesses->Items->Delete(i);
	ListProcesses->Items->EndUpdate();
	LockWindowUpdate(nullptr);
}

UnicodeString __fastcall TMainForm::UpdateTime()
{
	ULONGLONG systemUptime = GetTickCount64();
	ULONGLONG seconds = (systemUptime / 1000) % 60;
	ULONGLONG minutes = (systemUptime / (1000 * 60)) % 60;
	ULONGLONG hours = (systemUptime / (1000 * 60 * 60)) % 24;
	ULONGLONG days = (systemUptime / (1000 * 60 * 60 * 24));
	return L"Время работы компьютера: " + UnicodeString(days) + L" дн. " + UnicodeString(hours) + L" ч. " +
	UnicodeString(minutes) + L" мин. " + UnicodeString(seconds) + L" сек.";
}

void __fastcall TMainForm::UpdateSystemInfo()
{
	CPUInfo->Caption =
	L"Количество ядер: " + UnicodeString(SYSINFO.dwNumberOfProcessors) +
	L" | Тип процессора: " + UnicodeString(SYSINFO.dwProcessorType) +
	L" | Архитектура процессора: " + GetProcessorArchitecture(SYSINFO) +
	L" | OEM ID процессора: " + UnicodeString(SYSINFO.dwOemId) +
	L"\n\nРазмер страницы памяти: " + UnicodeString(SYSINFO.dwPageSize) +
	L" | Активное количество процессоров: " + UnicodeString(SYSINFO.dwNumberOfProcessors) +
	L"\n\nМинимальный адрес памяти: " + IntToHex((int)SYSINFO.lpMinimumApplicationAddress, 8) +
	L"\n\nМаксимальный адрес памяти: " + IntToHex((int)SYSINFO.lpMaximumApplicationAddress, 8);
	PCTimeInfo->Caption = UpdateTime();
}

void EnumerateAutoStartEntries(TListView* AutoList)
{
	TRegistry *reg = new TRegistry;
	reg->RootKey = HKEY_CURRENT_USER;

	if (reg->OpenKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", false))
	{
		TStrings *values = new TStringList;
		reg->GetValueNames(values);

		for (int i = 0; i < values->Count; i++)
		{
			TListItem *listItem = AutoList->Items->Add();
			listItem->Caption = values->Strings[i];
			UnicodeString filePath = reg->ReadString(values->Strings[i]);

			int startPos = filePath.Pos(L"\"");
			int endPos = filePath.LastDelimiter(L"\"");
			if (startPos > 0 && endPos > startPos)
			{
				UnicodeString extractedPath = filePath.SubString(startPos + 1, endPos - startPos - 1);
				listItem->SubItems->Add(extractedPath);
			}
			else
			{
				listItem->SubItems->Add(filePath);
			}

			bool isEnabled = reg->ValueExists(values->Strings[i]);
			listItem->SubItems->Add(isEnabled ? L"Включено" : L"Отключено");
		}

		delete values;
	}

	delete reg;
}

void __fastcall TMainForm::FormCreate(TObject* Sender)
{
	BorderStyle = bsSizeable;
	Caption = L"Task Manager";
	Cursor = crHandPoint;
	ProcessUpdateThread->FreeOnTerminate = true;
	ProcessUpdateThread->Start();
	SetCheckStates(2);
	GetSystemInfo(&SYSINFO);
	UpdateSystemInfo();
	ZeroMemory(&CurrentSysProcTimes[0], sizeof(CurrentSysProcTimes));
	ZeroMemory(&PreviousSysProcTimes[0], sizeof(PreviousSysProcTimes));
	lib = LoadLibrary(L"Ntdll.dll"); //динамическая загрузка библиотеки
	if (!lib)
	{
		Application->MessageBox(L"Error #1: Не удалось динамически загрузить библиотеку!", L"Warning", MB_OK | MB_ICONERROR);
		CPUTimer->Enabled = false;
	}
	func = (ZwQuerySystemInformation)GetProcAddress(lib, "ZwQuerySystemInformation"); //запрос адреса функции
	if (!func)
	{
		Application->MessageBox(L"Error #2: Не удалось получить адрес функции!", L"Warning", MB_OK | MB_ICONERROR);
		FreeLibrary(lib);
		CPUTimer->Enabled = false;
	}
	if (func(SystemProcessorTimes, &PreviousSysProcTimes[0], sizeof(PreviousSysProcTimes), 0) != 0)
	{
		Application->MessageBox(L"Error #3: Ошибка функции!", L"Warning", MB_OK | MB_ICONERROR);
		FreeLibrary(lib);
		CPUTimer->Enabled = false;
	}

	//Добавление в AutoList информации о имени приложения в автозагрузке, пути к нему и состоянии (включено/не включено)
	EnumerateAutoStartEntries(AutoList);
}

void __fastcall TMainForm::SetCheckStates(int num)
{
	None->Checked = None->Tag == num;
	High->Checked = High->Tag == num;
	Average->Checked = Average->Tag == num;
	Rarely->Checked = Rarely->Tag == num;
}

void __fastcall TMainForm::NoneClick(TObject *Sender)
{
	ProcessUpdateThread->Paused = true;
	SetCheckStates(4);
}

void __fastcall TMainForm::HighClick(TObject *Sender)
{
	ProcessUpdateThread->TimeToSleep = 2500;
	ProcessUpdateThread->Paused = false;
	SetCheckStates(1);
}

void __fastcall TMainForm::AverageClick(TObject *Sender)
{
	ProcessUpdateThread->TimeToSleep = 5000;
	ProcessUpdateThread->Paused = false;
	SetCheckStates(2);
}

void __fastcall TMainForm::RarelyClick(TObject *Sender)
{
	ProcessUpdateThread->TimeToSleep = 10000;
	SetCheckStates(3);
}

void __fastcall TMainForm::UpdateBtnClick(TObject *Sender)
{
	UpdateProcessesInfo();
	UpdateListInfo();
}

void __fastcall TMainForm::ExitBtnClick(TObject *Sender)
{
	Close();
}

void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	if (lib)
		FreeLibrary(lib);
	delete[] processInfoArray;
}

void __fastcall TMainForm::CPUGraphScaleChange(TObject *Sender)
{
	CPUGraph->DataPoint(clYellow, cpu_usage);
	CPUGraph->Update();
}

void __fastcall TMainForm::CPUTimerTimer(TObject *Sender)
{
	DWORD nowtime = GetTickCount();
	DWORD pertime = nowtime - oldtime;
	oldtime = nowtime;
	DWORD rez;
	func(SystemProcessorTimes, &CurrentSysProcTimes[0], sizeof(CurrentSysProcTimes), &rez);
	int temp = 0;
	for (int j = 0; j < MAX_PROCESSORS; j++)
		temp += CurrentSysProcTimes[j].IdleTime - PreviousSysProcTimes[j].IdleTime;
	temp /= SYSINFO.dwNumberOfProcessors * 10000; //делим на количество ядер и (наносекунды в миллисекунды)
	temp = (unsigned int)pertime - temp;
	if (CPUTimer->Tag > 0) {
		cpu_usage = temp / (double)pertime * 100;
		if (cpu_usage > 100)
			cpu_usage = 0;
		CPUPercentage->Caption = UnicodeString("Загрузка ЦП: ") + cpu_usage + "%";
		CPUBar->Position = cpu_usage;
		CPUGraph->Scale++;
	}
	CPUTimer->Tag++;
	memcpy(&PreviousSysProcTimes[0], &CurrentSysProcTimes[0], sizeof(PreviousSysProcTimes));
}

void __fastcall TMainForm::LabelTimerTimer(TObject *Sender)
{
	PCTimeInfo->Caption = UpdateTime();
	PopulateMemoryInformation();
	MemGraph->Scale++;
}

void __fastcall TMainForm::MemGraphScaleChange(TObject *Sender)
{
	MemGraph->DataPoint(clYellow, mem_load);
	MemGraph->Update();
}

void __fastcall TMainForm::CreateTaskClick(TObject *Sender)
{
	ExecuteForm->ExecuteEdit->Text = L"cmd";
    ExecuteForm->AdminBox->Checked = false;
	ExecuteForm->Show();
}

void __fastcall TMainForm::ProcessPopupMenuPopup(TObject *Sender)
{
	if (ListProcesses->ItemIndex != -1 && ListProcesses->ItemIndex < ListProcesses->Items->Count)
	{
		UnicodeString processIdString = ListProcesses->Items->Item[ListProcesses->ItemIndex]->SubItems->Strings[1];
		processToTerminate = StrToInt(processIdString);
	}
}

void __fastcall TMainForm::DeletePopupClick(TObject *Sender)
{
	HANDLE hProcessToTerminate = OpenProcess(PROCESS_TERMINATE, FALSE, processToTerminate);
	if (hProcessToTerminate != NULL)
	{
		ShowMessage(TerminateProcess(hProcessToTerminate, 0) ? L"Процесс успешно завершен." : L"Ошибка при завершении процесса.");
		ListProcesses->Items->Delete(ListProcesses->ItemIndex);
		CloseHandle(hProcessToTerminate);
	}
	else
		ShowMessage(L"Ошибка доступа к данному процессу");
}

void __fastcall TMainForm::DirectoryPopupClick(TObject *Sender)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processToTerminate);
	if (hProcess != NULL)
	{
		DWORD dwSize;
		TCHAR processPath[MAX_PATH];
		if (QueryFullProcessImageName(hProcess, 0, processPath, &dwSize))
		{
			UnicodeString directory = ExtractFilePath(processPath);
			ShellExecute(0, L"open", directory.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		CloseHandle(hProcess);
	}
}

void __fastcall TMainForm::SaveInfoClick(TObject *Sender)
{
	TSaveDialog *saveDialog = new TSaveDialog(this);
	saveDialog->Filter = L"CSV Files|*.csv";
	if (saveDialog->Execute())
	{
		UnicodeString fileName = saveDialog->FileName;
		if (ExtractFileExt(fileName).LowerCase() != ".csv")
			fileName = ChangeFileExt(fileName, ".csv");
		TStringList *exportData = new TStringList;
		exportData->Add(L"Process Name, Process ID, Priority, CPU Usage, Memory Usage, Creation Time, Process Path, Process Bitness");
		exportData->Add("\n");
		for (int i = 0; i < ListProcesses->Items->Count; i++)
		{
			TListItem *item = ListProcesses->Items->Item[i];
			UnicodeString processData = item->Caption + L", " +
									  item->SubItems->Strings[1] + L", " +
									  item->SubItems->Strings[2] + L", " +
									  item->SubItems->Strings[3] + L", " +
									  item->SubItems->Strings[4] + L", " +
									  item->SubItems->Strings[5] + L", " +
									  item->SubItems->Strings[6] + L", " +
									  item->SubItems->Strings[7];
			exportData->Add(processData);
			exportData->Add("\n");
		}
		exportData->SaveToFile(fileName);
		delete exportData;
	}
	delete saveDialog;
}

void __fastcall TMainForm::AutoPopupMenuPopup(TObject *Sender)
{
	if (AutoList->ItemIndex != -1 && AutoList->ItemIndex < AutoList->Items->Count)
		autoPath = AutoList->Items->Item[AutoList->ItemIndex]->SubItems->Strings[0];
}

void __fastcall TMainForm::OpenAutoPathClick(TObject *Sender)
{
	ShellExecute(0, L"open", ExtractFilePath(autoPath).c_str(), NULL, NULL, SW_SHOWNORMAL);
}

int __stdcall SortProc(TListItem *Item1, TListItem *Item2, long Para)
{
	UnicodeString Str1, Str2;

	if (Para == 0)
	{
		Str1 = Item1->Caption;
		Str2 = Item2->Caption;
	}
	else if (Para == 2)
	{
		int Val1 = std::stoi(Item1->SubItems->Strings[Para - 1].c_str());
		int Val2 = std::stoi(Item2->SubItems->Strings[Para - 1].c_str());

		return !isAscend ? Val2 - Val1 : Val1 - Val2;
	}
	else if (Para >= 1 && Para <= 8)
	{
		Str1 = Item1->SubItems->Strings[Para - 1];
		Str2 = Item2->SubItems->Strings[Para - 1];
	}

    int result = CompareText(Str1, Str2);

	return !isAscend ? -result : result;
}

void __fastcall TMainForm::ListProcessesColumnClick(TObject *Sender, TListColumn *Column)
{
	if (nSortColumn == Column->Index)
		isAscend = !isAscend;
	else
	{
		nSortColumn = Column->Index;
		isAscend = true;
	}
	ListProcesses->CustomSort((PFNLVCOMPARE)SortProc, Column->Index);
}


