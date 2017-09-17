//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include <stdio.h>
#include <string.h>
#include "mycalc.h"
#include "subr.h"
//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TForm1 *Form1;
MyCalc clc;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Edit1OnChange(TObject *Sender)
{
    AnsLabel->Text = Edit1->Text;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Edit1OnKeyPress(TObject *Sender, char &Key)
{
    if (Key == 0x0d && Edit1->Text.Length()) { // CR ƒL[‚¾‚Á‚½‚ç‚Î
    	clc.Expr(Edit1->Text.Trim().c_str());
    	if (clc.errmsg == NULL)
    	    AnsLabel->Text = asprintf("%18f (0x%08x)", clc.Val, (int)clc.Val);
    	else
    	    AnsLabel->Text = clc.errmsg;
    	Edit1->Text = "";
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormPaint(TObject *Sender)
{
    Edit1->Width = Form1->ClientWidth - Edit1->Left;
    if (Edit1->Width < 128) {
    	Edit1->Width = 128;
    }
    AnsLabel->Width = Edit1->Width;
}
//---------------------------------------------------------------------------
