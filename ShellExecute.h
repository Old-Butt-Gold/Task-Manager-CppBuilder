//---------------------------------------------------------------------------

#ifndef ShellExecuteH
#define ShellExecuteH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Imaging.pngimage.hpp>
//---------------------------------------------------------------------------
class TExecuteForm : public TForm
{
__published:	// IDE-managed Components
	TImage *ExecuteImage;
	TLabel *InfoLabel;
	TLabel *OpenLabel;
	TEdit *ExecuteEdit;
	TCheckBox *AdminBox;
	TButton *ExecuteBtn;
	TButton *CloseBtn;
	TButton *TakePathBtn;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall CloseBtnClick(TObject *Sender);
	void __fastcall ExecuteBtnClick(TObject *Sender);
	void __fastcall TakePathBtnClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TExecuteForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TExecuteForm *ExecuteForm;
//---------------------------------------------------------------------------
#endif
