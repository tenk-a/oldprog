#include <stdio.h>
#include "va98.h"

void
main(void)
{
	int c;
	static char *md[] = {"9801","NoMse","Native","Std","98Text","98TextK","98Mono","98Grph"};

    if (Va_ChkVA()) {
		c = Va_GetMseMode();	Va_PutStr(md[c + 2]);	Va_PutStr("\r\n");
		if (Cn_Init())
			printf("va\n");
		else
			printf("bug\n");
		c = Va_GetMseMode();	Va_PutStr(md[c + 2]);	Va_PutStr("\r\n");
		Va_SetMseMode(MSE_NON);
		c = Va_GetMseMode();	Va_PutStr(md[c + 2]);	Va_PutStr("\r\n");
		Va_SetMseMode(MSE_NATIVE);
		c = Va_GetMseMode();	Va_PutStr(md[c + 2]);	Va_PutStr("\r\n");
		Va_SetMseMode(MSE_STD);
		c = Va_GetMseMode();	Va_PutStr(md[c + 2]);	Va_PutStr("\r\n");
		Va_SetMseMode(MSE_TEXT);
		c = Va_GetMseMode();	Va_PutStr(md[c + 2]);	Va_PutStr("\r\n");
		Va_SetMseMode(MSE_TEXTK);
		c = Va_GetMseMode();	Va_PutStr(md[c + 2]);	Va_PutStr("\r\n");
		Va_SetMseMode(MSE_MONO);
		c = Va_GetMseMode();	Va_PutStr(md[c + 2]);	Va_PutStr("\r\n");
		Va_SetMseMode(MSE_GRPH);
		c = Va_GetMseMode();	Va_PutStr(md[c + 2]);	Va_PutStr("\r\n");
    }

	Cn_Init();
	Cn_PutChr('@');
	Cn_PutChr('0');
	Cn_PosXY(2,6);
	Cn_PutStr("abc");
	Cn_PosXY(1,1);
	Cn_Color(2);
	Cn_PutChr(0x824f);
	Cn_PutStrFar("‚ ‚¢‚¤");
	Cn_GetPos();
	Cn_Color(7);
	Cn_PosXY(10,0);
	printf ("(%d,%d)",Cn_x,Cn_y);
	Cn_PosXY(Cn_x,Cn_y);
	Cn_CurOn();
	Cn_KeyInit();
	for (;;) {
	    c = (char)Cn_GetKeyWait();
		if (c == '0')
			Cn_TextOff();
		else if (c == '1')
			Cn_TextOn();
		else if (c == '2')
			Cn_Text20();
		else if (c == '5')
			Cn_Text25();
		else if (c == '8')
			Cn_FuncKeyOff();
		else if (c == '9')
			Cn_FuncKeyOn();
		else if (c == 'Q')
			break;
		else
			Cn_PutChr(c);
	}
	Cn_CurOn();
	Cn_CurBlnkOn();
	Cn_Term();
}
