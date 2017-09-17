#define _DOS
#include "stdio.h"
#include "malloc.h"
#include "string.h"
#include "conio.h"
#include "stdlib.h"
#include "io.h"
#include "ntuttf.h"

struct TBL
       {
       char tag[6]; /* 4 for tag, 1 for nul end, 1 for alignment */
       unsigned long off, len, chk;
       };

FILE *fn;
char inbuf[256];
struct TBL *ttf_table;
short numTbl, noLongMetric = -1;
short loc_format = -1; /* -1 unknow 0 short 1 long */
unsigned long chk_adj = 0L; /* potential 0 also */

void read_tags();
void proc_query(char *str);
void dump_cmap();
void dump_cmap_f2(unsigned short len);
void dump_head();
void dump_hhea();
void dump_hmtx();
void dump_loca();
void dump_maxp();
void dump_name();
void dump_post();
void dump_cvt();
void dump_glyf();
void decode(unsigned short len);
void dump_fpgm();
void dump_prep();
void dump_os2();
void get_table(char *str); /* get tag filename */
void chksum_verify();
void dump_lyjs();

void main(short argv, char *argc[])
     {
     if (argv < 2) { printf("Usage: READTTF filename\n"); return; }
     fn = fopen(argc[1],"rb");
     if (fn == NULL) { printf("%s open error!\n",argc[1]); return; }
     read_tags();
     do {
        printf("(cmap head hhea hmtx loca maxp name post cvt glyf fpgm prep lyjs get chk exit os2)\n>");
        gets(inbuf);
        proc_query(inbuf);
        }
     while (stricmp(inbuf,"exit")!=0);
     }

void read_tags()
     {
     short i;
     printf("Version\t:\t%lx\n",get32(fn));
     numTbl = get16(fn);
     printf("numTables\t:\t%d\n",numTbl);
     printf("searchRange\t:\t%d\n",get16(fn));
     printf("entrySelector\t:\t%d\n",get16(fn));
     printf("rangeShift\t:\t%d\n",get16(fn));
     if (!numTbl) return; /* No table, usually error */
     ttf_table = (struct TBL *)calloc(sizeof(struct TBL),numTbl+1);
     if (!ttf_table) { printf("Memory error at Read_Tags\n"); return; }
     for (i = 0; i < numTbl; i++)
         {
         fread(ttf_table[i].tag,1,4,fn);
         ttf_table[i].tag[4] = 0;
         if (ttf_table[i].tag[3] == ' ') ttf_table[i].tag[3] = 0;
         printf("***Tag\t:\t%s\n",ttf_table[i].tag);
         ttf_table[i].chk = get32(fn);
         printf("\tCheckSum:\t%lx\n",ttf_table[i].chk);
         ttf_table[i].off = get32(fn);
         printf("\tOffset:\t%ld\n",ttf_table[i].off);
         ttf_table[i].len = get32(fn);
         printf("\tLength:\t%ld\n",ttf_table[i].len);
         printf("Press anykey\r");
         getch();
         }
     }

void proc_query(char *str)
     {
     while ((*str) && (*str == ' ')) str++; /* Skip space */
     if (!stricmp(str,"exit")) printf("Goodbye!\n");
     else if (!stricmp(str,"cmap")) dump_cmap();
     else if (!stricmp(str,"head")) dump_head();
     else if (!stricmp(str,"hhea")) dump_hhea();
     else if (!stricmp(str,"hmtx")) dump_hmtx();
     else if (!stricmp(str,"loca")) dump_loca();
     else if (!stricmp(str,"maxp")) dump_maxp();
     else if (!stricmp(str,"name")) dump_name();
     else if (!stricmp(str,"post")) dump_post();
     else if (!stricmp(str,"cvt")) dump_cvt();
     else if (!stricmp(str,"glyf")) dump_glyf();
     else if (!stricmp(str,"fpgm")) dump_fpgm();
     else if (!stricmp(str,"prep")) dump_prep();
     else if (!stricmp(str,"lyjs")) dump_lyjs();
     else if (!stricmp(str,"os2")) dump_os2();
     else if (!_strnicmp(str,"get",3)) { while (*str > ' ') str++; get_table(str);}
     else if (!stricmp(str,"chk")) chksum_verify();
     }

