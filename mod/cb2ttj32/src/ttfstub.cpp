#define _DOS
#ifndef NO_MFC
#  include <afx.h>
#  include <afxcoll.h>
#endif
#include <iostream.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "ntuttf.h"
#include "array.h"

#define NFONT 2
#ifdef SJIS
#  define RFONT 	0
#  define CFONT 	8836
# ifdef HANKANA		//îºäpÉJÉiÇí«â¡
#  define EFONT 	(94+64)
# else	// âpêîÇÃÇ›
#  define EFONT		94
# endif
#  define EachFont 	188	//ëSäpÇÃâ∫à ÉoÉCÉgÇÃêÿÇÍñ⁄
#  define EachSeg  	189	//ëSäpÇÃâ∫à ÉoÉCÉgÇÃë±Ç´
#  define EachOff 	(EachSeg-EachFont)
#elif defined ANK
# define RFONT 0
# define CFONT 256
# define EachFont 256
# define EachSeg 257
# define EachOff (EachSeg-EachFont)
# define EFONT 256
#else
# define RFONT 5809
# define CFONT 13502
# define EachFont 157
# define EachSeg 191
# define EachOff (EachSeg-EachFont)
# define EFONT 94		/* Eng translate from ASCII 0x21 to 0x7E total 94 fonts*/
#endif

#define numTbl 13
#define OS2  0
#define CMAP 1
#define GLYF 2
#define HEAD 3
#define HHEA 4
#define HMTX 5
#define LOCA 6
#define MAXP 7
#define NAME 8
#define POST 9
#define MORT 10
#define PREP 11 /* Only set scan type , control and grid */
#define LYJS 12

char *ttfTag[numTbl] = {
     "OS/2", "cmap", "glyf", "head", "hhea", "hmtx", "loca", "maxp", "name",
     "post", "mort", "prep", "LYJs"};

class table: public bigfirst
     {
public:
     table(): bigfirst() { chk = off = len = 0L; }
     unsigned long chksum4();
     void write(FILE *fn);
     unsigned long chk, off, len;
     };

unsigned long table::chksum4()
     {
     int i, c;
     unsigned long l;
     unsigned char __far *t = (unsigned char __far *)getbuf();
     unsigned char __far *eot = t + getlen();
     chk = 0L;
     while (t < eot) {
         for (l = i = 0; i < 4; i++) {
             c = (t < eot)? *t++ : 0;
             l = (l >> 8) + (c << 24);
         }
         chk += l;
     }
     return chk;
     }

void table::write(FILE *fn)
     {
     for (unsigned short i = 0; i < getlen(); i++) putc((*this)[i],fn);
     if ((unsigned long)getlen() < len) //LOCA and HMTX
        for (unsigned long j = (unsigned long)getlen();j < len; j++) putc(0,fn);
     }

extern "C"
     {
     void gene_cmap();
     void gene_basicglyf();
     void glyf_spcfnt(bigfirst &glyf);
     void glyf_engfnt(bigfirst &glyf, short i);
     void gene_fixdata();
     void gene_profile();
     void gene_name();
     void addstr(bigfirst &st, char *str);
     void addstr2(bigfirst &st, char *str);
     void gene_result(FILE *fn);
     void make_gidx(bigfirst &gidx);
     }

table ttfTbl[numTbl];

/*Table OS/2 size 78*/
unsigned char os2_data[78] = {
   0,  0,  2,  0,  1,144,  0,  5,  0,  0,  2,  0,  2,  0,  0,  0,  0,  0, 2,  0,
   2,  0,  0,  0,  2,  0,  0, 51,  1,  4,  0,  5,  2,  1,  6,  9,  0,  1, 1,  1,
   1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,'L','Y',
  'J', 's',  0, 64,  0, 32,255,229,  3, 52,255, 52,  0,204,  3, 52,  0,204};

/*Table mort size 116*/
unsigned char mort_data[116] = {
   0,  1,  0,  0,  0,  0,  0,  1,  0,  0,  0,  1,  0,  0,  0,108,  0,  3,  0,  1,
   0,  3,  0,  0,  0,  0,  0,  1,255,255,255,255,  0,  3,  0,  1,  0,  0,  0,  0,
 255,255,255,254,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 60,128,  4,
   0,  0,  0,  1,  0,  6,  0,  4,  0, 10,  0, 12,  0,  3,  0,  8,  2,158, 55, 81,
   2,169, 55, 82,  2,214, 55, 83,  2,215, 55, 84,  2,216, 55, 85,  2,217, 55, 86,
   2,218, 55, 87,  2,219, 55, 88,  2,220, 55, 89,  2,221, 55, 90};

/* prep program */
unsigned char prep_prog[8] = {
   0xb9, 1, 0xff, 0, 0, // push word 511, 0
   0x8d, /* scan type */ 0x85, //  scan ctrl
   0x18}; //round to grid

/* Eng translate from ASCII 0x21 to 0x7E total 94 fonts*/
#ifdef SJIS
  #ifdef HANKANA		/* the following string is in SJIS! */
unsigned char ptn_2byte[EFONT*2+2] = 
	"ÅIÅhÅîÅêÅìÅïÅfÅiÅjÅñÅ{ÅCÅ|ÅDÅ^ÇOÇPÇQÇRÇSÇTÇUÇVÇWÇXÅFÅGÅÉÅÅÅÑÅH"
	"ÅóÇ`ÇaÇbÇcÇdÇeÇfÇgÇhÇiÇjÇkÇlÇmÇnÇoÇpÇqÇrÇsÇtÇuÇvÇwÇxÇyÅmÅ_ÅnÅOÅQ"
	"ÅeÇÅÇÇÇÉÇÑÇÖÇÜÇáÇàÇâÇäÇãÇåÇçÇéÇèÇêÇëÇíÇìÇîÇïÇñÇóÇòÇôÇöÅoÅbÅpÅP"
	"Å@ÅBÅuÅvÅAÅEÉíÉ@ÉBÉDÉFÉHÉÉÉÖÉáÉbÅ[ÉAÉCÉEÉGÉIÉJÉLÉNÉPÉRÉTÉVÉXÉZÉ\"
	"É^É`ÉcÉeÉgÉiÉjÉkÉlÉmÉnÉqÉtÉwÉzÉ}É~ÉÄÉÅÉÇÉÑÉÜÉàÉâÉäÉãÉåÉçÉèÉìÅJÅK";
  #else					/* the following string is in EUC! */
//unsigned char ptn_2byte[189] =
//"°™°…°Ù°°Û°ı°«° °À°ˆ°‹°§°›°•°ø£∞£±£≤£≥£¥£µ£∂£∑£∏£π°ß°®°„°·°‰°©°˜£¡£¬£√£ƒ£≈£∆£«£»£…£ £À£Ã£Õ£Œ£œ£–£—£“£”£‘£’£÷£◊£ÿ£Ÿ£⁄°Œ°Ô°œ°∞°≤°°£·£‚£„£‰£Â£Ê£Á£Ë£È£Í£Î£Ï£Ì£Ó£Ô££Ò£Ú£Û£Ù£ı£ˆ£˜£¯£˘£˙°–°√°—°±";
unsigned char ptn_2byte[189] =
"ÅIÅhÅîÅêÅìÅïÅfÅiÅjÅñÅ{ÅCÅ|ÅDÅ^ÇOÇPÇQÇRÇSÇTÇUÇVÇWÇXÅFÅGÅÉÅÅÅÑÅHÅóÇ`ÇaÇbÇcÇdÇeÇfÇgÇhÇiÇjÇkÇlÇmÇnÇoÇpÇqÇrÇsÇtÇuÇvÇwÇxÇyÅmÅèÅnÅOÅQÅ@ÇÅÇÇÇÉÇÑÇÖÇÜÇáÇàÇâÇäÇãÇåÇçÇéÇèÇêÇëÇíÇìÇîÇïÇñÇóÇòÇôÇöÅoÅbÅpÅP";
  #endif
