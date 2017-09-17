/*------------------------------------------------------*/
/*�X���[���E���f���� far�������g�p��ړI�Ƃ������C�u����*/
/*                                                      */
/*------------------------------------------------------*/

#ifndef _FAR_H_
#define _FAR_H_

#define V   void
#define I   int
#define L   long
#define B   unsigned char
#define W   unsigned
#define D   unsigned long
#define F   far
#define N   near
#define FC				/*far�R�[���֐��Ȃ� far ��ݒ�*/

/*------------------------------------------------------*/
/* �������E������                                       */
/*------------------------------------------------------*/
/* �|�C���^�֌W */

//B F *str_fptohp(p);

/* ������֌W */
B F * FC str_memcpy(B F *dst, B F *src, W sz);
B F * FC str_memcpyl(B F *dst, B F *src, L sz);
B F * FC str_memmove(B F *dst, B F *src, W sz);
B F * FC str_memcmp(B F *s1, B F *s2, W sz);
B F * FC str_memchr(B F *str, I chr, W sz);
B F * FC str_memset(B F *str, I chr, W sz);

B F * FC str_end(B F *cs);
W     FC str_len(B F *cs);
B F * FC str_cpy(B F *s1, B F *cs);
B F * FC str_ncpy(B F *s1, B F *cs, W sz);
B F * FC str_pcpy(B F *s1, B F *s2);
B F * FC str_cat(B F *s1, B F *cs2);
B F * FC str_ncat(B F *s1, B F *cs2,W sz2);
I     FC str_cmp(B F *cs1, B F *cs2);
I     FC str_ncmp(B F *cs1, B F *cs2, W sz2);
I     FC str_icmp(B F *cs1, B F *cs2);
I     FC str_incmp(B F *cs1, B F *cs2, W sz2);
B F * FC str_chr(B F *cs, I chr);
B F * FC str_rchr(B F *cs, I chr);
L     FC str_atol(B F *cs);
L     FC str_htol(B F *cs);
B F * FC str_printfN(B F *buf, B N *fmt, ...);
B F * FC str_printfF(B F *buf, B F *fmt, ...);
#undef  str_printf
#define str_printf str_printfN
B F * FC str_nprintfN(B F *buf, W sz, B N *fmt, ...);
B F * FC str_nprintfF(B F *buf, W sz, B F *fmt, ...);
B F * FC str_tab(B F *s, I tabsiz, I tabsizmin, I flg);
B F * FC str_detab(B F *s,B F * cs, I tabsiz);
B F * FC str_upr(B F *s);
B F * FC str_lwr(B F *s);
//B F * FC str_str(B F *cs, B F *ct);
//B F * FC str_tok(B F *s, B F *ct);
//B F * FC str_pbrk(B F *cs1, B F *cs2);
//I     FC str_htoi(B F *cs);
//I     FC str_atoi(B F *cs);
//L     FC str_tol(B F *cs, B F* F* p, I n);
//D     FC str_toul(B F *cs, B F* F* p, I n);
//B F * FC str_rev(B F *s);
//B F * FC str_scanfN(B F *buf, B N *fmt, ...);
//B F * FC str_scanfF(B F *buf, B F *fmt, ...);

/* ������. MS�S�p�l���� */
extern I far str_flagj;
I     FC str_setflagj(I sw);
I     FC str_getflagj(V);
B F * FC str_uprj(B F *s);
B F * FC str_lwrj(B F *s);
B F * FC str_chrj(B F *cs, I chr);
B F * FC str_rchrj(B F *cs, I chr);
I     FC str_cmpj(B F *cs1, B F *cs2);
I     FC str_ncmpj(B F *cs1, B F *cs2, W sz);
I     FC str_icmpj(B F *cs1, B F *cs2);
I     FC str_incmpj(B F *cs1, B F *cs2, W sz);
//B F * FC str_strj(B F *cs, B F *ct);
//B F * FC str_tokj(B F *s, B F *ct);
//B F * FC str_pbrkj(B F *cs1, B F *cs2);
//B F * FC str_npbrkj(B F *cs1, B F *cs2);