void dump_cmap()
     {
     unsigned short j, numEncode, sub_format, len, segCnt;
     short i, k;
     unsigned long *enc_table, start_cmap;
     for (i = 0; (i < numTbl) && (strcmp("cmap",ttf_table[i].tag)); i++);
     if (i >= numTbl) return; /* No found */
     printf("%s\t%ld\t%ld\n",ttf_table[i].tag,ttf_table[i].off,ttf_table[i].len);
     if (fseek(fn,ttf_table[i].off,SEEK_SET)) return; /* can't seek */
     start_cmap = ttf_table[i].off;
     printf("Version\t:\t%x\n",get16(fn));
     numEncode = get16(fn);
     printf("numTable\t:\t%d\n",numEncode);
     enc_table = (unsigned long *)calloc(sizeof(unsigned long),numEncode+1);
     if (!enc_table) { printf("Memory over at dump_cmap!\n"); return; }
     for (i = 0; i < numEncode; i++)
         {
         printf("***PID\t:\t%d\n",get16(fn));
         printf("\tPS encode id:\t%d\n",get16(fn));
         enc_table[i] = get32(fn);
         printf("\toffset:\t%ld\n",enc_table[i]);
         printf("Press anykey\r");
         getch();
         }
     for (i = 0; i < numEncode; i++)
         {
         if (fseek(fn,start_cmap+enc_table[i],SEEK_SET)) return; /* can't seek */
         printf("---Subtable %d at %ld + %ld\n",i,start_cmap,enc_table[i]);
         sub_format = get16(fn);
         printf("\tformat:\t%d\n",sub_format);
         len = get16(fn);
         printf("\tlength:\t%d\n",len);
         printf("\trevision:\t%d\n",get16(fn));
         switch (sub_format)
                {
                case 0: for (j = 0; j < 256; j++) printf("%4u",get8(fn));
                        printf("\n");
                        break;
                case 2: printf("Would you like to parse code-table?(y/n*)");
                        if ((getch() & 0xffdf) == 'Y') dump_cmap_f2(len-6);
                        else printf("\n");
                        break;
                case 4: segCnt = get16(fn);
                        printf("\tsegCount*2:\t%d\n",segCnt);
                        segCnt /= 2;
                        printf("\tsearchRange:\t%d\n",get16(fn));
                        printf("\tentrySelect:\t%d\n",get16(fn));
                        printf("\trangeShift:\t%d\n",get16(fn));
                        printf("Would you like to parse code-table?(y/n*)");
                        if ((getch() & 0xffdf) == 'Y')
                           {
                           printf("End Code for each segment\n");
                           for (j = 0; j < segCnt; j++)
                               {
                               printf("%5x",get16(fn));
                               if ((j % 200) == 199)
                                  { printf("Press anykey\r"); getch(); }
                               }
                           printf("\nPress anykey\r"); getch();
                           printf("Start code for each segment\n");
                           get16(fn); /* A more 0 undocumated here */
                           for (j = 0; j < segCnt; j++)
                               {
                               printf("%5x",get16(fn));
                               if ((j % 200) == 199)
                                  { printf("Press anykey\r"); getch(); }
                               }
                           printf("\nPress anykey\r"); getch();
                           printf("idDelta for each segment\n");
                           for (j = 0; j < segCnt; j++)
                               {
                               printf("%6u",get16(fn));
                               if ((j % 13) == 12) printf("\n");
                               if ((j % 130) == 129)
                                  { printf("Press anykey\r"); getch(); }
                               }
                           printf("\nPress anykey\r"); getch();
                           printf("idOffset for each segment\n");
                           for (j = 0; j < segCnt; j++)
                               {
                               printf("%6u",get16(fn));
                               if ((j % 13) == 12) printf("\n");
                               if ((j % 130) == 129)
                                  { printf("Press anykey\r"); getch(); }
                               }
                           printf("\nPress anykey\r"); getch();
                           printf("Glyph index array\n");
                           for (j = (7+4*segCnt)*2, k = 0; j < len; j+=2, k++)
                               {
                               printf("%6u",get16(fn));
                               if ((k % 13) == 12) printf("\n");
                               if ((k % 130) == 129)
                                  { printf("Press anykey\r"); getch(); }
                               }
                           }
                        printf("\n");
                        break;
                case 6: printf("\t1stCode:\t%d\n",get16(fn));
                        printf("\tentryCount:\t%d\n",get16(fn));
                        printf("\tFollow glyphIndexArray[] unprinted.\n");
                        break;
                default: printf("Unknow format!\n");
                         break;
                }
         printf("Press anykey(ESC brk)\r");
         if (getch() == 27) break;
         }
     free(enc_table);
     }

void dump_cmap_f2(unsigned short len)
     {
     unsigned long curr = ftell(fn);
     unsigned short i, sub_var = 0, tmp;
     printf("\n");
     for (i = 0; i < 256; i++)
         {
         tmp = get16(fn);
         printf("%4d",tmp);
         if (tmp/8 > sub_var) sub_var = tmp/8;
         }
     printf("\nTotal subHeader %d\nPress anykey\r",++sub_var);
     getch();
     for (i = 0; i < sub_var; i++)
         {
         printf("\t%u",get16(fn));
         printf("\t%u",get16(fn));
         printf("\t%d",get16(fn));
         printf("\t%u\n",get16(fn));
         if ((i % 16) == 15) { printf("Press anykey\r"); getch(); }
         }
     printf("Press anykey\r");
     getch();
     len = len/2 - 256 - sub_var*4;
     for (i = 0; i < len; i++)
         {
         printf("%6u",get16(fn));
         if ((i % 13) == 12) printf("\n");
         if ((i % 130) == 129) { printf("Press anykey\r"); getch(); }
         }
     printf("\n");
     }

void dump_head()
     {
     short i;
     for (i = 0; (i < numTbl) && (strcmp("head",ttf_table[i].tag)); i++);
     if (i >= numTbl) return; /* No found */
     printf("%s\t%ld\t%ld\n",ttf_table[i].tag,ttf_table[i].off,ttf_table[i].len);
     if (fseek(fn,ttf_table[i].off,SEEK_SET)) return; /* can't seek */
     printf("Version\t:\t%lx\n",get32(fn));
     printf("FontRevision\t:\t%lx\n",get32(fn));
     chk_adj = get32(fn);
     printf("ChkSum\t:\t%lx\n",chk_adj);
     printf("MagicNo\t:\t%lx\n",get32(fn));
     printf("Flag\t:\t%x\n",get16(fn));
     printf("UnitPerEM\t:\t%d\n",get16(fn));
     printf("CreateDate\t:\t%lx",get32(fn)); printf("%08lx\n",get32(fn));
     /*Use 2 function call force stack pushing order */
     printf("ModifyDate\t:\t%lx",get32(fn)); printf("%08lx\n",get32(fn));
     printf("xMin\t:\t%d\n",get16(fn));
     printf("yMin\t:\t%d\n",get16(fn));
     printf("xMax\t:\t%d\n",get16(fn));
     printf("yMax\t:\t%d\n",get16(fn));
     printf("MacStyle\t:\t%x\n",get16(fn));
     printf("LowestRec\t:\t%u\n",get16(fn));
     printf("FontDir\t:\t%d\n",get16(fn));
     loc_format = get16(fn);
     printf("LocFormat\t:\t%d\n",loc_format);
     printf("GlyphFormat\t:\t%d\n",get16(fn));
     }

