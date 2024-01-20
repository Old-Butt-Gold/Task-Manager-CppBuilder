#ifndef TaskManagerH
#define TaskManagerH
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.ToolWin.hpp>
#include <Vcl.CustomizeDlg.hpp>
#include <System.Win.TaskbarCore.hpp>
#include <Vcl.Taskbar.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.Tabs.hpp>
#include <Vcl.DockTabSet.hpp>
#include <System.ImageList.hpp>
#include <Vcl.ImgList.hpp>
#include "perfgrap.h"
#include <Vcl.Touch.Keyboard.hpp>
#include "trayicon.h"
#include <Vcl.Grids.hpp>
#include <Vcl.Outline.hpp>
#include <Vcl.Samples.DirOutln.hpp>
class TMainForm : public TForm
{
__published:
	TListView *ListProcesses;
	TLabel *MemLabel;
	TLabel *CountProcessesLabel;
	TLabel *MemAllLabel;
	TLabel *MemFreeLabel;
	TProgressBar *MemLoadBar;
	TPageControl *PageControl;
	TTabSheet *TabSheet;
	TTabSheet *TabSheetGraphic;
	TMainMenu *TaskMainMenu;
	TMenuItem *N2;
	TMenuItem *High;
	TMenuItem *Average;
	TMenuItem *Rarely;
	TMenuItem *N6;
	TMenuItem *None;
	TMenuItem *UpdateBtn;
	TImageList *ImageList1;
	TMenuItem *N1;
	TMenuItem *CreateTask;
	TMenuItem *ExitBtn;
	TPerformanceGraph *CPUGraph;
	TTimer *CPUTimer;
	TLabel *CPUPercentage;
	TProgressBar *CPUBar;
	TLabel *PCTimeInfo;
	TLabel *CPUInfo;
	TTimer *LabelTimer;
	TPerformanceGraph *MemGraph;
	TPopupMenu *ProcessPopupMenu;
	TMenuItem *DeletePopup;
	TMenuItem *N4;
	TMenuItem *DirectoryPopup;
	TMenuItem *SaveInfo;
	TCheckBox *IconsCheckBox;
	TTabSheet *TabSheet1;
	TListView *AutoList;
	TPopupMenu *AutoPopupMenu;
	TMenuItem *OpenAutoPath;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall PopulateProcessList();
	void __fastcall UpdateListInfo();
    void __fastcall SetCheckStates(int num);
	void __fastcall NoneClick(TObject *Sender);
	void __fastcall HighClick(TObject *Sender);
	void __fastcall AverageClick(TObject *Sender);
	void __fastcall RarelyClick(TObject *Sender);
	void __fastcall UpdateBtnClick(TObject *Sender);
	void __fastcall ExitBtnClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall CPUGraphScaleChange(TObject *Sender);
	void __fastcall CPUTimerTimer(TObject *Sender);
	void __fastcall UpdateSystemInfo();
	UnicodeString __fastcall UpdateTime();
	void __fastcall LabelTimerTimer(TObject *Sender);
    void __fastcall PopulateMemoryInformation();
	void __fastcall MemGraphScaleChange(TObject *Sender);
	void __fastcall CreateTaskClick(TObject *Sender);
	void __fastcall ProcessPopupMenuPopup(TObject *Sender);
	void __fastcall DeletePopupClick(TObject *Sender);
	void __fastcall DirectoryPopupClick(TObject *Sender);
	void __fastcall SaveInfoClick(TObject *Sender);
	void __fastcall UpdateProcessesInfo();
	void __fastcall AutoPopupMenuPopup(TObject *Sender);
	void __fastcall OpenAutoPathClick(TObject *Sender);
	void __fastcall ListProcessesColumnClick(TObject *Sender, TListColumn *Column);


private:
public:
	__fastcall TMainForm(TComponent* Owner);
	SYSTEM_INFO SYSINFO;
};
extern PACKAGE TMainForm *MainForm;
#endif
