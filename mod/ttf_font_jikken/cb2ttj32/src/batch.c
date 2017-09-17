#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <process.h>
#if 1
#define USE_WIN
#include <windows.h>
#endif

typedef struct slist_t {
	struct slist_t	*link;
	char			*s;
} slist_t;


void *callocE(size_t a, size_t b)
{
	/* エラーがあれば即exitの calloc() */
	void *p;

	if (a== 0 || b == 0)
		a = b = 1;
	p = calloc(a,b);
	if (p == NULL) {
		printf("not enough memory\n");
		exit(1);
	}
	return p;
}


char *strdupE(char *s)
{
	/* エラーがあれば即exitの strdup() */
	char *p;

	if (s == NULL)
		return callocE(1,1);
	p = callocE(1,strlen(s)+8);	//拡張子付け替えのために余分にメモリを取る
	strcpy(p, s);
	return p;
}


slist_t *slist_add(slist_t **p0, char *s)
{
	/* 文字列のリストを追加		*/
	slist_t* p;

	p = *p0;
	if (p == NULL) {
		p = callocE(1, sizeof(slist_t));
		p->s = strdupE(s);
		*p0 = p;
	} else {
		while (p->link != NULL) {
			p = p->link;
		}
		p->link = callocE(1, sizeof(slist_t));
		p = p->link;
		p->s = strdupE(s);
	}
	return p;
}


void slist_free(slist_t **p0)
{
	/* 文字列のリストを削除	*/
	slist_t *p, *q;

	for (p = *p0; p; p = q) {
		q = p->link;
		if (p->s)
			free(p->s);
		free(p);
	}
	*p0 = NULL;
}


char *fname_baseName(char *adr)
{
	char *p;

	p = adr;
	while (*p != '\0') {
		if (*p == ':' || *p == '/' || *p == '\\')
			adr = p + 1;
		p++;
	}
	return adr;
}


char *fname_addExt(char filename[], char *ext)
{
	if (strrchr(fname_baseName(filename), '.') == NULL) {
		strcat(filename,".");
		strcat(filename, ext);
	}
	return filename;
}



static void Usage(void)
{
	printf("Usage> BATCH [-opts] batchfile [start [end]]\n"
		"startからendまで、batchfile の 内容を繰り返し１つのbatchtmp.batを\n"
		"生成して実行. このとき %%1 をその値に置き換える.\n"
		"  -n      batchtmp.bat を生成するだけ\n"
		"  -bNAME  batchtmp.bat を NAME にする\n"
	);
	exit(1);
}



int main(int argc, char *argv[])
{
	int c, i, sf=0, ef=0, start = 0, end = 13502, nflg = 0;
	char    buf[4100];
	char	*batname = "batchtmp.bat";
	char	*name = NULL;
	char	*p;
	slist_t	*sl, *slist = NULL;
	FILE	*fp;
	int		tim;

	if (argc < 2) {
		Usage();
		return 1;
	}

  #ifdef USE_WIN
	tim = timeGetTime();
  #endif

	for (i = 1; i < argc; i++) {
		p = argv[i];
		if (*p == '-') {
			p++;
			c = *p++;
			c = toupper(c);
			switch (c) {
			case 'N':
				nflg = (*p != '-');
				break;
			case 'B':
				if (*p)
					batname = strdupE(p);
				break;
			case '\0':
				break;
			case '?':
				Usage();
			default:
				printf("Incorrect command line option : %s\n", argv[i]);
				return 1;
			}
		} else if (name == NULL) {
			name = strdupE(p);
			fname_addExt(name, "bat");
		} else if (sf == 0) {
			sf = 1;
			start = strtol(p, 0, 0);
		} else if (ef == 0) {
			ef = 1;
			end = strtol(p, 0, 0);
		}
	}

  #if 1
	if (end > 13502) {
		end = 13502;
	}
  #endif

	fp = fopen(name, "rt");
	if (fp == NULL) {
		printf("file `%s' not found.\n", name);
		return 1;
	}
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		slist_add(&slist, buf);
	}
	fclose(fp);

	/* バッチ作成 */
	fp = fopen(batname, "wt");
	if (fp == NULL) {
		printf("file `%s' open error.\n", batname);
		return 1;
	}
	fprintf(fp, "@echo off\n");
	for (i = start; i <= end; i++) {
		char nb[32];
		int l;

		sprintf(nb, "%d", i);
		l = strlen(nb);
		fprintf(fp, "echo %s/%d\n", nb, end);
		for (sl = slist; sl; sl = sl->link) {
			strcpy(buf, sl->s);
			p = buf;
			while ((p = strstr(p, "%1")) != NULL) {
				if (l != 2) {
					memmove(p+l, p+2, strlen(p+2)+1);
				}
				memcpy(p, nb, l);
				p += l;
			}
			fprintf(fp, "%s", buf);
		}
	}
	fclose(fp);
	slist_free(&slist);

	if (nflg == 0) {
		system(batname);
	  #ifdef USE_WIN
		{
			long sec = (timeGetTime() - tim + 999) / 1000;
			printf("time %ds.(%dmin.)\n", sec, (sec+59)/60);
		}
	  #endif
	}
	return 0;
}

