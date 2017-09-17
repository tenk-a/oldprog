#include "stdio.h"

void main()
     {
     unsigned char inbuf[256];
     for (;;)
         {
         printf("#>");
         gets(inbuf);
         printf("%d\n",big5(inbuf[0],inbuf[1]));
         }
     }
