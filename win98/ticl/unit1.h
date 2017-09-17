//---------------------------------------------------------------------------
#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE 管理のコンポーネント
    TEdit *Edit1;
    TEdit *AnsLabel;
    void __fastcall Edit1OnChange(TObject *Sender);
    void __fastcall Edit1OnKeyPress(TObject *Sender, char &Key);
    void __fastcall FormPaint(TObject *Sender);
private:    // ユーザー宣言
public:     // ユーザー宣言
    __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