#elif defined ANK
;
#else
unsigned char ptn_2byte[189] =
"!I°®°≠"C"H°Æ°¶!]!^°Ø°œ!A°–!D°˛¢Ø¢∞¢±¢≤¢≥¢¥¢µ¢∂¢∑¢∏!G!F°’°◊°÷!H"I¢œ¢–¢—¢“¢”¢‘¢’¢÷¢◊¢ÿ¢Ÿ¢⁄¢€¢‹¢›¢ﬁ¢ﬂ¢‡¢·¢‚¢„¢‰¢Â¢Ê¢Á¢Ë!e"@!f!s°≈°•¢È¢Í¢Î¢Ï¢Ì¢Ó¢Ô¢¢Ò¢Ú¢Û¢Ù¢ı¢ˆ¢˜¢¯¢˘¢˙¢˚¢¸¢˝¢˛#@#A#B#C!a!U!b°„";
#endif

char *id_str, *idc_str;

void main(short argc, char *argv[])
     {
     if (argc < 4)
        { cout << "Usage: TTFSTUB filename idstr chinese_idstr\n"; return; }
     FILE *fn = fopen(argv[1],"wb+");
     if (!fn) { cout << "File open error\n"; return; }
     id_str = argv[2];
     idc_str = argv[3];
     gene_basicglyf();
     gene_cmap();
     gene_fixdata();
     gene_profile();
     gene_name();
     gene_result(fn);
     fclose(fn);
     }

void gene_result(FILE *fn)
     {
     table hob;
     unsigned long chk_adj, sum = 0L;
     short i;
     hob.addlong (0x10000); /* Version */
     hob.addshort(numTbl);
     hob.addshort(128);
     hob.addshort(3);
     hob.addshort(numTbl*16-128);
     ttfTbl[OS2].off = 12 + numTbl*16;
     ttfTbl[CMAP].off = ttfTbl[OS2].off + ttfTbl[OS2].len;
     ttfTbl[HEAD].off = ttfTbl[CMAP].off + ttfTbl[CMAP].len;
     chk_adj = ttfTbl[HEAD].off+8;
     ttfTbl[HHEA].off = ttfTbl[HEAD].off + ttfTbl[HEAD].len;
     ttfTbl[HMTX].off = ttfTbl[HHEA].off + ttfTbl[HHEA].len;
     ttfTbl[LOCA].off = ttfTbl[HMTX].off + ttfTbl[HMTX].len;
     ttfTbl[MAXP].off = ttfTbl[LOCA].off + ttfTbl[LOCA].len;
     ttfTbl[NAME].off = ttfTbl[MAXP].off + ttfTbl[MAXP].len;
     ttfTbl[POST].off = ttfTbl[NAME].off + ttfTbl[NAME].len;
     ttfTbl[PREP].off = ttfTbl[POST].off + ttfTbl[POST].len;
     ttfTbl[MORT].off = ttfTbl[PREP].off + ttfTbl[PREP].len;
     ttfTbl[LYJS].off = ttfTbl[MORT].off + ttfTbl[MORT].len;
     ttfTbl[GLYF].off = ttfTbl[LYJS].off + ttfTbl[LYJS].len;
     for (i = 0; i < numTbl; i++)
         {
         hob.addbyte(ttfTag[i][0]); hob.addbyte(ttfTag[i][1]);
         hob.addbyte(ttfTag[i][2]); hob.addbyte(ttfTag[i][3]);
         hob.addlong(ttfTbl[i].chk); hob.addlong(ttfTbl[i].off);
         hob.addlong(ttfTbl[i].len);
         sum += ttfTbl[i].chk;
         }
     sum += hob.chksum4();
     hob.write(fn);
     ttfTbl[OS2].write(fn);
     ttfTbl[CMAP].write(fn);
     ttfTbl[HEAD].write(fn);
     ttfTbl[HHEA].write(fn);
     ttfTbl[HMTX].write(fn);
     ttfTbl[LOCA].write(fn);
     ttfTbl[MAXP].write(fn);
     ttfTbl[NAME].write(fn);
     ttfTbl[POST].write(fn);
     ttfTbl[PREP].write(fn);
     ttfTbl[MORT].write(fn);
     ttfTbl[LYJS].write(fn);
     ttfTbl[GLYF].write(fn);
     sum = 0xb1b0afba - sum;
     fseek(fn,chk_adj,SEEK_SET);
     fwrite(&sum,1,4,fn);
     }

char *cprite = "National Taiwan University CS&IE", *style = "Regular";
char *version = "FAX(8862)7601847";
char *trade = "Lin, Yaw Jen (LYR) at TPE TW";

