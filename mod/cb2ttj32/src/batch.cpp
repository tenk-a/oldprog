#define _DOS
#include <afx.h>
#include <iostream.h>
#include <stdio.h>
#include <process.h>
#include <stdlib.h>

void main(short argc,char *argv[])
     {
     if (argc < 2) { cout << "Usage: BATCH batchfile [start [end]]\n"; return; }
     short start = 0, end = 13502;
     if (argc >= 3) start = atoi(argv[2]);
     if (argc >= 4) end = atoi(argv[3]);
     if (end > 13502) end = 13502; //Current only support to here
     for (short i = start; i < end; i++)
         {
         cout << "\r" << i; cout.flush();
         char data[256];
         sprintf(data, "%s %d", argv[1],i);
         system(data);
         }
     }