I     FC str_chknilj(B F *s, W sz);
		/*����sz�̍s����\0���s���S�p�����邩�𒲂ׂ�*/
W     FC str_delnilj(B F *s, W sz);
	/*����sz�̍s����\0,�s���S�p���폜*/
		/*ret �ϊ���̻���*/
B F * FC str_linetrimj(B F *s);
		/*�����񖖂̋�(���۰٥�S����)���폜.*/
		/*����'\n'��'\0'�̒��O�ɂ���΋󔒍폜��t��������*/
B F * FC str_linencut(B F *s, W sz);
    	/*�s��sz�޲Ă��폜�B�����񎩑̂�����������. \n�l�� */

//B F * FC str_nthchk(B F *s);
//B F * FC str_zen(B F *s, B F *cs, W mode);
//B F * FC str_han(B F *s, B F *cs, W mode);
//B F * FC str_zenhan(B F *s, B F *cs, W mode);

/*------------------------------------------------------*/
/* �������E�A���P�[�g�֌W                               */
/*------------------------------------------------------*/
I     FC alc_pfree(W pseg);
W     FC alc_palloc(W psz);
//W     FC alc_prealloc(W pseg, W psz);
I     FC alc_free(V F *m);/*alc_(m|c|z)alloc(z)�Ŋm�ۂ��������������*/
V F * FC alc_malloc(L sz);
V F * FC alc_mallocz(L sz);
//V F * FC alc_realloc(V F *p, L sz);
//V F * FC alc_reallocz(V F *p, L sz);
//V F * FC alc_calloc(L n, W sz);
//V F * FC alc_callocz(L n, W sz);
//V F * FC alc_strdup(B F *s);
//W     FC alc_dosmaxfree(V);
		/*DOS�̍ő�󂫗̈�̃T�C�Y�𓾂�*/

//W     FC alc_resizemax(W psz);/*sz�͍ő��para����*/
	/*alc_���Ǘ��������؂�sz�ɋߕt���悤��DOS��蓾�� */
//W     FC alc_resizemin(W psz);/*sz�͍ŏ���para����*/
	/*alc_���Ǘ����Ă������؂��Œ�sz�����c����DOS�ɕԂ�*/

/* exec */
//I     FC commandcom(B F *s);

/*------------------------------------------------------*/
/* �t�@�C���֌W                                         */
/*------------------------------------------------------*/
/*  �t�@�C���E�n���h���n */
I     FC fil_open(B F* fname, W acs);
I     FC fil_creat(B F* fname, W attr);
I     FC fil_creatnew(B F*fname, W attr);
I     FC fil_close(I hdl);
W     FC fil_read(I hdl, B F*buf, W sz);
W     FC fil_write(I hdl, B F*buf, W sz);
//L     FC fil_lread(I hdl, B F*buf, L sz);
//L     FC fil_lwrite(I hdl, B F*buf, L sz);
I     FC fil_seek(I hdl, D ofs, I pos);
I     FC fil_dup(I hdl);
I     FC fil_dup2(I hdl1, I hdl2);
//I     FC fil_tmpopen(B F *path, W attr);
//I     FC fil_settime(I hdl, D datetime);
//D     FC fil_gettime(I hdl);
W     FC fil_printfN(I hdl, B N *fmt, ...);
W     FC fil_printfF(I hdl, B F *fmt, ...);
#define fil_printf fil_printfN

/* �t�@�C�����n */
I     FC fil_delete(B F *fname);
I     FC fil_rename(B F *oldname, B F *newname);
I     FC fil_getattr(B F *fname);
I     FC fil_setattr(B F *fname, W attr);
//I     FC fil_settime(B F *fname, D datetime);
//D     FC fil_gettime(B F *fname);

/* �t�@�C����������n */
B F * FC fil_fullpath(B F *path, B F *fullpath);
//B F * FC fil_abspath(B F *path, B F *abspath);
//B F * FC fil_basename(B F *path);
//B F * FC fil_chgext(B F *fname, B F *ext);
//B F * FC fil_addext(B F *fname, B F *ext);
I     FC fil_wccmp(B F *key, B F *str, W flg);

