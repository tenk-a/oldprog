#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <signal.h>
#include <io.h>
#include "def.h"
#include "rpal.h"
#include "vv.h"
#include "key.h"
#include "text.h"
#include "pim.h"

/*---------------------------------------------------------------------------*/

const int FX0=8,FY0=4,XSZ=64,YSZ=15,FCOL=0x05;

static int gPMxsz,gPMysz,gPMx0,gPMy0,gPMpln,gPMasp1,gPMasp2;
static char far *gPMfname, far *gPMsaver, far *gPMartist, far *gPMcmt;

void PriMsgSet(char far *fname,int xsz,int ysz, int x0,int y0, int pln,
				int asp1,int asp2, char far *saver,
				char far *artist, char far *cmt)
{
	gPMxsz = xsz;
	gPMysz = ysz;
	gPMx0 = x0;
	gPMy0 = y0;
	gPMpln = pln;
	gPMasp1 = asp1;
	gPMasp2 = asp2;
	gPMfname = fname;
	gPMsaver = saver;
	gPMartist = artist;
	gPMcmt = cmt;
}

void PriCmt(void)
{
	printf("%Fs : %Fs %Fs\n", FIL_BaseNameFar(gPMfname),gPMsaver, gPMartist);
	printf("%Fs\n",gPMcmt);
}

int PriDoc(void)
{
	BYTE name[260];
	int i;
  #if 1
	char *p;
	int hdl;

	for(i=0;i<260;i++)
		name[i] = gPMfname[i];
	name[259]=0;
	FIL_ChgExt(name,"DOC");

	if ((hdl = _open(name,0)) < 0)
		return 0;
	FIL_GetLt(hdl);
	Text_Cls();
	Text_Sw(1);
	for (i=23; ;) {
		p = FIL_GetLt(-1);
		if (p == NULL)
			break;
		puts(p);
		if (--i == 0) {
			i = 23;
			puts("[more]");
			Key_Wait();
			puts("\b\b\b\b\b\b      \b\b\b\b\b\b");
		}
	}
	Key_Wait();
	Text_Cls();
	_close(hdl);
  #else
	BYTE buf[440];
	char *p,*b;
	int hdl;
	int l;

	for(i=0;i<260;i++) {
		name[i] = gPMfname[i];
	}
	name[259]=0;
	FIL_ChgExt(name,"DOC");

	if ((hdl = _open(name,0)) < 0) {
		return 0;
	}
	Text_Cls();
	Text_Sw(1);
	buf[0] = 0;
	for (b = buf, l = 0, i=23; ;) {
		if (b[0] == 0) {
			b = buf;
			memset(buf,0,440);
			if (_read(hdl,buf+l,410-l) == 0) {
				if (l)
					puts(buf);
				break;
			}
		}
		if ((p = strchr(b,'\r')) != NULL) {
			*p++ = '\0';
			if (*p == '\n')
				*p++ = '\0';
			puts(b);
			b = p;
			l = 0;
		} else if ((p = strchr(b,'\n')) != NULL) {
			*p++ = '\0';
			puts(b);
			b = p;
			l = 0;
		} else {
			l = strlen(b);
			memcpy(buf,b,l);
			b = buf+l;
			b[0] = 0;
		}
		if (--i == 0) {
			i = 23;
			printf("[more]");
			Key_Wait();
			printf("\b\b\b\b\b\b      \b\b\b\b\b\b");
		}
	}
	Key_Wait();
	Text_Cls();
	_close(hdl);
  #endif
	return 1;
}


