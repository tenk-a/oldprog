/**
 *  @file   bccEr2hm.c
 *  @brief  Bcc32���o�͂����G���[���b�Z�[�W���G�ۂŃ^�O�W�����v�ł���`�ɕϊ�
 *  @author tenk@6809.net
 *  @date   199?
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#define STDERR	    stdout



void usage(void)
{
    printf(
    	"usage>BccEr2Hm [-opts] [infile]   // " __DATE__ "  by tenk*\n"
    	"bcc32 ���o�͂����G���[���b�Z�[�W��W������(infile���w�肳����\n"
    	"�t�@�C���Ǎ�)���A�G�ۃG�f�B�^�Ń^�O�W�����v�ł���`�i�Ƃ�����\n"
    	"��ʓI?�ȃ^�O�t�@�C���`��)�ɕς��ĕW���o�͂���\n"
    	" -o[FILE]  �W���o�͂łȂ� FILE �ɓf���o��.\n"
    	//" 	      FILE���Ȃ���Γ��̓t�@�C���ɏ㏑������\n"
    );
    exit(1);
}



void err_exit(char *fmt, ...)
{
    va_list app;
    va_start(app, fmt);
    vfprintf(STDERR, fmt, app);
    va_end(app);
    exit(1);
}



FILE *fopenE(const char *name, char *mod)
{
    /* �G���[������Α�exit�� fopen() */
    FILE *fp;

    fp = fopen(name,mod);
    if (fp == NULL) {
    	err_exit("�t�@�C�� %s ���I�[�v���ł��܂���\n",name);
    }
    setvbuf(fp, NULL, _IOFBF, 1024*1024);
    return fp;
}



void BccErrMsg2vc(const char *iname, const char *oname)
{
    FILE *fp;
    FILE *ofp;
    int  l;
    char mm[256];
    char buf[2048];
    char *p;
    char *q;

    if (iname == NULL)
    	fp = stdin;
    else
    	fp = fopenE(iname, "rt");
    if (oname == NULL) {
    	ofp = stdout;
    } else {
    	ofp = fopenE(oname, "wt");
    }

    for (;;) {
    	p = fgets(buf, sizeof(buf), fp);
    	if (p == NULL)
    	    break;
    	if (ferror(fp))
    	    err_exit("�t�@�C���ǂݍ��݂ŃG���[����\n");
    	if (memcmp(buf, "�G���[ E", 8) == 0) {
    	    memcpy(mm, buf, 12);
    	    mm[12] = 0;
    	    p = strchr(buf+12+1, ' ');
    	    if (p) {
    	    	p++;
    	    	q = strchr(p, ':');
    	    	if (q) {
    	    	    p = buf+13;
    	    	    l = q - p;
    	    	    memmove(buf, p, l);
    	    	    buf[l] = ':';
    	    	    memcpy(buf+l+1, mm, strlen(mm));
    	    	}
    	    }
    	} else if (memcmp(buf, "�x�� W", 6) == 0) {
    	    memcpy(mm, buf, 10);
    	    mm[10] = 0;
    	    p = strchr(buf+10+1, ' ');
    	    if (p) {
    	    	p++;
    	    	q = strchr(p, ':');
    	    	if (q) {
    	    	    p = buf + 11;
    	    	    l = q - p;
    	    	    memmove(buf, p, l);
    	    	    buf[l] = ':';
    	    	    memcpy(buf+l+1, mm, strlen(mm));
    	    	}
    	    }
    	}
    	fprintf(ofp, "%s", buf);
    }
    if (oname)
    	fclose(ofp);
    if (iname)
    	fclose(fp);
}



int main(int argc, char *argv[])
{
    int     c;
    int     i;
    char    *p;
    char    tmp[2048];
    char    *name  = NULL;
    char    *oname = NULL;

    for (i = 1; i < argc; i++) {
    	p = argv[i];
    	if (*p == '-') {
    	    p++;
    	    c = *p++;
    	    c = toupper(c);
    	    switch (c) {
    	    case 'O':
    	    	oname = strdup(p);
    	    	break;
    	    case '?':
    	    	usage();
    	    default:
    	    	err_exit("�I�v�V��������������(%s)\n", argv[i]);
    	    }
    	} else if (name == NULL) {
    	    name = p;
    	} else {
    	    err_exit("�t�@�C��������������\n");
    	}
    }
    if (name == NULL) {
    	// �W�����͂��肾����A�G���[�`�F�b�N������_���I
    	//err_exit("�t�@�C�������w�肳��Ă��Ȃ�\n");
    }
    tmp[0] = 0;
    if (oname) {
    	if (oname[0] == 0 || (name && stricmp(name,oname) == 0)) {
    	    tmpnam(tmp);
    	    oname = tmp;
    	}
    }
    BccErrMsg2vc(name, oname);
    if (tmp[0] && name) {
    	char *p = calloc(1, strlen(name)+16);
    	if (p) {
    	    sprintf(p, "%s.bak", name);
    	    remove(p);
    	    rename(name, p);
    	    rename(tmp, name);
    	}
    }
    return 0;
}