/* �f�B�X�N�n */
I     FC fil_setdrive(I drv);
I     FC fil_getdrive(V);
//I   fil_dskfree(I drv);
//I   fil_verify(I sw);

/* �f�B���N�g���n */
I     FC fil_mkdir(B F *dir);
I     FC fil_rmdir(B F *dir);
I     FC fil_setcdir(B F *dir);
I     FC fil_getcdir(I drv, B F *dir);
I     FC fil_setcwd(B F *dir);
#define fil_chdir(x)    fil_setcwd(x)
B F * FC fil_getcwd(B F *dir);

/* �f�B���N�g���E�G���g�� */
typedef struct fil_find_t {
    B sattr;
    B sdrv;
    B sname[8];
    B sext[3];
    B rsv[8];
    B attr;
    W time;
    W date;
    D size;
    W name;
} fil_find_t;

typedef struct fil_dir_t {
    B name[13];
    B attr;
    W time;
    W date;
    D size;
    W work;
} fil_dir_t;

//I     FC fil_findfirst(B F *srchname, W atr, fil_find_t F *fndbuf);
//I     FC fil_findnext (fil_find_t F *findbuf);
//I     FC fil_readdir(B F *srchname, W attr,fil_dir_t F *buf,
//            W bufcnt,fil_find_t fndbuf);
//I     FC fil_readdir2(B F* F* srchnames, W srchnamecnt, W attr,
//             fil_dir_t F *buf,W bufcnt,fil_find_t fndbuf);
//B F * FC fil_de2fname(B F *dename, B F *fname);
//B F * FC fil_f2dename(B F *fname, B F *dename);

/* �o�b�t�@�����O�n */
typedef struct filb_t {
    /*-----
    W attr;
    W time;
    W date;
    D size;
    -----*/
    I hundle;
    W flags;    /*0:R 1:W  7:B|T*/
    D curpos;
    D buffer;
    W bufpos;
    W bufend;
    W eof;
    W error;
    W crflg;
} filb_t;

//filb_t F * FC filb_open(B F* fname, W acs, B F *buf, W sz);
//filb_t F * FC filb_creat(B F* fname, W attr, B F *buf, W sz);
//filb_t F * FC filb_creatnew(B F*fname, W attr, B F *buf, W sz);
//filb_t F * FC filb_topen(B F* fname, W acs, B F *buf, W sz);
//filb_t F * FC filb_tcreat(B F* fname, W attr, B F *buf, W sz);
//filb_t F * FC filb_tcreatnew(B F*fname, W attr, B F *buf, W sz);
//filb_t F * FC filb_dup(filb_t F *fp, filb_t F *fp2);
//filb_t F * FC filb_dup2(filb_t F *fp, filb_t F *fp);
//I     FC filb_close(filb_t F *fp);
//I     FC filb_seek(filb_t F *fp, D ofs, I pos);
//W     FC filb_read(filb_t hdl, B F *buf, W sz);
//W     FC filb_readln(filb_t F *fp, B F *buf, W sz);
//I     FC filb_readc(filb_t F *fp);
//W     FC filb_reads(filb_t F *fp, B F *buf, W sz);
//W     FC filb_scanfN(filb_t F *fp, B F *fmt, ...);
//W     FC filb_scanfF(filb_t F *fp, B F *fmt, ...);
//W     FC filb_write(filb_t F *fp, B F *buf, W sz);
//W     FC filb_writln(filb_t F *fp, B F *buf, W sz);
//I     FC filb_writec(filb_t F *fp, I c);
//W     FC filb_writes(filb_t F *fp, B F *buf);
//W     FC filb_printfN(filb_t F *fp, B N *fmt, ...);
//W     FC filb_printfF(filb_t F *fp, B F *fmt, ...);
//#define filb_printf filb_printfN
//W     FC filb_flush(filb_t F *fp);

/*------------------------------------------------------*/
/*                                                      */
/*------------------------------------------------------*/
#undef N
#undef F
#undef D
#undef W
#undef B
#undef L
#undef I
#undef V
#endif
