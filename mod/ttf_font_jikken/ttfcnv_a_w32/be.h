/* win32—p ‰R be.h
 */
#ifndef	BE_H
#define	BE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef	short	       int16;
typedef	unsigned       uint;
typedef	unsigned short uint16;
typedef	unsigned int   uint32;
typedef	unsigned char  uchar;

inline int B_BENDIAN_TO_HOST_INT16(int c) { return ((((c)&0xff)<<8)  | ((c)>>8)&0xff); }
inline int B_BENDIAN_TO_HOST_INT32(int c) { return ((((c)&0xff)<<24) | (((c)&0xff00)<<8) | (((c)>>8)&0xff00) | (((c)>>24)&0xff)); }
inline int B_HOST_TO_BENDIAN_INT16(int c) { return ((((c)&0xff)<<8)  |	((c)>>8)&0xff);	}
inline int B_HOST_TO_BENDIAN_INT32(int c) { return ((((c)&0xff)<<24) | (((c)&0xff00)<<8) | (((c)>>8)&0xff00) | (((c)>>24)&0xff)); }

#endif
