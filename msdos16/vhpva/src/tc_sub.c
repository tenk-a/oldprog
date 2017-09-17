/* LSI-C(MS-C)�ɂ����Ăs�b�ɂȂ� DOS�֌W�̊֐� */
#include <dos.h>
#include <errno.h>
#include "tc_sub.h"

#define PTR_OFF(p)	((unsigned short) (p))
#define PTR_SEG(p)	((unsigned short)((unsigned long)(void far*)(p) >> 16))

#if 1
/*--------------------------------------------------------------------------*/
unsigned
_dos_read(int hno, void far *buf, unsigned siz, unsigned *cnt)
{
    union REGS reg;
    struct SREGS sreg;

    reg.h.al = 0x00;
    reg.h.ah = 0x3f;
    reg.x.bx = hno;
    reg.x.cx = siz;
    reg.x.dx = PTR_OFF(buf);
    sreg.ds  = PTR_SEG(buf);
    intdosx(&reg, &reg, &sreg);

 #if defined(LSI_C)
    if (reg.x.flags & 1)
 #else
    if (reg.x.cflag)
 #endif
    {
        *cnt = 0;
 #if 0
        if (reg.x.ax == 0x05)
            errno = EACCES;
        else if (reg.x.ax == 0x06)
            errno = EBADF;
        return reg.x.ax;
 #else
        return (errno = reg.x.ax);
 #endif
    } else {
        *cnt = reg.x.ax;
        return 0;
    }
}
#endif

/* _dos_findfirst/next ------------------------------------------------------
 
  _dos_findfirst(),_dos_findnext()�́A�ȉ���PDS�̿�����ʂ������A�֐����A
   �\���̖������킹,error�̂���Ƃ��Aerrno�ɒl��Ă���悤�ɂ��܂����B
 
 > dosdir.c --- DOS�ˑ��̃f�B���N�g���A�N�Z�X�֐�
 > $Header: DOSDIR.Cv  1.2  90/11/08 00:19:44  H.Kuno(NIF:MGG02367)  Exp $
 >
 >	DOS�Ɉˑ������p�X���ϊ���f�B���N�g���A�N�Z�X�֐��Q
 >
 > 1st coded by �v�� NIF:MGG02367   Sat, Sep  1 1990  00:15
 >
 >�@���̃\�[�X�� public domain �Ƃ��܂��̂ŁA����҂ɒf��Ȃ��A
 > �c���ړI���ӂ��߂Ď��R�ɗ��p�A�z�z�A���p�A���ς��s���Ă��������B

*/

/*--------------------------------------------------------
 �t�@�C���������J�n����(DOS #4e)
 
 <<<�߂�l>>>
  0     ... OK
  other ... MS-DOS Error code
 
 	<<<����>>>
  szPathName ... �����p�X��
  nAttrib    ... �����t�@�C���A�g���r���[�g
  pfind      ... �����o�b�t�@
*/
unsigned
_dos_findfirst(char* szPathName, unsigned nAttrib, struct find_t *pfind)
{
    union REGS reg;
    struct SREGS sreg;

	/* set DTA */
    reg.h.ah = 0x1a;
    reg.x.dx = PTR_OFF(pfind);
    sreg.ds  = PTR_SEG(pfind);
    intdosx(&reg, &reg, &sreg);
	/* findfirst */
    reg.h.ah = 0x4e;
    reg.x.cx = nAttrib;
    reg.x.dx = PTR_OFF(szPathName);
    sreg.ds  = PTR_SEG(szPathName);
    intdosx(&reg, &reg, &sreg);
	if (pfind->name[0] == 0x05)
            pfind->name[0] = 0xe5; /* DOS 3.1�ȑO�̃o�O�΍� */
 #if defined(LSI_C)
    return (reg.x.flags & 1) ? (errno = reg.x.ax) : 0;
 #else
    return (reg.x.cflag) ? (errno = reg.x.ax) : 0;
 #endif
}

/*-------------------------------------------------------
  dos_findfirst() �ɑ����ăt�@�C���������s��(DOS #4f)
 
 	<<<�߂�l>>>
  0     ... OK
  other ... MS-DOS Error code
 
 	<<<����>>>
  pfind      ... �����o�b�t�@(_dos_findfirst()�ŏ����ς݂̂���)
*/
unsigned
_dos_findnext(struct find_t *pfind)
{
    union REGS reg;
    struct SREGS sreg;

	/* set DTA */
    reg.h.ah = 0x1a;
    reg.x.dx = PTR_OFF(pfind);
    sreg.ds  = PTR_SEG(pfind);
    intdosx(&reg, &reg, &sreg);
	/* findnext */
    reg.h.ah = 0x4f;
    intdos(&reg, &reg);
	if (pfind->name[0] == 0x05)
            pfind->name[0] = 0xe5; /* DOS 3.1�ȑO�̃o�O�΍� */
 #if defined(LSI_C)
    return (reg.x.flags & 1) ? (errno = reg.x.ax) : 0;
 #else
    return (reg.x.cflag) ? (errno = reg.x.ax) : 0;
 #endif
}
