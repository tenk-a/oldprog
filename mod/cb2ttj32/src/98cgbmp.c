#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "ntuttf.h"

#  define _putw(x,f) {putc((x) & 0xff, f); putc((x) >> 8, f);}

/* bit_offset is not implemented. */
/* width is not used. */
static void get_bitmap(int kanji_flag,
		       FILE *fp, int code, int width, int height,
		       int bytes_per_line, int bit_offset, char buf[])
{
	const int WIDTH98 = kanji_flag ? 16 : 8;
	const int HEIGHT98 = 16;
	const int BYTES98 = kanji_flag ? 32 : 16;
	unsigned char mask[] = {
		0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	int lines_per_98line = height / HEIGHT98;
	int bytes_per_98pixel = bytes_per_line / WIDTH98; /* 1line */
	unsigned char cg_buf[32];
	int index = ((code - 0x2100) % 0x80) - 0x20; /* 0 - 0x5f */
	int index2 = 0x60 * (((code - 0x2100) & ~0x80) / 0x100);
	const int offset = BYTES98 * (kanji_flag ? (index2 + index) : code);
	int x, y;
	
	if (fseek(fp, offset, SEEK_SET) < 0) {
		fprintf(stderr, "Can't seek\n");
		exit(1);
	}
	if (fread(cg_buf, BYTES98, 1, fp) != 1) {
		fprintf(stderr, "Can't read\n");
		exit(1);
	}
	if (0x2921 <= code && code <= 0x2b7e) { /* hankaku kanji */
		char tmp_buf[16];
		memcpy(tmp_buf, cg_buf, 16);
#if 0
		/* 8x16 */
		memset(cg_buf, 0, 32);
		for (y = 0; y < 16; y++)
			cg_buf[2 * y] = tmp_buf[y];
#else
		/* 16x16 */
		for (y = 0; y < 16; y++) {
			cg_buf[2 * y + 0] =
				((tmp_buf[y] & 0x80) ? 0xc0 : 0) |
				((tmp_buf[y] & 0x40) ? 0x30 : 0) |
				((tmp_buf[y] & 0x20) ? 0x0c : 0) |
				((tmp_buf[y] & 0x10) ? 0x03 : 0);
			cg_buf[2 * y + 1] =
				((tmp_buf[y] & 0x08) ? 0xc0 : 0) |
				((tmp_buf[y] & 0x04) ? 0x30 : 0) |
				((tmp_buf[y] & 0x02) ? 0x0c : 0) |
				((tmp_buf[y] & 0x01) ? 0x03 : 0);
		}
#endif
	}
	for (y = 0; y < HEIGHT98; y++) {
		for (x = 0; x < WIDTH98; x++) {
			if (cg_buf[(WIDTH98 / 8) * y + x / 8] & mask[x % 8]) {
				char* ptr =
					&buf[(bytes_per_line *
					      lines_per_98line * y +
					      bytes_per_98pixel * x)];
				int l;
/* 				printf("*"); */
				for (l = 0; l < lines_per_98line; l++) {
					memset(ptr, 0xff, bytes_per_98pixel);
					ptr += bytes_per_line;
				}
			}
/* 			else */
/* 				printf(" "); */
		}
/* 		printf("\n"); */
	}
}

int main(int argc, char **argv)
{
	unsigned char *buf;
	FILE *fp;
	int x, y, s, l, i, jis;
	int mask[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	int bufsize = XSIZE*YSIZE/8;
	int kanji_flag;
	
	if (argc != 5) {
		fprintf(stderr, "Usage: 98cgbmp cgfile code outfile kanji_or_ank(1or0) \n");
		exit(1);
	}

	if ((fp = fopen(argv[1], "rb")) == NULL) {
		fprintf(stderr, "Can't open cg file \'%s\'\n", argv[1]);
		exit(1);
	}
	if ((i = atoi(argv[2])) >= 94*94) {
		fprintf(stderr, "Invalid code\n");
		exit(1);
	}
	kanji_flag = atoi(argv[4]);
	if (kanji_flag) {
		jis = ((i / 94) + 0x21) * 256 + (i % 94) + 0x21;
		printf("\tjis = %04x\n", jis);
	} else {
		jis = i;
		printf("\tank = %02x\n", jis);
	}
	buf = malloc(bufsize);
	memset(buf, 0, bufsize);
	if (jis <= 0x7c7f)
		get_bitmap(kanji_flag,
			   fp, jis, XSIZE, YSIZE, XSIZE / 8, 0, buf);
	fclose(fp);
	
	if ((fp = fopen(argv[3], "wb")) == NULL) {
		fprintf(stderr, "Can't open \'%s\'\n", argv[3]);
		exit(1);
	}
	fwrite("DYNA", 1, 4, fp);
	for (y = 0; y < YSIZE; y++) {
		s = l = 0;
		for (x = 0; x < XSIZE; x++) {
			if (buf[y*XSIZE/8 + x/8]  & mask[x & 0x07]) {
				l++;
			} else {
				if (l > 0) {
					_putw(y, fp);
					_putw(s, fp);
					_putw(s + l, fp);
					l = 0;
				}
				s = x;
			}
		}
		if (l > 0) {
			_putw(y, fp);
			_putw(s, fp);
			_putw(s + l, fp);
			l = 0;
		}
	}
	fclose(fp);
	free(buf);
	return(0);
}
