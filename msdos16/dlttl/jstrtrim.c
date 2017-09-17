#include <stddef.h>
#include "jstr.h"
#include "tenkdefs.h"

#define iskanji(c)  (((c)>=0x81 && (c)<=0x9f) || ((c)>=0xE0 && (c)<=0xfc))
#define iskanji2(c) ((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)

/*
 * unsigned char *jstrtrim(unsigned char *jstr);
 * 行末の余分な空白･ｺﾝﾄﾛｰﾙｺｰﾄﾞを取りのぞく。
 * ただし、文字列の最後の文字が'\n'ならば、それは取り除いた後付加する。
 * 復帰値はjstrと同じ.
 */
byte *
jstrtrim(jstr)
     byte *jstr;
{
     byte *q;
     REGI byte *p;

     q = p = jstr;
     while (*p != '\0') {
    	  if (*p <= ' ')
    	       ++p;
    	  else if (*p == 0x81 && *(p+1) == 0x40)
    	       p += 2;
    	  else
    	       q = (++p);
     }
     if (*(p - 1) == '\n')
    	  *(q++) = '\n';
     *q = '\0';
     return jstr;
}
