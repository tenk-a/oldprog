/* LSI-C(MS-C)�ɂ����Ăs�b�ɂȂ� DOS�֌W�̊֐� */

#ifndef _TC_SUB_H_
#define _TC_SUB_H_

/* DOS�̃t�@�C���A�g���r���[�g�l */
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_VOLID  0x08
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20
/* �t�@�C�������̃o�b�t�@(dos_findfirst/next()�p) */
struct find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
};

unsigned _dos_findfirst(char *PathName, unsigned Attrib, struct find_t *pfind);
unsigned _dos_findnext(struct find_t *pfind);
unsigned _dos_read(int hno, void far *buf, unsigned siz, unsigned *cnt);

#endif