void gene_name()
     {
     bigfirst sob;  //Make string packing
     short cp, cpl, ide, idel, st, stl, vs, vsl, td, tdl, idc, idcl;
     short cp2, cp2l, ide2, ide2l, st2, st2l, vs2, vs2l, td2, td2l, i;
     table *tmp = &ttfTbl[NAME];
     cp = sob.getlen();   cpl = strlen(cprite);  cp2l = 2*cpl;
     addstr(sob,cprite); cp2 = sob.getlen();  addstr2(sob,cprite);
     ide = sob.getlen();  idel = strlen(id_str); ide2l = 2*idel;
     addstr(sob,id_str); ide2 = sob.getlen(); addstr2(sob,id_str);
     st = sob.getlen();   stl = strlen(style);   st2l = 2*stl;
     addstr(sob,style); st2 = sob.getlen();  addstr2(sob,style);
     vs = sob.getlen();   vsl = strlen(version); vs2l = 2*vsl;
     addstr(sob,version); vs2 = sob.getlen();  addstr2(sob,version);
     td = sob.getlen();   tdl = strlen(trade);   td2l = 2*tdl;
     addstr(sob,trade); td2 = sob.getlen();  addstr2(sob,trade);
#ifdef SJIS
     {	/* Unicode coversion */
        char tmp[160];
        idc = sob.getlen();
        idcl = j2unicode(idc_str, tmp, 160);
        for (i = 0; i < idcl; i++) {
            sob.addbyte(tmp[i]);
	}
     }
     tmp->addshort(0); tmp->addshort(32); /* #NameRec */
     tmp->addshort(6+32*12); /* offset of strorage */
     /* plateform 1 */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(11); tmp->addshort(0);
     tmp->addshort(cpl); tmp->addshort(cp); /* Copyrite */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(11); tmp->addshort(1);
     tmp->addshort(idel); tmp->addshort(ide); /* IDe */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(11); tmp->addshort(2);
     tmp->addshort(stl); tmp->addshort(st); /* Style */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(11); tmp->addshort(3);
     tmp->addshort(idel); tmp->addshort(ide); /* IDe */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(11); tmp->addshort(4);
     tmp->addshort(idel); tmp->addshort(ide); /* IDe */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(11); tmp->addshort(5);
     tmp->addshort(vsl); tmp->addshort(vs); /* Version */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(11); tmp->addshort(6);
     tmp->addshort(idel); tmp->addshort(ide); /* IDe */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(11); tmp->addshort(7);
     tmp->addshort(tdl); tmp->addshort(td); /* Trade */
     /* plateform 2 */
     tmp->addshort(1); tmp->addshort(1); tmp->addshort(11); tmp->addshort(0);
     tmp->addshort(cp2l); tmp->addshort(cp2); /* Copyrite */
     tmp->addshort(1); tmp->addshort(1); tmp->addshort(11); tmp->addshort(1);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(1); tmp->addshort(1); tmp->addshort(11); tmp->addshort(2);
     tmp->addshort(st2l); tmp->addshort(st2); /* Style */
     tmp->addshort(1); tmp->addshort(1); tmp->addshort(11); tmp->addshort(3);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(1); tmp->addshort(1); tmp->addshort(11); tmp->addshort(4);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(1); tmp->addshort(1); tmp->addshort(11); tmp->addshort(5);
     tmp->addshort(vs2l); tmp->addshort(vs2); /* Version */
     tmp->addshort(1); tmp->addshort(1); tmp->addshort(11); tmp->addshort(6);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(1); tmp->addshort(1); tmp->addshort(11); tmp->addshort(7);
     tmp->addshort(td2l); tmp->addshort(td2); /* Trade */
     /* plateform 3 */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1041); tmp->addshort(0);
     tmp->addshort(cp2l); tmp->addshort(cp2); /* Copyrite */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1041); tmp->addshort(1);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1041); tmp->addshort(2);
     tmp->addshort(st2l); tmp->addshort(st2); /* Style */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1041); tmp->addshort(3);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1041); tmp->addshort(4);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1041); tmp->addshort(5);
     tmp->addshort(vs2l); tmp->addshort(vs2); /* Version */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1041); tmp->addshort(6);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1041); tmp->addshort(7);
     tmp->addshort(td2l); tmp->addshort(td2); /* Trade */
     /* plateform 4 */
     tmp->addshort(3); tmp->addshort(2); tmp->addshort(1041); tmp->addshort(0);
     tmp->addshort(cp2l); tmp->addshort(cp2); /* Copyrite */
     tmp->addshort(3); tmp->addshort(2); tmp->addshort(1041); tmp->addshort(1);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(3); tmp->addshort(2); tmp->addshort(1041); tmp->addshort(2);
     tmp->addshort(st2l); tmp->addshort(st2); /* Style */
     tmp->addshort(3); tmp->addshort(2); tmp->addshort(1041); tmp->addshort(3);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(3); tmp->addshort(2); tmp->addshort(1041); tmp->addshort(4);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(3); tmp->addshort(2); tmp->addshort(1041); tmp->addshort(5);
     tmp->addshort(vs2l); tmp->addshort(vs2); /* Version */
     tmp->addshort(3); tmp->addshort(2); tmp->addshort(1041); tmp->addshort(6);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(3); tmp->addshort(2); tmp->addshort(1041); tmp->addshort(7);
     tmp->addshort(td2l); tmp->addshort(td2); /* Trade */
#elif defined ANK
     {	/* Unicode coversion */
//         char tmp[160];
	idc = sob.getlen();
        int len = strlen(idc_str);
	idcl = len * 2;
        for (i = 0; i < len; i++) {
            sob.addbyte(0);	// high
            sob.addbyte(idc_str[i]); // low
	}
     }
     tmp->addshort(0); tmp->addshort(32); /* #NameRec */
     tmp->addshort(6+32*12); /* offset of strorage */
     /* plateform 1 */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(0); tmp->addshort(0);
     tmp->addshort(cpl); tmp->addshort(cp); /* Copyrite */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(0); tmp->addshort(1);
     tmp->addshort(idel); tmp->addshort(ide); /* IDe */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(0); tmp->addshort(2);
     tmp->addshort(stl); tmp->addshort(st); /* Style */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(0); tmp->addshort(3);
     tmp->addshort(idel); tmp->addshort(ide); /* IDe */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(0); tmp->addshort(4);
     tmp->addshort(idel); tmp->addshort(ide); /* IDe */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(0); tmp->addshort(5);
     tmp->addshort(vsl); tmp->addshort(vs); /* Version */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(0); tmp->addshort(6);
     tmp->addshort(idel); tmp->addshort(ide); /* IDe */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(0); tmp->addshort(7);
     tmp->addshort(tdl); tmp->addshort(td); /* Trade */
     /* plateform 2 */
     tmp->addshort(1); tmp->addshort(1); tmp->addshort(0); tmp->addshort(0);
     tmp->addshort(cp2l); tmp->addshort(cp2); /* Copyrite */
     tmp->addshort(1); tmp->addshort(1); tmp->addshort(0); tmp->addshort(1);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(1); tmp->addshort(1); tmp->addshort(0); tmp->addshort(2);
     tmp->addshort(st2l); tmp->addshort(st2); /* Style */
     tmp->addshort(1); tmp->addshort(1); tmp->addshort(0); tmp->addshort(3);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(1); tmp->addshort(1); tmp->addshort(0); tmp->addshort(4);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(1); tmp->addshort(1); tmp->addshort(0); tmp->addshort(5);
     tmp->addshort(vs2l); tmp->addshort(vs2); /* Version */
     tmp->addshort(1); tmp->addshort(1); tmp->addshort(0); tmp->addshort(6);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(1); tmp->addshort(1); tmp->addshort(0); tmp->addshort(7);
     tmp->addshort(td2l); tmp->addshort(td2); /* Trade */
     /* plateform 3 */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1033); tmp->addshort(0);
     tmp->addshort(cp2l); tmp->addshort(cp2); /* Copyrite */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1033); tmp->addshort(1);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1033); tmp->addshort(2);
     tmp->addshort(st2l); tmp->addshort(st2); /* Style */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1033); tmp->addshort(3);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1033); tmp->addshort(4);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1033); tmp->addshort(5);
     tmp->addshort(vs2l); tmp->addshort(vs2); /* Version */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1033); tmp->addshort(6);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1033); tmp->addshort(7);
     tmp->addshort(td2l); tmp->addshort(td2); /* Trade */
     /* plateform 4 */
     tmp->addshort(3); tmp->addshort(2); tmp->addshort(1033); tmp->addshort(0);
     tmp->addshort(cp2l); tmp->addshort(cp2); /* Copyrite */
     tmp->addshort(3); tmp->addshort(2); tmp->addshort(1033); tmp->addshort(1);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(3); tmp->addshort(2); tmp->addshort(1033); tmp->addshort(2);
     tmp->addshort(st2l); tmp->addshort(st2); /* Style */
     tmp->addshort(3); tmp->addshort(2); tmp->addshort(1033); tmp->addshort(3);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(3); tmp->addshort(2); tmp->addshort(1033); tmp->addshort(4);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(3); tmp->addshort(2); tmp->addshort(1033); tmp->addshort(5);
     tmp->addshort(vs2l); tmp->addshort(vs2); /* Version */
     tmp->addshort(3); tmp->addshort(2); tmp->addshort(1033); tmp->addshort(6);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(3); tmp->addshort(2); tmp->addshort(1033); tmp->addshort(7);
     tmp->addshort(td2l); tmp->addshort(td2); /* Trade */
