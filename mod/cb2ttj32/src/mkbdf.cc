// mkbdf.cc
// This program makes bdf files from 98CG image files.
// I don't have knowledge about font structure very much.


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define	Stop() Stop_Stop(__FILE__, __LINE__)

typedef unsigned char byte;			  // 8bit
typedef unsigned short word;			  // 16bit
typedef unsigned long dword;			  // 32bit

const char* const TMPFILE1	= "mkbdf.tmp1";
const char* const TMPFILE2	= "mkbdf.tmp2";
const int BUF_ANK_BYTES		= 16 * 256;
const int BUF_QUARTER_BYTES	= 8 * 256;
const int BUF_KANJI_BYTES	= 32 * ((0x7c - 0x21 + 1) * (0x7f - 0x20 + 1));


static FILE* start_bdf();
static void end_bdf(FILE* const fp_tmp1,
		    const char* const filename,
		    const int x,
		    const int y,
		    const int points,
		    const char* registry,
		    const int encoding,
		    const int default_char,
		    const int chars);
static void load_font(const char* const filename,
		      byte* const buf,
		      const int len);
static int make_font(FILE* const fp,
		     const byte* buf,
		     const int x,
		     const int y,
		     const int font_bytes,
		     const word start_code,
		     const word end_code);
static void Stop_Stop(const char *const filename, const unsigned int line);


int main()
{
	int chars;
	FILE* fp;
	
	byte* const buf_ank = new byte[BUF_ANK_BYTES];
	load_font("ank.cg", buf_ank, BUF_ANK_BYTES);
	fp = start_bdf();
	chars = make_font(fp, buf_ank, 8, 16, 16, 0, 0xff);
	end_bdf(fp, "98ank.bdf", 8, 16, 12,
		"JISX0201.0000", 0, 0x20, chars);
	delete[] buf_ank;
					  
	byte* const buf_quarter = new byte[BUF_QUARTER_BYTES];
	load_font("quarter.cg", buf_quarter, BUF_QUARTER_BYTES);
	fp = start_bdf();
	chars = make_font(fp, buf_quarter, 8, 8, 8, 0, 0xff);
	end_bdf(fp, "98quarter.bdf", 8, 8, 5,
		"JISX0201.0000", 0, 0x20, chars);
	delete[] buf_quarter;
					  
	byte* const buf_kanji = new byte[BUF_KANJI_BYTES];
	load_font("kanji.cg", buf_kanji, BUF_KANJI_BYTES);
	fp = start_bdf();
 	chars = make_font(fp, buf_kanji, 16, 16, 32, 0x2120, 0x287f);
	chars += make_font(fp,
			   &buf_kanji[32 * ((0x29 - 0x21) * (0x7f - 0x20 +1))],
			   8, 16, 32, 0x2920, 0x2b7f);
 	chars += make_font(fp,
 			   &buf_kanji[32 * ((0x2c - 0x21) * (0x7f - 0x20 +1))],
 			   16, 16, 32, 0x2c20, 0x7c7f);
	end_bdf(fp, "98kanji.bdf", 16, 16, 15,
		"JISX0208.0000", 0, 0x2121, chars);
	delete[] buf_kanji;
	
	return 0;
}


FILE* start_bdf()
{
	FILE* fp = fopen(TMPFILE1, "w");
	if (fp == NULL)
		Stop();
	return fp;
}


