#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "ntuttf.h"
#include <VF.h>

#define _putw(x,f) {putc(x & 0xff, f); putc(x >> 8, f);}

int main(int argc, char **argv)
{
	unsigned char *buf;
	FILE *fp;
	int x, y, s, l, i, font, jis;
	int mask[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	int bufsize = XSIZE*YSIZE/8;

	if (argc != 4) {
		fprintf(stderr, "Usage: VFlibbmp fontname code outfile\n");
		exit(1);
	}

	if (VF_Init(NULL) < 0) {
		fprintf(stderr, "Initializing VFlib: FAILED\n");
		exit(1);
	}
    if ((font = VF_OpenFont(argv[1])) < 0){
		fprintf(stderr, "Can't open font \'%s\'\n", argv[1]);
		exit(1);
	}
	if ((i = atoi(argv[2])) >= 94*94) {
		fprintf(stderr, "Invalid code\n");
		exit(1);
	}
	jis = ((i / 94) + 0x21) * 256 + (i % 94) + 0x21;
	printf("\tjis = %04x\n", jis);
	buf = malloc(bufsize);
	for (i = 0; i < bufsize; i++) buf[i] = 0;
	VF_GetBitmap(jis, font, XSIZE, YSIZE, XSIZE/8, 0, buf);

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
	}
	fclose(fp);
	free(buf);
	return(0);
}