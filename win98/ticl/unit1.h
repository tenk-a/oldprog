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
__published:	// IDE �Ǘ��̃R���|�[�l���g
    TEdit *Edit1;
    TEdit *AnsLabel;
    void __fastcall Edit1OnChange(TObject *Sender);
    void __fastcall Edit1OnKeyPress(TObject *Sender, char &Key);
    void __fastcall FormPaint(TObject *Sender);
private:    // ���[�U�[�錾
public:     // ���[�U�[�錾
    __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