void end_bdf(FILE* const fp_tmp1,
	     const char* const filename,
	     const int x,
	     const int y,
	     const int points,
	     const char* registry,
	     const int encoding,
	     const int default_char,
	     const int chars)
{
	fprintf(fp_tmp1, "ENDFONT\n");
	fclose(fp_tmp1);
	
	FILE* const fp_tmp2 = fopen(TMPFILE2, "w");
	if (fp_tmp2 == NULL)
		Stop();

	const char header[] =
"STARTFONT 2.1\n"
"COMMENT This bdf file was generated from pc98 font.\n"
"FONT -Neco-Fixed-Medium-R-Normal--%d-%d-75-75-C-%d-%s-%d\n"
"SIZE %d 75 75\n"
"FONTBOUNDINGBOX %d %d 0 0\n"
"STARTPROPERTIES 20\n"
"FONTNAME_REGISTRY \"\"\n"
"FOUNDRY \"Neco\"\n"
"FAMILY_NAME \"Fixed\"\n"
"WEIGHT_NAME \"Medium\"\n"
"SLANT \"R\"\n"
"SETWIDTH_NAME \"Normal\"\n"
"ADD_STYLE_NAME \"\"\n"
"PIXEL_SIZE %d\n"
"POINT_SIZE %d\n"
"RESOLUTION_X 75\n"
"RESOLUTION_Y 75\n"
"SPACING \"C\"\n"
"AVERAGE_WIDTH %d\n"
"CHARSET_REGISTRY \"%s\"\n"
"CHARSET_ENCODING \"%d\"\n"
"COPYRIGHT \"\"\n"
"DEFAULT_CHAR %d\n"
"X_HEIGHT %d\n"
"FONT_ASCENT %d\n"
"FONT_DESCENT 0\n"
"ENDPROPERTIES\n"
"CHARS %d\n";

	fprintf(fp_tmp2,
		header,
		y, points * 10, x * 10, registry, encoding,
		points,
		x, y,
		y,
		points * 10,
		x * 10,
		registry,
		encoding,
		default_char,
		y,
		y,
		chars);
	fclose(fp_tmp2);
	
	char buf[1000];
	sprintf(buf, "cat %s %s > %s", TMPFILE2, TMPFILE1, filename);
	system(buf);
	unlink(TMPFILE1);
	unlink(TMPFILE2);
}


void load_font(const char* const filename,
	       byte* const buf,
	       const int len)
{
	FILE* const fp = fopen(filename, "r");
	if (fp == NULL)
		Stop();

	if (fread(buf, len, 1, fp) != 1)
		Stop();

// 	if (fseek(fp, 0, SEEK_END) != len)
// 		Stop();
	
	fclose(fp);
}


int make_font(FILE* const fp,
	      const byte* buf,
	      const int x,
	      const int y,
	      const int font_bytes,
	      const word start_code,
	      const word end_code)
{
	int chars = 0;
	for (word code = start_code; code <= end_code; code++) {
		if (font_bytes == 32 && ((code & 0x00ff) < 0x0020 ||
					 (code & 0x00ff) > 0x007f))
			continue;
		
		if (code != 0x20 && code != 0x2121) { // if not space
			bool space_flag = true;
			for (int i = 0; i < font_bytes; i++)
				if (buf[i] != 0) {
					space_flag = false;
					break;
				}
			if (space_flag) {
				buf += font_bytes;
				continue;
			}
		}
		
		fprintf(fp, "STARTCHAR %04x\n", code);
		fprintf(fp, "ENCODING %d\n", code);
		fprintf(fp, "SWIDTH %d 0\n", ((x == 16) ? 1024 : 480));
		fprintf(fp, "DWIDTH %d 0\n", x);
		fprintf(fp, "BBX %d %d 0 0\n", x, y);
		fprintf(fp, "BITMAP\n");
		if (x == 16)
			for (int i = 0; i < y * 2; i += 2)
				fprintf(fp, "%02x%02x\n", buf[i], buf[i + 1]);
		else if (x == 8)
			for (int i = 0; i < y; i++)
				fprintf(fp, "%02x\n", buf[i]);
		else
			Stop();
		fprintf(fp, "ENDCHAR\n");
		chars++;
		buf += font_bytes;
	}	
	return chars;
}


void Stop_Stop(const char *const filename, const unsigned int line)
{
	fflush(stdout);
	fprintf(stderr, "Stop: file \"%s\", line %d\n", filename, line);
	fflush(stderr);
	exit(EXIT_FAILURE);
}
