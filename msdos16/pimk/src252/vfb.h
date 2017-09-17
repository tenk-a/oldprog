#define VFB_SIZE    0x6000
void far VFB_GetLines(void far *dat, unsigned char far *buf);
void far VFB_GetLine24(void far *dat, unsigned char far *buf);
void VFB_RevX(unsigned Seg);
void VFB_RevY(unsigned Seg);