void dump_hhea()
     {
     short i;
     for (i = 0; (i < numTbl) && (strcmp("hhea",ttf_table[i].tag)); i++);
     if (i >= numTbl) return; /* No found */
     printf("%s\t%ld\t%ld\n",ttf_table[i].tag,ttf_table[i].off,ttf_table[i].len);
     if (fseek(fn,ttf_table[i].off,SEEK_SET)) return; /* can't seek */
     printf("Version\t:\t%lx\n",get32(fn));
     printf("Ascender\t:\t%d\n",get16(fn));
     printf("Descender\t:\t%d\n",get16(fn));
     printf("LineGap\t:\t%d\n",get16(fn));
     printf("advWidthMax\t:\t%u\n",get16(fn));
     printf("minLSBear\t:\t%d\n",get16(fn));
     printf("minRSBear\t:\t%d\n",get16(fn));
     printf("xMaxExtend\t:\t%d\n",get16(fn));
     printf("caretRise\t:\t%d\n",get16(fn));
     printf("caretRun\t:\t%d\n",get16(fn));
     printf("Reserverd 5 words: ");
     for (i = 0; i < 5; i++) printf("%d ",get16(fn));
     printf("\nmetricDataFormat\t:\t%d\n",get16(fn));
     noLongMetric = get16(fn);
     printf("noLongMetric\t:\t%d\n",noLongMetric);
     }

void dump_hmtx()
     {
     short i;
     unsigned long len, j;
     if (noLongMetric == -1)
        {
        printf("Dump hhea to get noLongMetric!\n");
        dump_hhea();
        printf("Press anykey\r");
        getch();
        }
     for (i = 0; (i < numTbl) && (strcmp("hmtx",ttf_table[i].tag)); i++);
     if (i >= numTbl) return; /* No found */
     printf("%s\t%ld\t%ld\n",ttf_table[i].tag,ttf_table[i].off,ttf_table[i].len);
     if (fseek(fn,ttf_table[i].off,SEEK_SET)) return; /* can't seek */
     len = ttf_table[i].len;
     for (i = 0; i < noLongMetric; i++)
         {
         printf("(%6u ",get16(fn)); printf("%6d) ",get16(fn));
         if ((i % 50) == 49)
            { printf("Press anykey(ESC brk)\r"); if (getch() == 27) break; }
         }
     printf("\nPress anykey\r");
     getch();
     len = (len - noLongMetric*4)/2;
     for (j = 0; j < len; j++)
         {
         printf("%6d",get16(fn));
         if ((j % 13) == 12) printf("\n");
         if ((j % 130) == 129)
            { printf("Press anykey(ESC brk)\r"); if (getch() == 27) break; }
         }
     printf("\n");
     }

void dump_loca()
     {
     short i;
     unsigned long len, j;
     if (loc_format == -1)
        {
        printf("Dump head to get loc_format!\n");
        dump_head();
        printf("Press anykey\r");
        getch();
        }
     for (i = 0; (i < numTbl) && (strcmp("loca",ttf_table[i].tag)); i++);
     if (i >= numTbl) return; /* No found */
     printf("%s\t%ld\t%ld\n",ttf_table[i].tag,ttf_table[i].off,ttf_table[i].len);
     if (fseek(fn,ttf_table[i].off,SEEK_SET)) return; /* can't seek */
     len = ttf_table[i].len/(loc_format?4:2);
     if (loc_format) for (j = 0; j < len; j++)
        { /* long */
        printf("%16lu",get32(fn));
        if ((j % 50) == 49)
           { printf("Press anykey(ESC brk)\r"); if (getch() == 27) break; }
        }
     else for (j = 0; j < len; j++)
          { /* short */
          printf("%6u",get16(fn));
          if ((j % 13) == 12) printf("\n");
          if ((j % 130) == 129)
             { printf("Press anykey(ESC brk)\r"); if (getch() == 27) break; }
          }
     printf("\n");
     }

void dump_maxp()
     {
     short i;
     for (i = 0; (i < numTbl) && (strcmp("maxp",ttf_table[i].tag)); i++);
     if (i >= numTbl) return; /* No found */
     printf("%s\t%ld\t%ld\n",ttf_table[i].tag,ttf_table[i].off,ttf_table[i].len);
     if (fseek(fn,ttf_table[i].off,SEEK_SET)) return; /* can't seek */
     printf("Version\t:\t%lx\n",get32(fn));
     printf("#glyph\t:\t%d\n",get16(fn));
     printf("maxPoints\t:\t%d\n",get16(fn));
     printf("maxContours\t:\t%d\n",get16(fn));
     printf("maxCompPoints\t:\t%d\n",get16(fn));
     printf("maxCompContours\t:\t%d\n",get16(fn));
     printf("maxZones\t:\t%d\n",get16(fn));
     printf("maxTwilitePoint\t:\t%d\n",get16(fn));
     printf("maxStorage\t:\t%d\n",get16(fn));
     printf("maxFDEFs\t:\t%d\n",get16(fn));
     printf("maxIDEFs\t:\t%d\n",get16(fn));
     printf("maxStackElem\t:\t%d\n",get16(fn));
     printf("maxSizeInst\t:\t%d\n",get16(fn));
     printf("maxCompElem\t:\t%d\n",get16(fn));
     printf("maxCompDepth\t:\t%d\n",get16(fn));
     }

void dump_name()
     {
     unsigned long stor_area, curr_off;
     short i, j, numRec, str_off;
     for (i = 0; (i < numTbl) && (strcmp("name",ttf_table[i].tag)); i++);
     if (i >= numTbl) return; /* No found */
     printf("%s\t%ld\t%ld\n",ttf_table[i].tag,ttf_table[i].off,ttf_table[i].len);
     if (fseek(fn,ttf_table[i].off,SEEK_SET)) return; /* can't seek */
     stor_area = ttf_table[i].off;
     printf("Selector\t:\t%d\n",get16(fn));
     numRec = get16(fn);
     printf("#NameRecord\t:\t%d\n",numRec);
     i = get16(fn);
     printf("Off to StoreArea\t:\t%d\n",i);
     stor_area += i;
     for (i = 0; i < numRec; i++)
         {
         printf("---Pid\t:\t%d\n",get16(fn));
         printf("\tPS encode id:\t%d\n",get16(fn));
         printf("\tLanguage id:\t%d\n",get16(fn));
         printf("\tName id:\t%d\n",get16(fn));
         j = get16(fn);
         str_off = get16(fn);
         curr_off = ftell(fn);
         fseek(fn,stor_area+str_off,SEEK_SET);
         while (j--) printf("%c",get8(fn));
         fseek(fn,curr_off,SEEK_SET);
         printf("\nPress anykey\r");
         getch();
         }
     }

