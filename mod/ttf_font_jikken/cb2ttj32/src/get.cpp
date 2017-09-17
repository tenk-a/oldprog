#define	_DOS
#include <afx.h>
#include <iostream.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include "ntuttf.h"

struct ttfTB
     {
     unsigned long chk,	off, len, seat;
     } glyf, loca, lyjs, head;


extern "C"
     {
     void build_ref(FILE *fn);
     void get_table(struct ttfTB *tbl, short i,	FILE *fn);
     }

short index;
unsigned short size;
unsigned long fs;

void main(short	argc, char *argv[])
     {
     if	(argc <	3) { cout << "Usage: GET ttfile idx glyfile \n"; return; }
     FILE *fn =	fopen(argv[1],"rb");
     FILE *fo =	fopen(argv[3],"wb");
     if	(!fn ||	!fo) { cout << "File open error.\n"; return; }
     index = atoi(argv[2]);
     build_ref(fn);
     fseek(fn,fs+glyf.off,SEEK_SET);
     char *tmp = new char[size];
     fread(tmp,1,size,fn);
     fwrite(tmp,1,size,fo);
     delete tmp;
     fclose(fn);
     fclose(fo);
     }

void build_ref(FILE *fn)
     {
     fseek(fn,4L,SEEK_SET);
     short numTbl = get16(fn);
     fseek(fn,12L,SEEK_SET);
     for (short	i = 0; i < numTbl; i++)
    	 {
    	 char tmp[5];
    	 fread(tmp,1,4,fn);
    	 tmp[4]	= 0;
    	 if (!strcmp(tmp,"glyf"))      get_table(&glyf,i,fn);
    	 else if (!strcmp(tmp,"loca")) get_table(&loca,i,fn);
    	 else if (!strcmp(tmp,"LYJs")) get_table(&lyjs,i,fn);
    	 else if (!strcmp(tmp,"head")) get_table(&head,i,fn);
    	 else {	get32(fn); get32(fn); get32(fn); } //skip
    	 }
     fseek(fn,lyjs.off+10L,SEEK_SET);
     index += get16(fn); //start chinese
     fseek(fn,loca.off+(long)index*4L,SEEK_SET);
     fs	= get32(fn);
     unsigned long fend	= get32(fn);
     size = (unsigned short)(fend-fs);
     }

void get_table(struct ttfTB *tbl, short	i, FILE	*fn)
     {
     tbl->chk =	get32(fn);
     tbl->off =	get32(fn);
     tbl->len =	get32(fn);
     tbl->seat = 12L+(long)i*16L;
     }
/*
unsigned long chk_adj =	0L;
short rep_flag = 0;

void update(FILE *fn,unsigned long sum)
     {
     if	(!glyf.seat || !loca.seat || !lyjs.seat	|| !head.seat)
    	{ cout << "Not addable TrueType	font file\n"; return;}
     fseek(fn,lyjs.off+10L,SEEK_SET);
     short sc =	get16(fn); //Start of Chinese
     //Set location
     fseek(fn,loca.off+(long)(sc+index)*4L,SEEK_SET);
     unsigned long old_off = get32(fn);
     fs	= _filelength(_fileno(fn)) - glyf.off;
     fseek(fn,loca.off+(long)(sc+index+1)*4L,SEEK_SET);
     put32(fs,fn); chk_adj += fs*2; loca.chk +=	fs;
     //Set glyf	len
     chk_adj -=	glyf.len;
     glyf.len =	fs;
     chk_adj +=	glyf.len;
     //Set lyjs
     fseek(fn,lyjs.off+8L,SEEK_SET);
     put16(index+1,fn);
     chk_adj +=	2; lyjs.chk++;
     //Save tables
     fseek(fn,head.off+8L,SEEK_SET);
     put32(chk_adj,fn);	//Chksum adjust
     fseek(fn,loca.seat+4L,SEEK_SET);
     put32(loca.chk,fn); put32(loca.off,fn); put32(loca.len,fn);
     fseek(fn,glyf.seat+4L,SEEK_SET);
     put32(glyf.chk,fn); put32(glyf.off,fn); put32(glyf.len,fn);
     fseek(fn,glyf.seat+4L,SEEK_SET);
     put32(glyf.chk,fn); put32(glyf.off,fn); put32(glyf.len,fn);
     }
*/