#else
     idc = sob.getlen();  idcl = strlen(idc_str); addstr(sob,idc_str);
     tmp->addshort(0); tmp->addshort(32); /* #NameRec */
     tmp->addshort(6+32*12); /* offset of strorage */
     /* plateform 1 */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(19); tmp->addshort(0);
     tmp->addshort(cpl); tmp->addshort(cp); /* Copyrite */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(19); tmp->addshort(1);
     tmp->addshort(idel); tmp->addshort(ide); /* IDe */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(19); tmp->addshort(2);
     tmp->addshort(stl); tmp->addshort(st); /* Style */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(19); tmp->addshort(3);
     tmp->addshort(idel); tmp->addshort(ide); /* IDe */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(19); tmp->addshort(4);
     tmp->addshort(idel); tmp->addshort(ide); /* IDe */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(19); tmp->addshort(5);
     tmp->addshort(vsl); tmp->addshort(vs); /* Version */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(19); tmp->addshort(6);
     tmp->addshort(idel); tmp->addshort(ide); /* IDe */
     tmp->addshort(1); tmp->addshort(0); tmp->addshort(19); tmp->addshort(7);
     tmp->addshort(tdl); tmp->addshort(td); /* Trade */
     /* plateform 2 */
     tmp->addshort(1); tmp->addshort(2); tmp->addshort(19); tmp->addshort(0);
     tmp->addshort(cp2l); tmp->addshort(cp2); /* Copyrite */
     tmp->addshort(1); tmp->addshort(2); tmp->addshort(19); tmp->addshort(1);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(1); tmp->addshort(2); tmp->addshort(19); tmp->addshort(2);
     tmp->addshort(st2l); tmp->addshort(st2); /* Style */
     tmp->addshort(1); tmp->addshort(2); tmp->addshort(19); tmp->addshort(3);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(1); tmp->addshort(2); tmp->addshort(19); tmp->addshort(4);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(1); tmp->addshort(2); tmp->addshort(19); tmp->addshort(5);
     tmp->addshort(vs2l); tmp->addshort(vs2); /* Version */
     tmp->addshort(1); tmp->addshort(2); tmp->addshort(19); tmp->addshort(6);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(1); tmp->addshort(2); tmp->addshort(19); tmp->addshort(7);
     tmp->addshort(td2l); tmp->addshort(td2); /* Trade */
     /* plateform 3 */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1028); tmp->addshort(0);
     tmp->addshort(cp2l); tmp->addshort(cp2); /* Copyrite */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1028); tmp->addshort(1);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1028); tmp->addshort(2);
     tmp->addshort(st2l); tmp->addshort(st2); /* Style */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1028); tmp->addshort(3);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1028); tmp->addshort(4);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1028); tmp->addshort(5);
     tmp->addshort(vs2l); tmp->addshort(vs2); /* Version */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1028); tmp->addshort(6);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(3); tmp->addshort(1); tmp->addshort(1028); tmp->addshort(7);
     tmp->addshort(td2l); tmp->addshort(td2); /* Trade */
     /* plateform 4 */
     tmp->addshort(3); tmp->addshort(4); tmp->addshort(1028); tmp->addshort(0);
     tmp->addshort(cp2l); tmp->addshort(cp2); /* Copyrite */
     tmp->addshort(3); tmp->addshort(4); tmp->addshort(1028); tmp->addshort(1);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(3); tmp->addshort(4); tmp->addshort(1028); tmp->addshort(2);
     tmp->addshort(st2l); tmp->addshort(st2); /* Style */
     tmp->addshort(3); tmp->addshort(4); tmp->addshort(1028); tmp->addshort(3);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(3); tmp->addshort(4); tmp->addshort(1028); tmp->addshort(4);
     tmp->addshort(idcl); tmp->addshort(idc); /* IDe */
     tmp->addshort(3); tmp->addshort(4); tmp->addshort(1028); tmp->addshort(5);
     tmp->addshort(vs2l); tmp->addshort(vs2); /* Version */
     tmp->addshort(3); tmp->addshort(4); tmp->addshort(1028); tmp->addshort(6);
     tmp->addshort(ide2l); tmp->addshort(ide2); /* IDe */
     tmp->addshort(3); tmp->addshort(4); tmp->addshort(1028); tmp->addshort(7);
     tmp->addshort(td2l); tmp->addshort(td2); /* Trade */
#endif
     for (i = 0; i < (short)sob.getlen(); i++) tmp->addbyte(sob[i]);
     tmp->chksum4();
     tmp->len = tmp->getlen();
     }

void addstr(bigfirst &st, char *str)
     { while (*str) st.addbyte(*str++); }

void addstr2(bigfirst &st, char *str)
     { while (*str) { st.addbyte('\0'); st.addbyte(*str++); } }

void gene_profile()
     { //gene head, hhea, hmtx, maxp, lyjs and post
     table *tmp = &ttfTbl[HEAD];
     tmp->addlong( 0x10000); 				/* Version */
     tmp->addlong( 0x10000); 				/* Revision */
     tmp->addlong( 0); 						/* Reserved for chk adjust */
     tmp->addlong( 0x5f0f3cf5); 			/* magic # */
     tmp->addshort(3); 						/* flag */
     tmp->addshort(XSIZE); 					/* uPEM */
     tmp->addlong( 0); tmp->addlong( 1993);
     tmp->addlong( 0); tmp->addlong( 1993); /* date */
     tmp->addshort(0);
     tmp->addshort((unsigned short)(-1*DESCENT));
     tmp->addshort(XSIZE);
     tmp->addshort(YSIZE-DESCENT); 			/* xyMinMax */
     tmp->addshort(0); 						/* MacStyle */
     tmp->addshort(25);	 					/* lowestPPEM */
     tmp->addshort(2); 						/* dir */
     tmp->addshort(1); 						/* locFormat, long */
     tmp->addshort(0);
     tmp->chksum4();
     tmp->len = tmp->getlen();

     tmp = &ttfTbl[HHEA];
     tmp->addlong( 0x10000); 						/* Version */
     tmp->addshort(YSIZE-DESCENT); 					/* ascent */
     tmp->addshort((unsigned short)(-1*DESCENT)); 	/* descent */
     tmp->addshort(DESCENT); 						/* line gap */
     tmp->addshort(XSIZE); 							/* advWMax */
     tmp->addshort(0); 								/* minLSB */
     tmp->addshort(0); 								/* minRSB */
     tmp->addshort(XSIZE); 							/* maxExt */
     tmp->addshort(1); 								/* slope */
     tmp->addshort(0);								/* vertical */
     tmp->addshort(0); tmp->addshort(0); tmp->addshort(0);
     tmp->addshort(0); tmp->addshort(0);
     tmp->addshort(0); 								/* curr format */
     tmp->addshort(97); 							/* #longMetrics */
     tmp->chksum4();
     tmp->len = tmp->getlen();
     tmp = &ttfTbl[HMTX];
     tmp->addshort(XSIZE/2); tmp->addshort(0);  	//AdvMax and LSB
     tmp->addshort(XSIZE); tmp->addshort(0);
     for (short i = 0; i < EFONT; i++)
         { tmp->addshort(XSIZE/2); tmp->addshort(0); }
     tmp->addshort(XSIZE); tmp->addshort(0);
     //Follow RFONT+CFONT-1 of zeros reserved
     tmp->chksum4();
     tmp->len = (unsigned long)tmp->getlen()+(unsigned long)(RFONT+CFONT-1)*2L;

     tmp = &ttfTbl[MAXP];
     tmp->addlong( 0x10000); 				/* Version */
     tmp->addshort(RFONT+CFONT+EFONT+2); 	/* #glyf */
     tmp->addshort(800);
     tmp->addshort(40);
     tmp->addshort(800); 					/* for componet */
     tmp->addshort(40);
     tmp->addshort(1); 						/* Zone */
     tmp->addshort(0); 						/* Twilite */
     tmp->addshort(10); 					/* Storage */
     tmp->addshort(0); 						/* FDEFS */
     tmp->addshort(0); 						/* IDEFS */
     tmp->addshort(1024); 					/* stack */
     tmp->addshort(0); 						/* glyf inst */
     tmp->addshort(1); 						/* component elemt */
     tmp->addshort(1); 						/* component depth */
     tmp->chksum4();
     tmp->len = tmp->getlen();

     tmp = &ttfTbl[LYJS];  					//Lin, Y.J. expanded definition
     tmp->addlong( 0x10000); 				/* Version */
     tmp->addshort(1); 						//big5
     tmp->addshort(RFONT+CFONT); 			//# of chinese
     tmp->addshort(0); 						//# of chinese filled
     tmp->addshort(EFONT+2); 				//#glyf of first chinese
     tmp->addlong(0L); 						//Reserved
     tmp->chksum4();
     tmp->len = tmp->getlen();

     tmp = &ttfTbl[POST];
     tmp->addlong( 0x30000); 				/* Version */
     tmp->addlong( 0); 						/* Italic */
     tmp->addshort((unsigned short)(-1*DESCENT/2)); /* underline */
     tmp->addshort(25); 					/* underline thick */
     tmp->addlong( 1); 						/* Fixed */
     tmp->addlong( 10000); 					/* minMemType42 */
     tmp->addlong( 100000); 				/* max */
     tmp->addlong( 10000); 					/* minMemType1 */
     tmp->addlong( 100000); 				/* max */
     tmp->chksum4();
     tmp->len = tmp->getlen();
     }