void dump_post()
     {
     short i;
     unsigned long format;
     for (i = 0; (i < numTbl) && (strcmp("post",ttf_table[i].tag)); i++);
     if (i >= numTbl) return; /* No found */
     printf("%s\t%ld\t%ld\n",ttf_table[i].tag,ttf_table[i].off,ttf_table[i].len);
     if (fseek(fn,ttf_table[i].off,SEEK_SET)) return; /* can't seek */
     format = get32(fn);
     printf("Version\t:\t%lx\n",format);
     printf("italicAngle\t:\t%ld\n",get32(fn));
     printf("underlinePosition\t:\t%d\n",get16(fn));
     printf("underlineThick\t:\t%d\n",get16(fn));
     printf("isFixedPitch\t:\t%lu\n",get32(fn));
     printf("minMemType42\t:\t%lu\n",get32(fn));
     printf("maxMemType42\t:\t%lu\n",get32(fn));
     printf("minMemType1\t:\t%lu\n",get32(fn));
     printf("maxMemType1\t:\t%lu\n",get32(fn));
     }

void dump_cvt()
     {
     short i;
     unsigned long len, j;
     for (i = 0; (i < numTbl) && (strcmp("cvt",ttf_table[i].tag)); i++);
     if (i >= numTbl) return; /* No found */
     printf("%s\t%ld\t%ld\n",ttf_table[i].tag,ttf_table[i].off,ttf_table[i].len);
     if (fseek(fn,ttf_table[i].off,SEEK_SET)) return; /* can't seek */
     len = ttf_table[i].len/2;
     for (j = 0; j < len; j++)
         {
         printf("%6d",get16(fn));
         if ((j % 13) == 12) printf("\n");
         if ((j % 130) == 129)
            { printf("Press anykey(ESC brk)\r"); if (getch() == 27) break; }
         }
     printf("\n");
     }

struct coor { short flag, xcoor, ycoor; };

void dump_glyf()
     {
     short i, j, k, numContour, numPts, *endContour; /* Simple component var */
     unsigned long glyfData, locaData, loca_len, curr_off;
     unsigned short flag; /* Component glyf var */
     char *str;
     struct coor *tmp;
     for (i = 0; (i < numTbl) && (strcmp("glyf",ttf_table[i].tag)); i++);
     if (i >= numTbl) return; /* No found */
     printf("%s\t%ld\t%ld\n",ttf_table[i].tag,ttf_table[i].off,ttf_table[i].len);
     if (fseek(fn,ttf_table[i].off,SEEK_SET)) return; /* can't seek */
     glyfData = ttf_table[i].off;
     if (loc_format == -1)
        {
        printf("Dump head to get loc_format!\n");
        dump_head();
        printf("Press anykey\r");
        getch();
        }
     for (i = 0; (i < numTbl) && (strcmp("loca",ttf_table[i].tag)); i++);
     if (i >= numTbl) return; /* No found */
     printf("%s\t%ld\t%ld\n",ttf_table[i].tag,ttf_table[i].off,ttf_table[i].len);
     if (fseek(fn,ttf_table[i].off,SEEK_SET)) return; /* can't seek */
     locaData = ttf_table[i].off;
     loca_len = ttf_table[i].len/(loc_format?4:2);
     for (;;)
         {
         printf("(# Q)>");
         gets(inbuf);
         for (str = inbuf; *str == ' '; str++); /* Skip space */
         if ((*str == 'Q') || (*str == 'q')) break;
         i = atoi(str);
         if ((i >= 0) && ((unsigned long)i < loca_len))
            {
            curr_off = (unsigned long)i*(loc_format?4L:2L) + locaData;
            fseek(fn,curr_off,SEEK_SET);
            curr_off = (loc_format?get32(fn):get16(fn)*2) + glyfData;
            printf("Locate\t:\t%lu\n",curr_off);
            fseek(fn,curr_off,SEEK_SET);
            numContour = get16(fn);
            printf("#Contour\t:\t%d\n",numContour);
            printf("xMin\t:\t%d\n",get16(fn));
            printf("yMin\t:\t%d\n",get16(fn));
            printf("xMax\t:\t%d\n",get16(fn));
            printf("yMax\t:\t%d\n",get16(fn));
            printf("Press anykey\r");
            getch();
            if (numContour < 0)
               {
               do {
                  flag = get16(fn);
                  printf("***flag\t:\t%x\n",flag);
                  printf("\tindex:\t%u\n",get16(fn));
                  printf("\targ 1:\t%d\n",(flag & 1)?get16(fn):get8(fn));
                  printf("\targ 2:\t%d\n",(flag & 1)?get16(fn):get8(fn));
                  if (flag & 8) printf("\tScale:\t%f\n",((double)((signed)get16(fn)))/16384.0);
                  else if (flag & 0x40)
                       {
                       printf("\txScale:\t%f\n",((double)((signed)get16(fn)))/16384.0);
                       printf("\tyScale:\t%f\n",((double)((signed)get16(fn)))/16384.0);
                       }
                  else if (flag & 0x80)
                       {
                       printf("\txScale:\t%f\n",((double)((signed)get16(fn)))/16384.0);
                       printf("\tScale01:\t%f\n",((double)((signed)get16(fn)))/16384.0);
                       printf("\tScale10:\t%f\n",((double)((signed)get16(fn)))/16394.0);
                       printf("\tyScale:\t%f\n",((double)((signed)get16(fn)))/16384.0);
                       }
                  printf("Press anykey\r"); getch();
                  } while (flag & 0x20);
               if (flag & 0x100)
                  {
                  i = get16(fn);
                  printf("#instructions\t:\t%d\n",i);
                  decode(i);
                  }
               else printf("%20s\r",""); /* Clear line */
               }
            else {
                 endContour = (short *)calloc(numContour,sizeof(short));
                 if (!endContour)
                    { printf("Memory over at get contour.\n"); return;}
                 for (i = 0; i < numContour; i++) endContour[i] = get16(fn);
                 numPts = endContour[numContour-1];
                 i = get16(fn);
                 printf("#instruction\t:\t%d\n",i);
                 if (i) decode(i);
                 tmp = (struct coor *)calloc(numPts+1,sizeof(struct coor));
                 if (!tmp)
                    { printf("Memory over at dump coordinates.\n"); return;}
                 for (i = 0; i <= numPts; i++) /* 0 based */
                     { /* Read flags */
                     tmp[i].flag = get8(fn);
                     if (tmp[i].flag & 8)
                        { /* Repeat */
                        j = get8(fn);
                        for (k = 0, j &= 255; k < j; k++)
                            tmp[i+k+1].flag = tmp[i].flag;
                        i += k;
                        }
                     }
                 for (i = 0; i <= numPts; i++)
                     {
                     switch (tmp[i].flag & 0x12)
                            {
                            case 0x12: tmp[i].xcoor = get8(fn);
                                       break; /* positive short */
                            case 2: tmp[i].xcoor = get8(fn) * -1;
                                    break;
                            case 0x10: tmp[i].xcoor = 0;
                                       break;
                            case 0: tmp[i].xcoor = get16(fn);
                                    break;
                            }
                     }
                 for (i = 0; i <= numPts; i++)
                     {
                     switch (tmp[i].flag & 0x24)
                            {
                            case 0x24: tmp[i].ycoor = get8(fn);
                                       break; /* positive short */
                            case 4: tmp[i].ycoor = get8(fn) * -1;
                                    break;
                            case 0x20: tmp[i].ycoor = 0;
                                       break;
                            case 0: tmp[i].ycoor = get16(fn);
                                    break;
                            }
                     }
                 for (i = 1; i <= numPts; i++)
                     {
                     tmp[i].xcoor += tmp[i-1].xcoor;
                     tmp[i].ycoor += tmp[i-1].ycoor;
                     }
                 for (i = 0, j = 0; i < numContour; i++)
                     {
                     printf("Contour %d from %d to %d\n",i,j,endContour[i]);
                     for (k = 0; j <= endContour[i]; j++, k++)
                         {
                         printf("(%d %5d %5d) ",tmp[j].flag&1,tmp[j].xcoor,tmp[j].ycoor);
                         if ((k % 50) == 49)
                            {
                            printf("Press anykey(ESC brk)\r");
                            if (getch() == 27) break;
                            }
                         }
                     printf("\n");
                     if (i < numContour - 1)
                        {
                        printf("Press anykey(ESC brk)\r");
                        if (getch() == 27) break;
                        }
                     }
                 free(tmp);
                 free(endContour);
                 }
            }
         }
     }

