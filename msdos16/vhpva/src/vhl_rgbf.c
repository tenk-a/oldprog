#include <stdio.h>
#include <string.h>
#include "tenkafun.h"
#define LMAXX	160 /* ÀÞÐ° */
#define LMAXY	200 /* ÀÞÐ° */
#include "vhl.h"
#include "sub.h"

/*---------------------------------------------------------------------------*/

static void
rgb_to_hsv(int r, int g, int b, int hsv[])
	/*
	 * RGB -> hsv
	 * in : r  - R (0-15)
	 *		g  - G (0-15)
	 *		b  - B (0-15)
	 *
	 * out: hsv[0]  - F‘Š     (0-360)
	 *		hsv[1]  - FÊ     (0-100)
	 *		hsv[2]  - Å‘å–¾“x (0-100)
	 */
{
	int  h, s, v, x, r1, g1, b1;

	r = r * 100 / 15;
	g = g * 100 / 15;
	b = b * 100 / 15;

	h = 0;
	v = (r > g) ? r : g;
	v = (v > b) ? v : b;

	x = (r < g) ? r : g;
	x = (x < b) ? x : b;

	s = (v == 0) ? 0 : (v - x) * 100 / v;

	if (s != 0) {
		r1 = (v - r) * 60 / (v - x);
		g1 = (v - g) * 60 / (v - x);
		b1 = (v - b) * 60 / (v - x);

		if (r == v) {
			if (g == x)
				h = 300 + b1;
			else
				h = 60 - g1;
		} else if (g == v) {
			if (b == x)
				h = 60 + r1;
			else
				h = 180 - b1;
		} else {
			if (r == x)
				h = 180 + g1;
			else
				h = 300 - r1;
		}
		if (h < 0)
			h += 360;
		h %= 360;
	}
	hsv[0] = h;
	hsv[1] = s;
	hsv[2] = v;
}


static int
buildalg(byte *name, byte grb[])
	/* alg file ì¬ */
{
	FILE *fpw;
	static int cv[] = {0, 9, 10, 11, 12, 13, 14, 15, 8, 1, 2, 3, 4, 5, 6, 7};
	int  i, a, b, hsv[3], hsvbuf[48];

	if ((fpw = fopen(name, "wb")) == NULL) {
		puts(name);
		puts("Can't build alg file.");
		return 1;
	}
	for (i = 0; i < 16; ++i) {
		rgb_to_hsv(grb[i * 3 + 1], grb[i * 3 + 0], grb[i * 3 + 2], hsv);
		a = cv[i] / 8;
		b = cv[i] % 8;
		hsvbuf[a * 24 + b] = hsv[0];
		hsvbuf[a * 24 + b + 8] = hsv[1];
		hsvbuf[a * 24 + b + 16] = hsv[2];
	}

	for (i = 0; i < 48; ++i) {
		putc(hsvbuf[i] % 256, fpw);
		putc(hsvbuf[i] / 256, fpw);
	}
	fclose(fpw);
	return 0;
}


static int
buildrgb(byte *name, byte *grb)
	/* rgb file ì¬ */
{
	FILE *fpw;
	int  i;

	if ((fpw = fopen(name, "wb")) == NULL) {
		puts(name);
		puts("Can't build rgb file.");
		return 1;
	}
	for (i = 0; i < 16; ++i) {
		putc(grb[i*3 + 1] & 0xFF, fpw);
		putc(grb[i*3 + 0] & 0xFF, fpw);
		putc(grb[i*3 + 2] & 0xFF, fpw);
	}
	fclose(fpw);
	return 0;
}


void
putPalFile(int sw_r,byte *grb,byte *fname)
{
	byte algname[128];

	if (strlen(fname) > 120)
		return;
	strcpy(algname,fname);
	if (sw_r == 1) {
		reset_ext(algname,"RGB");
		buildrgb(algname, grb);
	} else {
		reset_ext(algname, "ALG");
		buildalg(algname, grb);
	}
	return;
}