void gene_fixdata()
     { /* Generate OS/2 MORT and PREP */
     short i;
     for (i = 0; i < 78; i++) ttfTbl[OS2].addbyte(os2_data[i]);
     ttfTbl[OS2].chksum4();
     ttfTbl[OS2].len = i;
     for (i = 0; i < 116; i++) ttfTbl[MORT].addbyte(mort_data[i]);
     ttfTbl[MORT].chksum4();
     ttfTbl[MORT].len = i;
     for (i = 0; i < 8; i++) ttfTbl[PREP].addbyte(prep_prog[i]);
     ttfTbl[PREP].chksum4();
     ttfTbl[PREP].len = i;
     }

struct cmap_sh
     {
     unsigned short fCod, numEnt;
     short idDelta;
     unsigned short idOff;
     };

#ifdef SJIS
void gene_cmap()
     {
     bigfirst fmat0, fmat2, fmat4, glyf_index;
     short i, j, base, fnum, segCnt;
     unsigned short start, *head_ary, *end_ary, *dlt_ary, *off_ary;
     struct cmap_sh *sub_ary;

     /* follow make format 0 */
     fmat0.addshort(0); 		/* format */
     fmat0.addshort(262); 		/* len */
     fmat0.addshort(0); 		/* Revision */
     for (i = 0; i <= 32; i++)	fmat0.addbyte(0);
     for (; i <= 0x7e; i++)		fmat0.addbyte((unsigned char)(i-32+1));
  #ifdef HANKANA
     for (; i < 0xA0; i++)		fmat0.addbyte(0);
     for (; i < 0xE0; i++)		fmat0.addbyte((unsigned char)(i-0xA0+96));
  #endif
     for (; i < 256; i++)		fmat0.addbyte(0);

     /* follow make format 2 */
     head_ary = new unsigned short[256];
     for (i = 0; i < 128; i++)	head_ary[i] = 0; 	/* English */
     for (; i < 0x81; i++)		head_ary[i] = 1; 	/* Ununsed */
     for (j = 2; i <= 0x9f;i++) head_ary[i] = j++;	/* First segment */
   #ifdef HANKANA
     for (; i < 0xe0; i++)		head_ary[i] = 0;	/* îºäpÉJÉi EnglishàµÇ¢ */
   #else
     for (; i < 0xe0; i++)		head_ary[i] = 1;	/* Unused */
   #endif
     for (; i <= 0xef; i++)		head_ary[i] = j++;	/* Sec segment */
     for (; i < 256; i++)		head_ary[i] = 1;	/* Unused */
     for (i = 0; i < 256; i++)	head_ary[i] *= 8;	// sub_aryÇÃóvëfÇ™8ÉoÉCÉgÇ»ÇÃÇ≈
     sub_ary = new struct cmap_sh[j];
     sub_ary[0].fCod =  0; sub_ary[0].numEnt = 256; sub_ary[0].idDelta = 0;
     sub_ary[1].fCod = 64; sub_ary[1].numEnt = sub_ary[1].idDelta = sub_ary[1].idOff = 0;
     /* 0x8140 -- 0x9ffc */
     for (fnum = EachFont*31, base = 0, i = 2;
         fnum > 0;
         i++,base += ((fnum > EachFont)?EachFont:fnum), fnum -= EachFont)
         {
         sub_ary[i].fCod   = 64;
         sub_ary[i].numEnt = (fnum > EachFont)?EachSeg:((fnum <= 63)?fnum:(fnum+EachOff));	//Ç±ÇÃè„à ÉoÉCÉgÇ…É}ÉbÉvÇ≥ÇÍÇƒÇÈï∂éöÇÃêî
         sub_ary[i].idDelta = base;
         }
     /* 0xe040 -- 0xeffc */
     for (fnum = EachFont*16, base = EachFont*31;
         fnum > 0;
         i++,base += ((fnum > EachFont)?EachFont:fnum), fnum -= EachFont)
         {
         sub_ary[i].fCod    = 64;
         sub_ary[i].numEnt  = (fnum > EachFont)?EachSeg:((fnum <= 63)?fnum:(fnum+EachOff));
         sub_ary[i].idDelta = base;
         }
     sub_ary[j-1].idOff = 2;
     for (i = j-2; i >= 2; i--) sub_ary[i].idOff = sub_ary[i+1].idOff + 8;
     sub_ary[0].idOff = (j-1)*8+2+EachSeg*2;
     make_gidx(glyf_index);

     fmat2.addshort(2); /* format */
     fmat2.addshort(6+256*2+j*8+glyf_index.getlen()); /* len */
     fmat2.addshort(0);
     for (i = 0; i < 256; i++) fmat2.addshort(head_ary[i]);
     delete head_ary;
     for (i = 0; i < j; i++)
         {
         fmat2.addshort(sub_ary[i].fCod);
         fmat2.addshort(sub_ary[i].numEnt);
         fmat2.addshort(sub_ary[i].idDelta);
         fmat2.addshort(sub_ary[i].idOff);
         }
     delete sub_ary;
     for (i = 0; i < (short)glyf_index.getlen(); i++)
         fmat2.addbyte(glyf_index[i]);

     // follow make format 4
     segCnt = 1 + 31 + 16 + 1;
     head_ary = new unsigned short[segCnt+1];
     end_ary  = new unsigned short[segCnt+1];
     dlt_ary  = new unsigned short[segCnt+1];
     off_ary  = new unsigned short[segCnt+1];
     head_ary[0] = 0;
     end_ary[0]  = 0xff;
     dlt_ary[0]  = 0;
     off_ary[0]  = (EachSeg+segCnt)*2;
     i = 1; /* above is english */
     /* 0x8140 -- 0x9ffc */
     for (base = 0, start = 0x8140, fnum = EachFont*31;
         fnum > 0;
         fnum -= EachFont, start += 256, base += EachFont, i++)
         {
         head_ary[i] = start;
         end_ary[i]  = start + ((fnum > EachFont)?EachSeg:((fnum > 63)?(fnum+EachOff):fnum)) - 1;
         dlt_ary[i]  = base;
         off_ary[i]  = (segCnt-i)*2;
         }
     /* 0xe040 -- 0xeffc */
     for (base = EachFont*31, start = 0xe040, fnum = EachFont*16;
         fnum > 0;
         fnum -= EachFont, start += 256, base += EachFont, i++)
         {
         head_ary[i] = start;
         end_ary[i]  = start + ((fnum > EachFont)?EachSeg:((fnum > 63)?(fnum+EachOff):fnum)) - 1;
         dlt_ary[i]  = base;
         off_ary[i]  = (segCnt-i)*2;
         }
     if (i >= segCnt)
        { cout << "Error at CMAP format4, memory may destroy.\n"; return;}
     head_ary[i] = 0xffff;
     end_ary[i]  = 0xffff;
     dlt_ary[i]  = 1;
     off_ary[i]  = 0;

     fmat4.addshort(4); /* format */
     fmat4.addshort(14+segCnt*8+glyf_index.getlen()); /* len */
     fmat4.addshort(0);
     fmat4.addshort(segCnt*2);
     fmat4.addshort(64); /* segCnt between 32 - 63 */
     fmat4.addshort(5);  /* log_2(64/2) */
     fmat4.addshort(segCnt*2-64);
     for (i = 0; i < segCnt; i++) fmat4.addshort(end_ary[i]);  delete end_ary;
     fmat4.addshort(0);
     for (i = 0; i < segCnt; i++) fmat4.addshort(head_ary[i]); delete head_ary;
     for (i = 0; i < segCnt; i++) fmat4.addshort(dlt_ary[i]);  delete dlt_ary;
     for (i = 0; i < segCnt; i++) fmat4.addshort(off_ary[i]);  delete off_ary;
     for (i = 0; i < (short)glyf_index.getlen(); i++)
         fmat4.addbyte(glyf_index[i]);

     /* follow make cmap */
     table *tmp = &ttfTbl[CMAP];
     tmp->addshort(0); /* Version */
     tmp->addshort(4); /* #tbl */
     /* Table 1 (format 0) */
     tmp->addshort(1); /* Platform ID = 1 -> Macintosh */
     tmp->addshort(0); /* Encoding ID = 0 -> Roman */
     tmp->addlong( 36L);
     /* Table 2 (format 2) */
     tmp->addshort(1); /* Platform ID = 1 -> Macintosh */
     tmp->addshort(1); /* Encoding ID = 1 -> Japanese */
     tmp->addlong( 36L+fmat0.getlen());
     /* Table 3 (format 2) */
     tmp->addshort(3); /* Platform ID = 3 -> Microsoft */
     tmp->addshort(2); /* Encoding ID = 2 -> ShiftJIS */
     tmp->addlong( 36L+fmat0.getlen());
     /* Table 4 (format 4) */
     tmp->addshort(3); /* Platform ID = 3 -> Microsoft */
     tmp->addshort(2); /* Encoding ID = 2 -> ShiftJIS */
     tmp->addlong( 36L+fmat0.getlen()+fmat2.getlen());
     for (i = 0; i < (short)fmat0.getlen(); i++) tmp->addbyte(fmat0[i]);
     for (i = 0; i < (short)fmat2.getlen(); i++) tmp->addbyte(fmat2[i]);
     for (i = 0; i < (short)fmat4.getlen(); i++) tmp->addbyte(fmat4[i]);
     tmp->chksum4();
     tmp->len = tmp->getlen();
     }