void decode(unsigned short len)
     {
     unsigned short i;
     short n, j, line;
     unsigned char inst;
     for (i = 0, line = 0; i < len; )
         {
         inst = get8(fn);
         switch (inst)
                {
                case 0x40: n = get8(fn);
                           printf("Push %d bytes\n",n);
                           for (j = 0; j < n; j++)
                               {
                               printf("%4d",get8(fn));
                               if ((j % 200) == 199)
                                  { line = 0; printf("Press anykey\r"); getch(); }
                               }
                           printf("\n");
                           i += n+2; line += (n+19)/20+1; break;
                case 0x41: n = get8(fn);
                           printf("Push %d words\n",n);
                           for (j = 0; j < n; j++)
                               {
                               printf("%6d",get16(fn));
                               if ((j % 13) == 12) printf("\n");
                               if ((j % 130) == 129)
                                  { line = 0; printf("Press anykey\r"); getch(); }
                               }
                           printf("\n");
                           i += n*2+2; line += (n+12)/13+1; break;
                case 0xb7: printf("Push %d\n\t",get8(fn)); i++; line++;
                case 0xb6: printf("Push %d\n\t",get8(fn)); i++; line++;
                case 0xb5: printf("Push %d\n\t",get8(fn)); i++; line++;
                case 0xb4: printf("Push %d\n\t",get8(fn)); i++; line++;
                case 0xb3: printf("Push %d\n\t",get8(fn)); i++; line++;
                case 0xb2: printf("Push %d\n\t",get8(fn)); i++; line++;
                case 0xb1: printf("Push %d\n\t",get8(fn)); i++; line++;
                case 0xb0: printf("Push %d\n",get8(fn)); i++; line++;
                           i++; break;
                case 0xbf: printf("Push %d\n\t",get16(fn)); i+=2; line++;
                case 0xbe: printf("Push %d\n\t",get16(fn)); i+=2; line++;
                case 0xbd: printf("Push %d\n\t",get16(fn)); i+=2; line++;
                case 0xbc: printf("Push %d\n\t",get16(fn)); i+=2; line++;
                case 0xbb: printf("Push %d\n\t",get16(fn)); i+=2; line++;
                case 0xba: printf("Push %d\n\t",get16(fn)); i+=2; line++;
                case 0xb9: printf("Push %d\n\t",get16(fn)); i+=2; line++;
                case 0xb8: printf("Push %d\n",get16(fn)); i+=2; line++;
                           i++; break;
                case 0x43: printf("Read store\n"); i++; line++; break;
                case 0x42: printf("Write store\n"); i++; line++; break;
                case 0x44: printf("Write CVT in pixel\n"); i++; line++; break;
                case 0x70: printf("Write CVT in FUnit\n"); i++; line++; break;
                case 0x45: printf("Read CVT in FUnit\n"); i++; line++; break;
                case 0: printf("Set vector to y-axis\n"); i++; line++; break;
                case 1: printf("Set vector to x-axis\n"); i++; line++; break;
                case 2: printf("Set prj vector to y-axis\n"); i++; line++; break;
                case 3: printf("Set prj vector to x-axis\n"); i++; line++; break;
                case 4: printf("Set frd vector to y-axis\n"); i++; line++; break;
                case 5: printf("Set frd vector to x-axis\n"); i++; line++; break;
                case 6: printf("Set prj vector to parelle line\n"); i++; line++; break;
                case 7: printf("Set prj vector to perpendicular line\n"); i++; line++; break;
                case 8: printf("Set frd vector to parelle line\n"); i++; line++; break;
                case 9: printf("Set frd vector to perpendicular line\n"); i++; line++; break;
                case 0xe: printf("Set frd vector to prj vector\n"); i++; line++; break;
                case 0x86: printf("Set dual prj vector to parelle line\n"); i++; line++; break;
                case 0x87: printf("Set dual prj vector to perpendicular line\n"); i++; line++; break;
                case 0xa: printf("Set prj vector\n"); i++; line++; break;
                case 0xb: printf("Set frd vector\n"); i++; line++; break;
                case 0xc: printf("Get prj vector\n"); i++; line++; break;
                case 0xd: printf("Get frd vector\n"); i++; line++; break;
                case 0x10: printf("Set Ref point 0\n"); i++; line++; break;
                case 0x11: printf("Set Ref point 1\n"); i++; line++; break;
                case 0x12: printf("Set Ref point 2\n"); i++; line++; break;
                case 0x13: printf("Set Zone point 0\n"); i++; line++; break;
                case 0x14: printf("Set Zone point 1\n"); i++; line++; break;
                case 0x15: printf("Set Zone point 2\n"); i++; line++; break;
                case 0x16: printf("Set Zone pointS\n"); i++; line++; break;
                case 0x19: printf("Round to half grid\n"); i++; line++; break;
                case 0x18: printf("Round to grid\n"); i++; line++; break;
                case 0x3d: printf("Round to dbl grid\n"); i++; line++; break;
                case 0x7d: printf("Round down to grid\n"); i++; line++; break;
                case 0x7c: printf("Round up to grid\n"); i++; line++; break;
                case 0x7a: printf("Round off\n"); i++; line++; break;
                case 0x76: printf("Super round \n"); i++; line++; break;
                case 0x77: printf("Super round 45 degree\n"); i++; line++; break;
                case 0x17: printf("Set loop var\n"); i++; line++; break;
                case 0x1a: printf("Set min dist\n"); i++; line++; break;
                case 0x8e: printf("Inst exec ctrl\n"); i++; line++; break;
                case 0x85: printf("Scan convert ctrl\n"); i++; line++; break;
                case 0x8d: printf("Scantype\n"); i++; line++; break;
                case 0x1d: printf("Set CVT cut in\n"); i++; line++; break;
                case 0x1e: printf("Set single width cut in\n"); i++; line++; break;
                case 0x1f: printf("Set single width\n"); i++; line++; break;
                case 0x4d: printf("Set auto-flip on\n"); i++; line++; break;
                case 0x4e: printf("Set auto-flip off\n"); i++; line++; break;
                case 0x7e: printf("Set angle width\n"); i++; line++; break;
                case 0x5e: printf("Set Delta-base\n"); i++; line++; break;
                case 0x5f: printf("Set Delta-Shift\n"); i++; line++; break;
                case 0x46: printf("Get coor use current position\n"); i++; line++; break;
                case 0x47: printf("Get coor use original outline\n"); i++; line++; break;
                case 0x48: printf("Set coor from stack\n"); i++; line++; break;
                case 0x49: printf("Measure dist in grid fitted outline\n"); i++; line++; break;
                case 0x4a: printf("Measure dist in original outline\n"); i++; line++; break;
                case 0x4b: printf("Measure PPEM\n"); i++; line++; break;
                case 0x4c: printf("Measure point size\n"); i++; line++; break;
                case 0x80: printf("Flip point \n"); i++; line++; break;
                case 0x81: printf("Flip range on\n"); i++; line++; break;
                case 0x82: printf("Flip range off\n"); i++; line++; break;
                case 0x32: printf("Shift point use rp2 by zp1\n"); i++; line++; break;
                case 0x33: printf("Shift point use rp1 by zp0\n"); i++; line++; break;
                case 0x34: printf("Shift contour use rp2 by zp1\n"); i++; line++; break;
                case 0x35: printf("Shift contour use rp1 by zp0\n"); i++; line++; break;
                case 0x36: printf("Shift zone use rp2 by zp1\n"); i++; line++; break;
                case 0x37: printf("Shift zone use rp1 by zp0\n"); i++; line++; break;
                case 0x38: printf("Shift point by pixel amount\n"); i++; line++; break;
                case 0x3a: printf("Move stack indir point without rp\n"); i++; line++; break;
                case 0x3b: printf("Move stack indir point with rp\n"); i++; line++; break;
                case 0x2e: printf("Move dir abs point without round\n"); i++; line++; break;
                case 0x2f: printf("Move dir abs point with round\n"); i++; line++; break;
                case 0x3e: printf("Move indir abs point without round\n"); i++; line++; break;
                case 0x3f: printf("Move indir abs point with round\n"); i++; line++; break;
                case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5:
                case 0xc6: case 0xc7: case 0xc8: case 0xc9: case 0xca: case 0xcb:
                case 0xcc: case 0xcd: case 0xce: case 0xcf: case 0xd0: case 0xd1:
                case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
                case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd:
                case 0xde: case 0xdf:
                           printf("Move dir point %s move rp0, %s keep dist, %s round with type %d\n",
                           (inst & 0x10)?"":"don't", (inst & 8)?"":"don't",
                           (inst & 4)?"":"don't", inst & 3);
                           i++; line += 2; break;
                case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5:
                case 0xe6: case 0xe7: case 0xe8: case 0xe9: case 0xea: case 0xeb:
                case 0xec: case 0xed: case 0xee: case 0xef: case 0xf0: case 0xf1:
                case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
                case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd:
                case 0xfe: case 0xff:
                           printf("Move indir relative point %s set rp0, %s keep dist, %s round with type %d\n",
                           (inst & 0x10)?"":"don't", (inst & 8)?"":"don't",
                           (inst & 4)?"":"don't", inst & 3);
                           i++; line += 2; break;
                case 0x3c: printf("Align relative point\n"); i++; line++; break;
                case 0x7f: printf("Adjust angle\n"); i++; line++; break;
                case 0xf: printf("Move point to intersection of 2 lines\n"); i++; line++; break;
                case 0x27: printf("Align points\n"); i++; line++; break;
                case 0x39: printf("Interpolate point by the last relative stretch\n"); i++; line++; break;
                case 0x29: printf("Untouch point\n"); i++; line++; break;
                case 0x30: printf("Interpolate untouched points in y dir\n"); i++; line++; break;
                case 0x31: printf("Interpolate untouched points in x dir\n"); i++; line++; break;
                case 0x5d: printf("Delta exception P1\n"); i++; line++; break;
                case 0x71: printf("Delta exception P2\n"); i++; line++; break;
                case 0x72: printf("Delta exception P3\n"); i++; line++; break;
                case 0x73: printf("Delta exception C1\n"); i++; line++; break;
                case 0x74: printf("Delta exception C2\n"); i++; line++; break;
                case 0x75: printf("Delta exception C3\n"); i++; line++; break;
                case 0x20: printf("Dup top stack element\n"); i++; line++; break;
                case 0x21: printf("Pop top stack element\n"); i++; line++; break;
                case 0x22: printf("Clear entire stack element\n"); i++; line++; break;
                case 0x23: printf("Swap top 2 stack elements\n"); i++; line++; break;
                case 0x24: printf("Return the depth of stack\n"); i++; line++; break;
                case 0x25: printf("Copy the indexed to the top of stack\n"); i++; line++; break;
                case 0x26: printf("Move the indexed to the top of stack\n"); i++; line++; break;
                case 0x8a: printf("Roll top 3 stack elements\n"); i++; line++; break;
                case 0x58: printf("If\n"); i++; line++; break;
                case 0x1b: printf("Else\n"); i++; line++; break;
                case 0x59: printf("Endif\n"); i++; line++; break;
                case 0x78: printf("Jump relative on true\n"); i++; line++; break;
                case 0x1c: printf("Jump\n"); i++; line++; break;
                case 0x79: printf("Jump relative on false\n"); i++; line++; break;
                case 0x50: printf("Less than\n"); i++; line++; break;
                case 0x51: printf("Less than or Equal\n"); i++; line++; break;
                case 0x52: printf("Greater than\n"); i++; line++; break;
                case 0x53: printf("Greater than or Equal\n"); i++; line++; break;
                case 0x54: printf("Equal\n"); i++; line++; break;
                case 0x55: printf("Notequal\n"); i++; line++; break;
                case 0x56: printf("ODD\n"); i++; line++; break;
                case 0x57: printf("EVEN\n"); i++; line++; break;
                case 0x5a: printf("And\n"); i++; line++; break;
                case 0x5b: printf("Or\n"); i++; line++; break;
                case 0x5c: printf("Not\n"); i++; line++; break;
                case 0x60: printf("Add\n"); i++; line++; break;
                case 0x61: printf("Sub\n"); i++; line++; break;
                case 0x62: printf("Div\n"); i++; line++; break;
                case 0x63: printf("Mul\n"); i++; line++; break;
                case 0x64: printf("Abs\n"); i++; line++; break;
                case 0x65: printf("Neg\n"); i++; line++; break;
                case 0x66: printf("Floor\n"); i++; line++; break;
                case 0x67: printf("Ceil\n"); i++; line++; break;
                case 0x8b: printf("MAX\n"); i++; line++; break;
                case 0x8c: printf("MIN\n"); i++; line++; break;
                case 0x68: case 0x69: case 0x6a: case 0x6b:
                           printf("Round with dist type %d\n",inst&3); i++; line++; break;
                case 0x6c: case 0x6d: case 0x6e: case 0x6f:
                           printf("Noround with dist type %d\n",inst&3); i++; line++; break;
                case 0x2c: printf("FDEF\n"); i++; line++; break;
                case 0x2d: printf("ENDF\n"); i++; line++; break;
                case 0x2b: printf("CALL\n"); i++; line++; break;
                case 0x2a: printf("Loop and CALL\n"); i++; line++; break;
                case 0x89: printf("IDEF\n"); i++; line++; break;
                case 0x4f: printf("Debug\n"); i++; line++; break;
                case 0x88: printf("Get information\n"); i++; line++; break;

                default: i++; break;
                }
         if (line > 10)
            {
            line = 0;
            printf("Press anykey(ESC brk)\r");
            if (getch() == 27) break;
            printf("%30s\r","");
            }
         }
     }

