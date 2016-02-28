/*
 *  VHP Loader & Saver
 */

#ifndef LSI_C
  /* TC v2.0c(ｽﾓｰﾙﾓﾃﾞﾙ) */
	#include <alloc.h>
	#include "tc_sub.h"
#endif

#define ON  1
#define OFF 0

#define BLUE   0
#define RED    1
#define GREEN  2

#define LMAXX 160
#define LMAXY 200

/* エラー */
#define ER_EXIT   0
#define ER_BUG    1
#define ER_WRITE  2 /* Sa */
#define ER_NTLONG 3 /* Sa */
#define ER_SW     4 /* Sa */
#define ER_AB     5 /* Lo */
#define ER_OPEN   6 /* Lo */

/* ハフマン符号化 */
#define MAXHUF 1024

/* パターン検索方向 */
#define UP     1
#define DOWN   2
#define LEFT   3
#define RIGHT  4

/* ペイント時の最大ネスト数 */
#define MAXNEST 500

/* パターンテーブルの大きさ */
#define MAXTBSIZE 256

/* ｐａｓｓ２パターンテーブルの大きさ */
#define MAXP2TB 1024

/* 画像データの属性 */
#define AT_200     0x00
#define AT_400     0x01
#define AT_C8      0x00
#define AT_C16     0x02
#define AT_DIGITAL 0x00
#define AT_ANALOG  0x04

/* pass2 の識別子 */
#define P_NON        0x00
#define P_SAMEUP     0x1ff    /* ０は非圧縮に使用する */
#define P_SAMEDOWN   0x0f
#define P_SAMELEFT   0x10
#define P_SAMERIGHT  0x1f
#define P_SAME1UP    0x20

#define P_COL0       0x2f
#define P_COL1       0x30
#define P_COL2       0x3f
#define P_COL3       0x40
#define P_COL4       0x4f
#define P_COL5       0x50
#define P_COL6       0x5f
#define P_COL7       0x60

#define P_COL8       0x200
#define P_COL9       0x20f
#define P_COL10      0x210
#define P_COL11      0x21f
#define P_COL12      0x220
#define P_COL13      0x22f
#define P_COL14      0x230
#define P_COL15      0x23f

#define P_LR11       0x6f
#define P_LR13       0x70
#define P_LR31       0x7f
#define P_LR22       0x80

#define P_LU11       0x8f
#define P_LU13       0x90
#define P_LU31       0x9f
#define P_LU22       0xa0

#define P_LD11       0xaf
#define P_LD13       0xb0
#define P_LD31       0xbf
#define P_LD22       0xc0

#define P_UR11       0xcf
#define P_UR13       0xd0
#define P_UR31       0xdf
#define P_UR22       0xe0

#define P_DR11       0xef
#define P_DR13       0xf0
#define P_DR31       0xff
#define P_DR22       0x100

#define P_UD11       0x10f
#define P_UD13       0x110
#define P_UD31       0x11f
#define P_UD22       0x120

#define P_DU11       0x12f
#define P_DU13       0x130
#define P_DU31       0x13f
#define P_DU22       0x140

#define P_S0         0x14f
#define P_SU1        0x150
#define P_SU2        0x15f
#define P_SU3        0x160
#define P_SD1        0x16f
#define P_SD2        0x170
#define P_SD3        0x17f

#define P_2COL1      0x180
#define P_2COL2      0x18f
#define P_2COL3      0x190
#define P_2COL4      0x19f
#define P_2COL5      0x1a0
#define P_2COL6      0x1af
#define P_2COL7      0x1b0
#define P_2COL8      0x1bf
#define P_2COL9      0x1c0
#define P_2COLA      0x1cf
#define P_2COLB      0x1d0
#define P_2COLC      0x1df
#define P_2COLD      0x1e0
#define P_2COLE      0x1ef

#define P_LC         0x00
#define P_RC         0x80
#define P_UC         0x100
#define P_DC         0x180