void make_gidx(bigfirst &gidx)
     {
     short i, base = EFONT+2;
     // ëSäpâ∫à ÉoÉCÉgÇÃèÛë‘
     for (i = 0; i < 63; i++, base++)   gidx.addshort(base);
     for (i = 63; i < 64; i++)          gidx.addshort(0);	//0x7f
     for (i = 64; i < 189; i++, base++) gidx.addshort(base);

     // ëSäpè„à ÉoÉCÉgÇÃèÛë‘
     for (i = 0; i <= 32; i++)      gidx.addshort( 0);
     for (; i <= 0x7e; i++)         gidx.addshort(i-32+1);
  #ifdef HANKANA
     for (; i < 0xA0; i++)			gidx.addshort(0);
     for (; i < 0xE0; i++)			gidx.addshort(i-0xA0+96);
  #endif
     for (; i < 256; i++)           gidx.addshort(0);
     }

#elif defined ANK
void gene_cmap()
     {
     bigfirst fmat0, fmat4, glyf_index;
     short i, j, base, fnum, segCnt;
     unsigned short start, *head_ary, *end_ary, *dlt_ary, *off_ary;
     struct cmap_sh *sub_ary;
     fmat0.addshort(0); /* format */
     fmat0.addshort(262); /* len */
     fmat0.addshort(0); /* Revision */
     for (i = 0; i < 256; i++) fmat0.addbyte((unsigned char)i);

     // follow make format 4
     segCnt = 1 + 31 + 16 + 1;
     head_ary = new unsigned short[segCnt+1];
     end_ary  = new unsigned short[segCnt+1];
     dlt_ary  = new unsigned short[segCnt+1];
     off_ary  = new unsigned short[segCnt+1];
     head_ary[0] = 0; end_ary[0] = 0xff; dlt_ary[0] = 0;
     off_ary[0] = (EachSeg+segCnt)*2;
     i = 1; /* above is english */
//      for (base = 0, start = 0, fnum = EachFont*1;
//          fnum > 0; fnum -= EachFont, start += 256, base += EachFont, i++)
//          {
//          head_ary[i] = start;
//          end_ary[i] =
//              start + ((fnum > EachFont)?EachSeg:((fnum > 63)?(fnum+EachOff):fnum)) - 1;
//          dlt_ary[i] = base;
//          off_ary[i] = (segCnt-i)*2;
//          }
     if (i >= segCnt)
        { cout << "Error at CMAP format4, memory may destroy.\n"; return;}
     head_ary[i] = end_ary[i] = 0xffff;
     dlt_ary[i] = 1; off_ary[i] = 0;
     fmat4.addshort(4); /* format */
     fmat4.addshort(14+segCnt*8+glyf_index.getlen()); /* len */
     fmat4.addshort(0);
     fmat4.addshort(segCnt*2);
     fmat4.addshort(64); /* segCnt between 32 - 63 */
     fmat4.addshort(5);  /* log_2(64/2) */
     fmat4.addshort(segCnt*2-64);
     for (i = 0; i < segCnt; i++) fmat4.addshort(end_ary[i]); delete end_ary;
     fmat4.addshort(0);
     for (i = 0; i < segCnt; i++) fmat4.addshort(head_ary[i]); delete head_ary;
     for (i = 0; i < segCnt; i++) fmat4.addshort(dlt_ary[i]); delete dlt_ary;
     for (i = 0; i < segCnt; i++) fmat4.addshort(off_ary[i]); delete off_ary;
     for (i = 0; i < (short)glyf_index.getlen(); i++)
         fmat4.addbyte(glyf_index[i]);
     /* follow make cmap */
     table *tmp = &ttfTbl[CMAP];
     tmp->addshort(0); /* Version */
     tmp->addshort(2); /* #tbl */
     /* Table 1 (format 0) */
     tmp->addshort(1); /* Platform ID = 1 -> Macintosh */
     tmp->addshort(0); /* Encoding ID = 0 -> Roman */
     tmp->addlong( 36L);
     /* Table 4 (format 4) */
     tmp->addshort(3); /* Platform ID = 3 -> Microsoft */
     tmp->addshort(2); /* Encoding ID = 2 -> ShiftJIS */
     tmp->addlong( 36L+fmat0.getlen());
     for (i = 0; i < (short)fmat0.getlen(); i++) tmp->addbyte(fmat0[i]);
     for (i = 0; i < (short)fmat4.getlen(); i++) tmp->addbyte(fmat4[i]);
     tmp->chksum4();
     tmp->len = tmp->getlen();
     }