void dump_fpgm()
     {
     short i;
     for (i = 0; (i < numTbl) && (strcmp("fpgm",ttf_table[i].tag)); i++);
     if (i >= numTbl) return; /* No found */
     printf("%s\t%ld\t%ld\n",ttf_table[i].tag,ttf_table[i].off,ttf_table[i].len);
     if (fseek(fn,ttf_table[i].off,SEEK_SET)) return; /* can't seek */
     decode((unsigned short)ttf_table[i].len);
     }

void dump_prep()
     {
     short i;
     for (i = 0; (i < numTbl) && (strcmp("prep",ttf_table[i].tag)); i++);
     if (i >= numTbl) return; /* No found */
     printf("%s\t%ld\t%ld\n",ttf_table[i].tag,ttf_table[i].off,ttf_table[i].len);
     if (fseek(fn,ttf_table[i].off,SEEK_SET)) return; /* can't seek */
     decode((unsigned short)ttf_table[i].len);
     }

void get_table(char *str)
     {
     short i;
     FILE *fo;
     unsigned long off;
     while (*str == ' ') str++;
     for (i = 0; (i < numTbl) && (_strnicmp(str,ttf_table[i].tag,strlen(ttf_table[i].tag))); i++);
     if (i >= numTbl) return; /* No found */
     printf("%s\t%ld\t%ld\n",ttf_table[i].tag,ttf_table[i].off,ttf_table[i].len);
     if (fseek(fn,ttf_table[i].off,SEEK_SET)) return; /* can't seek */
     while (*str > ' ') str++; while (*str == ' ') str++;
     fo = fopen(str,"at");
     if (!fo) {printf("File %s open fail\n",str); fo = stdout;} /* open fail */
     fprintf(fo,"Table %s size %ld\n",ttf_table[i].tag,ttf_table[i].len);
     for (off = 0L; off < ttf_table[i].len; off++)
         {
         fprintf(fo,"%4d",get8(fn));
         if ((fo != stdout) && ((off % 20) == 19)) fprintf(fo,"\n");
         }
     fprintf(fo,"\n");
     if (fo != stdout) fclose(fo);
     }