void PriHlp(int tone,int sttFlg)
{
	static char *gKeyHlpMsg[] = {
		"ESC        終了",
		"HELP ?     説明(ﾄｰﾝ変更有)",
		"TAB =      説明(  〃 　無)",
		"CR SPC     次の画像へ",
		"↑↓←→   ズリズリ",
		"ｼﾌﾄ+ｶｰｿﾙ   ズリズリ(早?)",
		"INS        トーン50％<->100％",
		"ROLL UP/DW トーン変更(0～200%)",
		"[          上下反転",
		"]          左右反転",

		"DEL        コメント表示",
		"HOME/CLR   始点(0,0)にして再表示",
		"-          再表示",
		"/          ループ・モード切替",
		"K          ｽｸﾛｰﾙでWAIT挿入/削除",
		"D          ドキュメント表示",
		"Y          1/16縮小再読込",
		"T          1/4縮小再読込",
		"G          普通に再読込",
		"B          4倍拡大再読込",
		//"M          256色モード変更",
		""
	};
	static char buf[300];
	int i;

	//Text_Cls();
	Text_Color(FCOL);
	Text_BoxLine(FX0-1,FY0-1,FX0+XSZ+1-1,FY0+YSZ+1-1,0,FCOL);
	//Text_Locate(FX0-1,4+4);		Text_PutChr(0x93/*�ｾ*/);
	//Text_Locate((FX0+XSZ-1)+1,4+4);Text_PutChr(0x92/*�ﾆ*/);
	Text_Box(FX0,FY0+4,(FX0+XSZ-1),FY0+4, 0x95/*�｢*/,FCOL);
	Text_PutXYCS(FX0,FY0,6,"ﾌｧｲﾙ:");Text_PutCStr(7,FIL_BaseNameFar(gPMfname));
	Text_PutXYCS(FX0+5+20,FY0,6,"PATH=");
		FIL_DirName(gPMfname, buf);
		for (i = 0; i < 32; i++) {
			if (buf[i] == 0) {
				break;
			}
		}
		if (buf[i]) {//…
			buf[i++] = 0x81;
			buf[i++] = 0x63;
		}
		buf[i] = 0;
		Text_PutCStr(7,buf);
	Text_PutXYCS(FX0+0,FY0+1,6,"作者:");	Text_PutCStr(7,gPMartist);
	sprintf(buf,"[%4Fs]",gPMsaver); Text_PutXYCS(FX0+5+20,FY0+1,6,buf);
	if (gPMpln <= 9)	sprintf(buf,"%3d色",1<<gPMpln);
	else				sprintf(buf,"%2dbit色",gPMpln);
						Text_PutXYCS(FX0+5+20+6,FY0+1,7,buf);
	Text_PutXYCS(FX0+5+20+6+7,FY0+1,6,"縦横比:");
		sprintf(buf,"%d:%d",gPMasp1,gPMasp2); Text_PutCStr(7,buf);
	Text_PutXYCS(FX0+0,FY0+2,6,"size:");
		sprintf(buf,"%d*%d",gPMxsz,gPMysz); Text_PutCStr(7,buf);
	Text_PutXYCS(FX0+5+20,FY0+2,6,"範囲:");
		sprintf(buf,"(%d,%d)-(%d,%d)",gPMx0,gPMy0,gPMx0+gPMxsz-1,gPMy0+gPMysz-1);
		Text_PutCStr(7,buf);
	Text_PutXYCS(FX0+0,FY0+3,6,"ｺﾒﾝﾄ:");
		for (i = 0; i < 64-5-1; i++) {
			buf[i] = gPMcmt[i];
			if (buf[i] == 0||buf[i] == '\n' ||buf[i] == '\r') {
				break;
			}
		}
		buf[i] = 0;
		Text_PutCStr(7,buf);
	Text_PutXYCS(FX0+0,FY0+4,5,"[キー]");
	for (i = 0; i < 10; i++) {
		Text_PutXYCS(FX0+0,FY0+5+i, 4, gKeyHlpMsg[i]);
		Text_PutXYCS(FX0+XSZ/2,FY0+5+i, 4, gKeyHlpMsg[i+10]);
	}
	Text_PutXYCS(FX0+5+20+29,FY0+1,6,"TONE:");
		sprintf(buf,"%3d%%",tone);Text_PutXYCS(FX0+5+20+29+5,FY0+1,7,buf);
	if (sttFlg) {
		Text_PutXYCS(FX0,FY0+YSZ+1,0x07,"始点が(0,0)でない画像はｽﾞﾘｽﾞﾘ不可(HOME/CLRｷｰを使えばできるかも)");
		Text_BoxLine(FX0-1,FY0+YSZ,FX0+XSZ+1-1,FY0+YSZ+2,0,0x02);
	}
	Text_Color(0x07);
}