void make_gidx(bigfirst &gidx)
     {
     short i;
     for (i = 0; i < 256; i++)     gidx.addshort(i);
     }
#else /* !SJIS && !ANK */

void gene_cmap()
     {
     bigfirst fmat0, fmat2, fmat4, glyf_index;
     short i, j, base, fnum, segCnt;
     unsigned short start, *head_ary, *end_ary, *dlt_ary, *off_ary;
     struct cmap_sh *sub_ary;
     fmat0.addshort(0); /* format */
     fmat0.addshort(262); /* len */
     fmat0.addshort(0); /* Revision */
     for (i = 0; i <= 32; i++) fmat0.addbyte(0);
     for (; i <= 0x7e; i++) fmat0.addbyte((unsigned char)(i-32+1));
     for (; i < 256; i++) fmat0.addbyte(0);
     /* follow make format 2 */
     head_ary = new unsigned short[256];
     for (i = 0; i < 128; i++) head_ary[i] = 0; /* English */
     for (; i < 0xa1; i++) head_ary[i] = 1; /* Ununsed */
     for (j = 2; i <= 0xc6; i++) head_ary[i] = j++; /* First segment */
     for (; i < 0xc9; i++) head_ary[i] = 1;
     for (; i <= 0xf9; i++) head_ary[i] = j++; /* Sec segment */
     for (; i < 256; i++) head_ary[i] = 1;
     for (i = 0xfa; i <= 0xfe; i++) head_ary[i] = j++;
     for (i = 0x8e; i <= 0xa0; i++) head_ary[i] = j++;
     for (i = 0x81; i <= 0x8d; i++) head_ary[i] = j++; // here j is #subhead
     for (i = 0; i < 256; i++) head_ary[i] *= 8;
     sub_ary = new struct cmap_sh[j];
     sub_ary[0].fCod = 0; sub_ary[0].numEnt = 256; sub_ary[0].idDelta = 0;
     sub_ary[1].fCod = 64;
     sub_ary[1].numEnt = sub_ary[1].idDelta = sub_ary[1].idOff = 0;
     fnum = 408; base = 0;
     for (fnum = 408, base = 13094, i = 2; fnum > 0;
         i++,base += ((fnum > 157)?157:fnum), fnum -= 157)
         {
         sub_ary[i].fCod = 64;
         sub_ary[i].numEnt = (fnum > 157)?191:((fnum <= 63)?fnum:(fnum+191-157));
         sub_ary[i].idDelta = base;
         }
     for (fnum = 5401, base = 0; fnum > 0;
         i++,base += ((fnum > 157)?157:fnum), fnum -= 157)
         {
         sub_ary[i].fCod = 64;
         sub_ary[i].numEnt =
             (fnum > 157)?191:((fnum <= 63)?fnum:(fnum+191-157));
         sub_ary[i].idDelta = base;
         }
     for (fnum = 7652+41; fnum > 0;
         i++,base += ((fnum > 157)?157:fnum), fnum -= 157)
         {
         sub_ary[i].fCod = 64;
         sub_ary[i].numEnt =
             (fnum > 157)?191:((fnum <= 63)?fnum:(fnum+191-157));
         sub_ary[i].idDelta = base;
         }
     for (fnum = 5809; (fnum > 0) && (i < j);
         i++,base += ((fnum > 157)?157:fnum), fnum -= 157)
         { //Reserve for user font
         sub_ary[i].fCod = 64;
         sub_ary[i].numEnt =
             (fnum > 157)?191:((fnum <= 63)?fnum:(fnum+191-157));
         sub_ary[i].idDelta = base;
         }
     sub_ary[j-1].idOff = 2;
     for (i = j-2; i >= 2; i--) sub_ary[i].idOff = sub_ary[i+1].idOff + 8;
     sub_ary[0].idOff = (j-1)*8+2+191*2;
     make_gidx(glyf_index);
     fmat2.addshort(2); /* format */
     fmat2.addshort(6+256*2+j*8+glyf_index.getlen()); /* len */
     fmat2.addshort(0);
     for (i = 0; i < 256; i++) fmat2.addshort(head_ary[i]);
     delete head_ary;
     for (i = 0; i < j; i++)
         {
         fmat2.addshort(sub_ary[i].fCod);
         fmat2.addshort(sub_ary[i].numEnt);
         fmat2.addshort(sub_ary[i].idDelta);
         fmat2.addshort(sub_ary[i].idOff);
         }
     delete sub_ary;
     for (i = 0; i < (short)glyf_index.getlen(); i++)
         fmat2.addbyte(glyf_index[i]);
     // follow make format 4
     segCnt = 89+5809/157;
     head_ary = new unsigned short[segCnt+1];
     end_ary  = new unsigned short[segCnt+1];
     dlt_ary  = new unsigned short[segCnt+1];
     off_ary  = new unsigned short[segCnt+1];
     head_ary[0] = 0; end_ary[0] = 0xff; dlt_ary[0] = 0;
     off_ary[0] = (191+segCnt)*2;
     i = 1; /* above is english */
     for (base = 13502+3768, start = 0x8140, fnum = 5809 - 3768;
         fnum > 0; fnum -= 157, start += 256, base += 157, i++)
         {
         head_ary[i] = start;
         end_ary[i] =
             start + ((fnum > 157)?191:((fnum > 63)?(fnum+191-157):fnum));
         dlt_ary[i] = base;
         off_ary[i] = (segCnt-i)*2;
         }
     for (base = 13502+785, start = 0x8e40, fnum = 2983; fnum > 0;
         fnum -= 157, start += 256, base += 157, i++)
         {
         head_ary[i] = start;
         end_ary[i] =
             start + ((fnum > 157)?191:((fnum > 63)?(fnum+191-157):fnum));
         dlt_ary[i] = base;
         off_ary[i] = (segCnt-i)*2;
         }
     for (base = 13094, start = 0xa140, fnum = 408; fnum > 0;
         fnum -= 157, start += 256, base += 157, i++)
         { /* Spc font */
         head_ary[i] = start;
         end_ary[i] =
             start + ((fnum > 157)?191:((fnum > 63)?(fnum+191-157):fnum));
         dlt_ary[i] = base;
         off_ary[i] = (segCnt-i)*2;
         }
     for (base = 0, start = 0xa440, fnum = 5401; fnum > 0;
         fnum -= 157, start += 256, base += 157, i++)
         { /* Usually font */
         head_ary[i] = start;
         end_ary[i] =
             start + ((fnum > 157)?191:((fnum > 63)?(fnum+191-157):fnum));
         dlt_ary[i] = base;
         off_ary[i] = (segCnt-i)*2;
         }
     for (base = 5401, start = 0xc940, fnum = 7693; fnum > 0;
         fnum -= 157, start += 256, base += 157, i++)
         { /* Usually font */
         head_ary[i] = start;
         end_ary[i] =
             start + ((fnum > 157)?191:((fnum > 63)?(fnum+191-157):fnum));
         dlt_ary[i] = base;
         off_ary[i] = (segCnt-i)*2;
         }
     for (base = 13502, start = 0xfa40, fnum = 785; fnum > 0;
         fnum -= 157, start += 256, base += 157, i++)
         {
         head_ary[i] = start;
         end_ary[i] =
             start + ((fnum > 157)?191:((fnum > 63)?(fnum+191-157):fnum));
         dlt_ary[i] = base;
         off_ary[i] = (segCnt-i)*2;
         }
     if (i >= segCnt)
        { cout << "Error at CMAP format4, memory may destroy.\n"; return;}
     head_ary[i] = end_ary[i] = 0xffff;
     dlt_ary[i] = 1; off_ary[i] = 0;
     fmat4.addshort(4); /* format */
     fmat4.addshort(14+segCnt*8+glyf_index.getlen()); /* len */
     fmat4.addshort(0);
     fmat4.addshort(segCnt*2);
     fmat4.addshort(128); /* segCnt between 89 - 126 */
     fmat4.addshort(6);
     fmat4.addshort(segCnt*2-128);
     for (i = 0; i < segCnt; i++) fmat4.addshort(end_ary[i]); delete end_ary;
     fmat4.addshort(0);
     for (i = 0; i < segCnt; i++) fmat4.addshort(head_ary[i]); delete head_ary;
     for (i = 0; i < segCnt; i++) fmat4.addshort(dlt_ary[i]); delete dlt_ary;
     for (i = 0; i < segCnt; i++) fmat4.addshort(off_ary[i]); delete off_ary;
     for (i = 0; i < (short)glyf_index.getlen(); i++)
         fmat4.addbyte(glyf_index[i]);
     /* follow make cmap */
     table *tmp = &ttfTbl[CMAP];
     tmp->addshort(0); /* Version */
     tmp->addshort(4); /* #tbl */
     tmp->addshort(1);
     tmp->addshort(0);
     tmp->addlong( 36L);
     tmp->addshort(1);
     tmp->addshort(2);
     tmp->addlong( 36L+fmat0.getlen());
     tmp->addshort(3);
     tmp->addshort(4);
     tmp->addlong( 36L+fmat0.getlen());
     tmp->addshort(3);
     tmp->addshort(4);
     tmp->addlong( 36L+fmat0.getlen()+fmat2.getlen());
     for (i = 0; i < (short)fmat0.getlen(); i++) tmp->addbyte(fmat0[i]);
     for (i = 0; i < (short)fmat2.getlen(); i++) tmp->addbyte(fmat2[i]);
     for (i = 0; i < (short)fmat4.getlen(); i++) tmp->addbyte(fmat4[i]);
     tmp->chksum4();
     tmp->len = tmp->getlen();
     }

