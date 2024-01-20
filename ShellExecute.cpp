#include <vcl.h>
#pragma hdrstop
#include "ShellExecute.h"
#pragma package(smart_init)
#pragma resource "*.dfm"
TExecuteForm* ExecuteForm;
__fastcall TExecuteForm::TExecuteForm(TComponent* Owner) : TForm(Owner) {}
void __fastcall TExecuteForm::FormCreate(TObject* Sender)
{
	InfoLabel->Caption = L"Введите имя программы, папки, документа или \nресурса интернета, которые требуется открыть.";
}

void __fastcall TExecuteForm::CloseBtnClick(TObject* Sender)
{
	ExecuteForm->Close();
}

void __fastcall Create(UnicodeString filePath)
{
	STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
	if (CreateProcess(nullptr, filePath.c_str(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
    {
		CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
	} 
	else 
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND) 
			ShowMessage(UnicodeString("Файл не найден: ") + filePath);
		else if (GetLastError() == ERROR_PATH_NOT_FOUND)
			ShowMessage(UnicodeString("Путь не найден: ") + filePath);
		else if (GetLastError() == ERROR_ACCESS_DENIED)
			ShowMessage(UnicodeString("Доступ запрещен: ") + filePath);
		else
			ShowMessage(UnicodeString("Не удалось запустить процесс. Код ошибки: ") + GetLastError());
	}
}

void __fastcall CreateAdmin(UnicodeString filePath)
{
	UnicodeString parameters = L"";
	SHELLEXECUTEINFO shExecInfo = {0};
	shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	shExecInfo.lpFile = filePath.c_str();
	shExecInfo.lpVerb = L"runas";
	shExecInfo.lpParameters = parameters.c_str();
	shExecInfo.nShow = SW_SHOWNORMAL;
	if (ShellExecuteEx(&shExecInfo))
	{
		WaitForSingleObject(shExecInfo.hProcess, INFINITE);
		CloseHandle(shExecInfo.hProcess);
	}
	else
		ShowMessage(UnicodeString("Не удалось запустить процесс с правами администратора. Код ошибки: ") + GetLastError());
}

void __fastcall TExecuteForm::ExecuteBtnClick(TObject* Sender)
{
	UnicodeString filePath = ExecuteEdit->Text;
	bool runAsAdmin = AdminBox->Checked;
	if (!runAsAdmin) 
	{
		Create(filePath);
		ExecuteForm->Close();

	} 
	else 
	{
		CreateAdmin(filePath);
        ExecuteForm->Close();
	}
}

void __fastcall TExecuteForm::TakePathBtnClick(TObject *Sender)
{
	TOpenDialog *openDialog = new TOpenDialog(this);
    openDialog->Filter = L"Все файлы (*.*)|*.*|Исполняемые файлы (*.exe, *.bat, *.com, *.pif, *.cmd)|*.exe;*.bat;*.com;*.pif;*.cmd";
	openDialog->Options = TOpenOptions() << ofFileMustExist;
    if (openDialog->Execute())
    {
        UnicodeString filePath = openDialog->FileName;
		bool runAsAdmin = AdminBox->Checked; // Получаем выбор пользователя
        if (!runAsAdmin)
        {
			Create(filePath);
			ExecuteForm->Close();
        }
        else
        {
			CreateAdmin(filePath);
			ExecuteForm->Close();
        }
    }
    delete openDialog;
}