/*---------------------------------------------------------------------------*/

int KeyLoop(unsigned vvSeg, VVF *gVf, int keyWaitFlg, int tone,
			int loopFlg, int kabeFlg)
	// ret 0:正常終了  -1:ESCアボート  1:1*1再読込 2:4倍再読込 3:1/4再読込
{
	int c,err;
	int sttFlg,txtSw,hlpSw,vwflg;
	int xsclFlg,ysclFlg;
	int nzuriFlg, ntonFlg;	// 0=S,W,MP,21  1=WAB,HF++
	VV far *vv;
	static int yd = 16;
	static int xd = 16;
	static int oofs = 64;
	static BYTE buf[10];

	Text_Cls();
	Text_Sw(1);
	vv = (VV far *)(vvSeg*0x10000L);
	sttFlg = hlpSw = txtSw = err = 0;
	//keyWaitFlg--;
	switch(gVf->no) {
	case N_S:case N_WL:case N_MP:
		ntonFlg = nzuriFlg = 0;
		break;
	case N_WAB:
		ntonFlg = 1;
		nzuriFlg = (loopFlg) ? 1 : -1;
		break;
	case N_21:
	  #ifdef WAB21
		ntonFlg = 1;
	  #else
		ntonFlg = 0;
	  #endif
		nzuriFlg = (loopFlg) ? 1 : -1;
		break;
	case N_HF:
		ntonFlg = 1;
		//nzuriFlg = 0;/*(loopFlg) ? 1 : -1;*/
		nzuriFlg = (loopFlg) ? 1 : -1;
		break;
	default://case N_HF:case N_SF:case N_HFB:
		ntonFlg = nzuriFlg = 1;
	}
	
	if (kabeFlg) {
		kabeFlg = (loopFlg) ? 1 : -1;
	}
	gVf->SclLoopMode(vvSeg, loopFlg);
	if ((vv->xstart || vv->ystart)
		/*&& (vv->xgsize>=vv->xgscrn||vv->ygsize>=vv->ygscrn)*/) {
		sttFlg++;
	}
	xsclFlg = ysclFlg = 0;
	if (vv->xvsize < vv->xgscrn) {
		xsclFlg = 1;
	}
	if (vv->yvsize < vv->ygscrn) {
		ysclFlg = 1;
	}
	for (;;) {
		if (hlpSw) {
			sprintf(buf,"%3d%%",tone);
			Text_PutXYCS(FX0+5+20+29+5,FY0+1,7,buf);
			if (loopFlg) {
				Text_PutXYCS(FX0+5+20+28,FY0+2,3,"[LOOPﾓｰﾄﾞ]");
			} else {
				Text_PutXYCS(FX0+5+20+28,FY0+2,7,"          ");
			}
		}
		if (nzuriFlg <= 0) {
		  KLOOP:
			Key_BufClr();
			c = Key_Shift();
			if (c & 0x0001) {
				vwflg = 0;
				yd = 64*2;
				xd = 64*2;
			} else {
				vwflg = keyWaitFlg;
				yd = 16;
				xd = 16;
			}
			c = Key_Sence(0x07);
			if (sttFlg) {
				c = 0;
			}
			if (c & 0x04 && ysclFlg == 0) { //UP
				if (vwflg)
					WAIT_VSYNC();
				gVf->SclUp(vvSeg, yd);
				goto KLOOP;//continue;
			} else if (c & 0x20 && ysclFlg == 0) { //DOWN
				if (vwflg)
					WAIT_VSYNC();
				gVf->SclDw(vvSeg, yd);
				goto KLOOP;//continue;
			} else if (c & 0x10 && xsclFlg == 0) { //Right
				if (vwflg)
					WAIT_VSYNC();
				gVf->SclRig(vvSeg, xd);
				goto KLOOP;//continue;
			} else if (c & 0x08 && xsclFlg == 0) { //Left
				if (vwflg)
					WAIT_VSYNC();
				gVf->SclLft(vvSeg, xd);
				goto KLOOP;//continue;
			}
		  #if 1
			c = Key_Wait();
		  #else
			c = Key_Scan();
			if (c == -1) {
				goto KLOOP;
				//continue;
			}
		  #endif
		} else {
			Key_BufClr();
			c = Key_Wait();
			switch(c >> 8) {
			case 0x3A:	goto B_UP;
			case 0x3B:	goto B_LF;
			case 0x3C:	goto B_RG;
			case 0x3D:	goto B_DW;
			}
		}
		switch (c >> 8) {
		case 0x00://ESC 00:1B	//終了
			err = -1;
			goto RET;
		case 0x1C://CR			//次の画像
		case 0x34://SPC
			err = 0;
			goto RET;
		case 0x15://'Y' 		//1/16で再読み込み
			err = 4;
			goto RET;
		case 0x14://'T' 		//1/4で再読み込み
			err = 3;
			goto RET;
		case 0x21://'G' 		//普通で再読み込み
			err = 1;
			goto RET;
		case 0x2D://'B' 		//4倍にして再読み込み
			err = 2;
			goto RET;
		case 0x0F://TAB 		//ヘルプ(トーン変更無)
			goto HLP02;
		case 0x3F://HELP		//ヘルプ(トーン変更有)
			goto HLP00;
		case 0x36://ROLL UP 	//トーン 1% UP
			if (gVf->no >= N_HF/*N_HF,N_SF,N_HFB*/) continue;
			tone = (tone < 200) ? tone + 1 : 200;
			gVf->SetTone(tone);
			continue;
		case 0x37://ROLL DOWN	//トーン 1% DOWN
			if (gVf->no >= N_HF/*N_HF,N_SF,N_HFB*/) continue;
			tone = (tone > 0) ? tone - 1 : 0;
			gVf->SetTone(tone);
			continue;
		case 0x38://INS 		//トーン 50<=>100 切替
			tone = (tone == 100) ? 50 : 100;
			WAIT_VSYNC();
			gVf->SetTone(tone);
			continue;
		case 0x39://DEL 		//コメント表示
			if (ntonFlg == 0) {
				txtSw = (txtSw == 0) ? 1 : 0;
				hlpSw = 0;
				Text_Cls();
				Text_Sw(1);
				if (txtSw) {
					PriCmt();
				}
			}else {
				gVf->ShowOff();
				Text_Cls();
				Text_Sw(1);
				PriCmt();
				Key_Wait();
				gVf->ShowOn();
			}
			continue;
		case 0x1b://'[' 		//上下反転
			gVf->RevY(vvSeg);
			goto JJJ1;
		case 0x28://']' 		//左右反転
			gVf->RevX(vvSeg);
			goto JJJ1;
		case 0x3E:// HOME/CLR	// 始点(0,0)で再表示
			gVf->GCls();
		  JJJ1:
			sttFlg = 0;
			vv->xvstart = vv->yvstart = 0;
			if (kabeFlg > 0) {
				gVf->PutPxLoop(vvSeg);
			} else {
				gVf->PutPxScrn(vvSeg,0,0);
			}
			if (hlpSw) {
				hlpSw = 0;
				goto HLP02;
			}
			continue;
		case 0x1F:	//D 		//ドキュメント表示
			if (ntonFlg == 0) {
				gVf->SetTone(50);
				if (PriDoc()) {
					txtSw=0;
					hlpSw=0;
					tone = 100;
				}
				gVf->SetTone(tone);
			} else {
				gVf->ShowOff();
				PriDoc();
				gVf->ShowOn();
			}
			continue;
		case 0x24:	//K			//キーWaitの挿入／削除
			keyWaitFlg = (keyWaitFlg == 0);
			continue;
		case 0x2f:	//M			//PC9821,WAB等のモード変更
			err = 20;
			goto RET;
		}

		switch (c&0xff) {
		case '/':				//ループ・モード
			kabeFlg = -kabeFlg;
			loopFlg = (loopFlg) ? 0 : 1;
			if (gVf->no == N_WAB||gVf->no == N_21
				||gVf->no == N_HF || gVf->no == N_SF) {
				nzuriFlg = (loopFlg) ? 1 : -1;
			}
			if (xsclFlg && ysclFlg) {
				loopFlg = 0;
			}
			gVf->SclLoopMode(vvSeg, loopFlg);
			vv->xvstart = vv->yvstart = 0;
			if (kabeFlg > 0) {
				gVf->PutPxLoop(vvSeg);
			} else {
				if (kabeFlg) {
					gVf->GCls();
				}
				gVf->PutPxScrn(vvSeg,0,0);
			}
			break;
		case '6': //
			B_RG:
			if (!xsclFlg) {
				vv->xvstart += oofs;
				if (!loopFlg && vv->xvstart+vv->xgsize >= vv->xvsize) {
					vv->xvstart = vv->xvsize - vv->xgsize;
				}
				if (vv->xvstart >= vv->xvsize) {
					vv->xvstart=/*(loopFlg==0)?vv->xvsize:*/
							vv->xvstart-vv->xvsize;
				}
			}
			goto CUR_OOFS;
		case '4': //
			B_LF:
			if (!xsclFlg) {
				vv->xvstart -= oofs;
				if (vv->xvstart < 0) {
					vv->xvstart =(loopFlg == 0) ? 0 : vv->xvstart + vv->xvsize;
				}
			}
			goto CUR_OOFS;
		case '8': //
			B_UP:
			if (!ysclFlg) {
				vv->yvstart -= oofs;
				if (vv->yvstart < 0) {
					vv->yvstart =(loopFlg == 0)?0:vv->yvstart + vv->yvsize;
				}
			}
			goto CUR_OOFS;
		case '2': //
			B_DW:
			if (!ysclFlg) {
				vv->yvstart += oofs;
				if (!loopFlg && vv->yvstart+vv->ygsize >= vv->yvsize) {
					vv->yvstart = vv->yvsize - vv->ygsize;
				}
				if (vv->yvstart >= vv->yvsize) {
					vv->yvstart=/*(loopFlg==0)?vv->yvsize:*/
							vv->yvstart-vv->yvsize;
				}
			}
		case '-':				//再表示
		  CUR_OOFS:
			gVf->PutPxScrn(vvSeg,vv->xvstart,vv->yvstart);
			break;
		case '?':				//ヘルプ(トーン変更有)
		  HLP00:
			if (ntonFlg == 0) {
				if (hlpSw == 0) {
					tone = 50;
				} else {
					tone = 100;
				}
				gVf->SetTone(tone);
			}
		case '=':				//ヘルプ(トーン変更無)
		  HLP02:
			Text_Cls();
			if (ntonFlg == 0) {
				hlpSw = (hlpSw) ? 0 : 1;
				txtSw = 0;
				Text_Cls();
				Text_Sw(1);//PC9821対策(T^T)
				if (hlpSw) {
					PriHlp(tone,sttFlg);
				}
			} else {
				gVf->ShowOff();
				Text_Cls();
				Text_Sw(1);//PC9821対策(T^T)
				PriHlp(tone,sttFlg);
				Key_Wait();
				Text_Cls();
				gVf->ShowOn();
			}
			break;
		case '!':	err = 10 + 0;goto RET;
		case '\"':	err = 10 + 1;goto RET;
		case '\#':	err = 10 + 2;goto RET;
		case '$':	err = 10 + 3;goto RET;
		case '%':	err = 10 + 4;goto RET;
		case '&':	err = 10 + 5;goto RET;
		case'\'':	err = 10 + 6;goto RET;
		case '(':	err = 10 + 7;goto RET;
		//case '(':	err = 10 + 8;goto RET;
		}
	}
  RET://終了
	if (hlpSw) {
		Text_Cls();
	}
	//ズリズリでVRAMのアドレスがズレたままなら、直して再表示(HF,SF以外)
	if (gVf->GetSclOfs(vvSeg) && gVf->no < N_HF) {
		gVf->PutPxScrn(vvSeg,vv->xvstart,vv->yvstart);
	}
	Text_Sw(1);
	return err;
}

//////////////////////////////////////////////////////////////////////////