void make_gidx(bigfirst &gidx)
     {
     short i, base = 96;
     for (i = 0; i < 63; i++, base++) gidx.addshort(base);
     for (i = 0; i < 191-157; i++)          gidx.addshort(0);
     for (i = 63; i < 157; i++, base++)     gidx.addshort(base);
     for (i = 0; i <= 32; i++)      gidx.addshort( 0);
     for (; i <= 0x7e; i++)         gidx.addshort(i-32+1);
     for (; i < 256; i++)           gidx.addshort(0);
     }
#endif

void gene_basicglyf()
     {
     table *loca = &ttfTbl[LOCA], *glyf = &ttfTbl[GLYF];
     // 0
     loca->addlong(0);
     glyf_spcfnt(*glyf);
     // 1
     loca->addlong((long)glyf->getlen());
     glyf_spcfnt(*glyf);
     //
     for (short i = 0; i < EFONT; i++)
         {
         loca->addlong((long)glyf->getlen());
         glyf_engfnt(*glyf,i);
         }
     loca->addlong((long)glyf->getlen());
     glyf->chksum4();
     loca->chksum4();
     glyf->len = glyf->getlen();
     loca->len = (unsigned long)loca->getlen() + \
                 (unsigned long)(CFONT+RFONT+1)*4L;  //Reserve chinese space
     }

void glyf_spcfnt(bigfirst &glyf)
     {
     glyf.addshort(1);				//numberObContours
     glyf.addshort(0);				// xMin
     glyf.addshort(-1*DESCENT);		// yMin
     glyf.addshort(XSIZE/2); 		// xMax	//hmtx define the kerning, not here
     glyf.addshort(YSIZE-DESCENT);	// yMax
     								// ?
     glyf.addshort(0);				// endPtsOfContours[n]
     glyf.addshort(0);				// instructionLength
     glyf.addbyte(0x37); 			// instruction[n] //On curve, both x,y are positive short
     glyf.addbyte(1);				// flag[n]
     glyf.addbyte(1);				// xCoordinates[]
     glyf.addshort(-1); 			// yCoordinates[] //for parent add
     }

void glyf_engfnt(bigfirst &glyf, short i)
     {
     // îºäpï∂éöÇÃèÄîı
     glyf.addshort((unsigned short)-1);		//numberObContours=-1 .. çáê¨GlyphãLèq
     glyf.addshort(0);						//ècÉCÉìÉfÉbÉNÉX
     glyf.addshort(-1*DESCENT);				//xÉIÉvÉZÉbÉg
     glyf.addshort(XSIZE/2); 				//xÉIÉtÉZÉbÉg//hmtx define the kerning, not here
     glyf.addshort(YSIZE-DESCENT);			//yÉIÉtÉZÉbÉg
     glyf.addshort(0x53);					//ïœå`ÉIÉvÉVÉáÉì
#ifdef SJIS
     /* ptn_2byte is in EUC */
     // glyf.addshort(euc(ptn_2byte[i*2], ptn_2byte[i*2+1])+EFONT+2);	//glyf index
     /* ptn_2byte is in SJIS */
     glyf.addshort(sjis(ptn_2byte[i*2], ptn_2byte[i*2+1])+EFONT+2);	//glyf index
#elif defined ANK
     glyf.addshort(i); //glyf index
#else
     glyf.addshort(big5(ptn_2byte[i*2], ptn_2byte[i*2+1])+96); //glyf index
#endif
     glyf.addshort((unsigned short)(-1*(XSIZE/4)));
          //Move right 1/4, reserved for engfnt opt
     glyf.addshort(0);
     glyf.addshort(0x4000); //Scale 1X1, reserved for engfnt opt
     glyf.addshort(0x4000);
     }
