#include <stddef.h>
#include "jstr.h"
#include "tenkdefs.h"

#define iskanji(c)  (((c)>=0x81 && (c)<=0x9f) || ((c)>=0xE0 && (c)<=0xfc))
#define iskanji2(c) ((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)

/*
 * unsigned char *jstrtrim(unsigned char *jstr);
 * �s���̗]���ȋ󔒥���۰ٺ��ނ����̂����B
 * �������A������̍Ō�̕�����'\n'�Ȃ�΁A����͎�菜������t������B
 * ���A�l��jstr�Ɠ���.
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
