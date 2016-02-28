#ifndef _RPAL_H_
#define _RPAL_H_
struct RAMPAL {
	byte id[10];
	byte toon;
	byte rsv[5];
	byte grb[16][3];
};

struct RAMPAL far *RPal_Search(void);
int  RPal_Get(int *toon, unsigned char *grb);
void RPal_Set(int  toon, unsigned char *grb);
#endif