void chksum_verify()
     {
     short i;
     unsigned long sum, curr, now, flen;
     if (!chk_adj)
        {
        printf("Dump head to get chk_adj!\n");
        dump_head();
        printf("Press anykey\r");
        getch();
        }
     for (i = 0; i < numTbl; i++)
         {
         printf("Verify %s ",ttf_table[i].tag);
         fseek(fn,ttf_table[i].off,SEEK_SET);
         for (sum = now = 0L;now < ttf_table[i].len; now += 4)
             {
             curr = get32(fn);
             if (ttf_table[i].len - now < 4)
                curr &= -1L << (8 * (4 - ttf_table[i].len + now));
             sum += curr;
             }
         if (sum == ttf_table[i].chk) printf("OK\n");
         else printf("%lx %lx\n",sum,ttf_table[i].chk);
         }
     printf("Verify font ");
     flen = _filelength(_fileno(fn));
     fseek(fn,0L,SEEK_SET);
     for (sum = now = 0; now < flen; now += 4)
         {
         curr = get32(fn);
         if (flen - now < 4) curr &= -1L << (8 * (4 - flen + now));
         sum += curr;
         }
     sum -= chk_adj;
     sum = 0xb1b0afba - sum;
     if (sum == chk_adj) printf("OK");
     else printf("%lx %lx\n",sum, chk_adj);
     }

