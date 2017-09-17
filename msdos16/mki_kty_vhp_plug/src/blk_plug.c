/*===========================================================================*/
/*  	DJP�ȃv���O�C���d�l�́@BLK->DJP,BMP,PMT �R���o�[�^  	    	     */
/*  	    	    	    	    	    	    	    	    	     */
/*  	    far(32�r�b�g)�|�C���^�̃R���p�C���E���f���ŃR���p�C�����邱��    */
/*  	    �܂�A��ư��Ӱ٤���߸Ă͑ʖ�(fread�̓s��^^;).	    	     */
/*===========================================================================*/

#include "tofmt4p.c"


/*---------------------------------------------------------------------------*/
/*-----    �w  ��  �v	 ----------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void Usage(void)
{
    puts(
    	"BLK_PLUG v1.00                      by Tenka* 1995\n"
    	"usage: blk_plug <COMMAND> <inputfile> <outputfile>\n"
    	" <COMMAND> ... DJ505JC, TO_DJP or TO_BMP.\n"
    	);
    exit(1);
}


/*---------------------------------------------------------------------------*/
/*-----    �� �� �� �[ �` ��   ----------------------------------------------*/
/*---------------------------------------------------------------------------*/

int  GLoad(TOFMT4P *t, char *name)
{
    FILE *fp;
    int  i;

    t->xsz = 640;
    t->ysz = 400;
    fp = fopen(name, "rb"); 	    	if (fp == NULL) return -1;
    fread(t->rgb, 1, 16*3, fp);     	if (ferror(fp)) return -1;
    /* 4bits->8bits, B:R:G->R:G:B */
    for (i = 0; i < 16*3; i+=3) {
    	unsigned char  r, g, b;
    	b = t->rgb[i+0] << 4;
    	r = t->rgb[i+1] << 4;
    	g = t->rgb[i+2] << 4;
    	t->rgb[i+0] = r;
    	t->rgb[i+1] = g;
    	t->rgb[i+2] = b;
    }
    fread(t->v[0], 1, 80*400U, fp); 	if (ferror(fp)) return -1;
    fread(t->v[1], 1, 80*400U, fp); 	if (ferror(fp)) return -1;
    fread(t->v[2], 1, 80*400U, fp); 	if (ferror(fp)) return -1;
    fread(t->v[3], 1, 80*400U, fp); 	if (ferror(fp)) return -1;
    fclose(fp);
    return 0;
}