void dump_lyjs()
     {
     short i;
     for (i = 0; (i < numTbl) && (strcmp("LYJs",ttf_table[i].tag)); i++);
     if (i >= numTbl) return; /* No found */
     printf("%s\t%ld\t%ld\n",ttf_table[i].tag,ttf_table[i].off,ttf_table[i].len);
     if (fseek(fn,ttf_table[i].off,SEEK_SET)) return; /* can't seek */
     printf("Version\t:\t%lx\n",get32(fn));
     printf("Format\t:\t%d\n",get16(fn));
     printf("#CFont\t:\t%d\n",get16(fn));
     printf("Next filled\t:\t%d\n",get16(fn));
     printf("Start Chinese\t:\t%d\n",get16(fn));
     }

void dump_os2()
     {
     short i;
     for (i = 0; (i < numTbl) && (strcmp("OS/2",ttf_table[i].tag)); i++);
     if (i >= numTbl) return; /* No found */
     printf("%s\t%ld\t%ld\n",ttf_table[i].tag,ttf_table[i].off,ttf_table[i].len);
     if (fseek(fn,ttf_table[i].off,SEEK_SET)) return; /* can't seek */
     printf("Version\t:\t%x\n",get16(fn));
     printf("Avg width\t:\t%d\n",get16(fn));
     printf("Weight class\t:\t%u\n",get16(fn));
     printf("Width class\t:\t%u\n",get16(fn));
     printf("fsTYPE\t:\t%x\n",get16(fn));
     printf("Sub X size\t:\t%d\n",get16(fn));
     printf("Sub Y size\t:\t%d\n",get16(fn));
     printf("Sub X off\t:\t%d\n",get16(fn));
     printf("Sub Y off\t:\t%d\n",get16(fn));
     printf("Super X size\t:\t%d\n",get16(fn));
     printf("Super Y size\t:\t%d\n",get16(fn));
     printf("Super X off\t:\t%d\n",get16(fn));
     printf("Super Y off\t:\t%d\n",get16(fn));
     printf("Strike out size\t:\t%d\n",get16(fn));
     printf("Strike out pos\t:\t%d\n",get16(fn));
     printf("Family\t:\t%x\n",get16(fn));
     getch();
     printf("Panose Family\t:\t%x\n",get8(fn));
     printf("Panose Serif\t:\t%x\n",get8(fn));
     printf("Panose Weight\t:\t%x\n",get8(fn));
     printf("Panose Proportion\t:\t%x\n",get8(fn));
     printf("Panose Contrast\t:\t%x\n",get8(fn));
     printf("Panose Stroke Var\t:\t%x\n",get8(fn));
     printf("Panose Arm style\t:\t%x\n",get8(fn));
     printf("Panose Letter form\t:\t%x\n",get8(fn));
     printf("Panose Midline\t:\t%x\n",get8(fn));
     printf("Panose Xheight\t:\t%x\n",get8(fn));
     printf("Char Range\t:\t%lx %lx %lx %lx\n",get32(fn),get32(fn),get32(fn),get32(fn));
     printf("Vendor\t:\t%c%c%c%c\n",get8(fn),get8(fn),get8(fn),get8(fn));
     printf("Fs Select\t:\t%x\n",get16(fn));
     printf("First index\t:\t%x\n",get16(fn));
     printf("Last index\t:\t%x\n",get16(fn));
     printf("Typo Ascender\t:\t%d\n",get16(fn));
     printf("Typo Descender\t:\t%d\n",get16(fn));
     printf("Typo LineGap\t:\t%d\n",get16(fn));
     printf("Win Ascender\t:\t%d\n",get16(fn));
     printf("Win Descender\t:\t%d\n",get16(fn));
     }
