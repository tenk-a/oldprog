//  ttfcnv_a  ver.0.01
//  ttf(jisx0213-shiftJIS) file	=> ttf(unicode)	file

#include    <Be.h>

struct GLYPH{
    uint16  o;	//unicode(output)
    uint16  i;	//chrcode(input)
    uint16  n;	//glyph	no.(input)
    uint32  p;	//glyph	offset(input)
    uint32  l;	//glyph	length
};

int 	make_xfer_tbl(char *xfn);
void	sort_glyph(GLYPH *g,int	s,int e);
bool	search_tbl(FILE	*fp,uint32 tblname,uchar *s);
uint32	set_tbl(uint32 *tp,uchar *dt,uint32 len);
uint32	sumB32(uint32 *p,int len);
void	quit(char *p);

int 	xt[0x10000];	//xfer table
int 	chrs=0;	    	//count	of glyf	for output
uchar	hd[0x1000];
uint32	tsump=0;
uint32	wpos=0;

main(int argc,char **argv)
{
    if(argc!=4)quit("ttfcnv_a xfer-table in-file out-file");
    FILE    *ifp,*ofp;
    int	    tbls;
    GLYPH   *g=NULL;
    uchar   s[256];

    chrs=make_xfer_tbl(argv[1]);
    g=new GLYPH[chrs+1];

    g[0].i=0;g[0].o=0;g[0].n=0;chrs=1;
    for(int i=1;i<0x10000;i++)if(xt[i]){
    	g[chrs].i=i;
    	g[chrs].o=xt[i];
    	chrs++;
    }

printf("sort character\n");
    sort_glyph(g,0,chrs-1); 	//sort GLYPHstruct

printf("open inp-file\n");
    if((ifp=fopen(argv[2],"rb"))==NULL)quit("cannot open inp-file");

    search_tbl(ifp,'hhea',s);
    uint32  len=B_BENDIAN_TO_HOST_INT32(*(uint32*)(s+12));
    fseek(ifp,B_BENDIAN_TO_HOST_INT32(*(uint32*)(s+8)),0);
printf("read hhea\n");
    fread(s,1,len,ifp);
    uint16  hmtc=B_BENDIAN_TO_HOST_INT16(*(uint16*)(s+34));
    search_tbl(ifp,'cmap',s);
    uint32  cml=B_BENDIAN_TO_HOST_INT32(*(uint32*)(s+12));
    fseek(ifp,B_BENDIAN_TO_HOST_INT32(*(uint32*)(s+8)),0);
    uchar   *cm=new uchar[cml];
printf("read cmap\n");
    fread(cm,1,cml,ifp);
    search_tbl(ifp,'loca',s);
    uint32  lcl=B_BENDIAN_TO_HOST_INT32(*(uint32*)(s+12));
    fseek(ifp,B_BENDIAN_TO_HOST_INT32(*(uint32*)(s+8)),0);
    uchar   *lc=new uchar[lcl];
printf("read loca\n");
    fread(lc,1,lcl,ifp);
    uint16  cmtc=B_BENDIAN_TO_HOST_INT16(*(uint16*)(cm+2)); 	    //tables
    uint32  *lcp=(uint32*)lc;
    g[0].p=B_BENDIAN_TO_HOST_INT32(*lcp);
    g[0].l=B_BENDIAN_TO_HOST_INT32(*(lcp+1)-g[0].p);
    for(int j=1;j<chrs;j++){
    	uchar	f=0;
    	g[j].n=0;g[j].p=B_BENDIAN_TO_HOST_INT32(*(uint32*)lc);g[j].l=0;
    	for(int	k=0;k<cmtc;k++){
    	    uchar   *cmp=cm+4+k*8;
    	    if(B_BENDIAN_TO_HOST_INT16(*(uint16*)(cmp+0))!=3)continue;	//pid
    	    if(B_BENDIAN_TO_HOST_INT16(*(uint16*)(cmp+2))!=2)continue;	//eid(sjis)
    	    cmp=cm+B_BENDIAN_TO_HOST_INT32(*(uint32*)(cmp+4));
    	    if(B_BENDIAN_TO_HOST_INT16(*(uint16*)(cmp+0))!=4)continue;	//tbl-fmt
    	    uint16  sc=B_BENDIAN_TO_HOST_INT16(*(uint16*)(cmp+6));

    	    uint16  *scp=(uint16*)(cmp+sc+16);
    	    uint16  *ecp=(uint16*)(cmp+14);
    	    uint16  *idp=(uint16*)(cmp+sc*2+16);
    	    uint16  *irp=(uint16*)(cmp+sc*3+16);
    	    uint16  *gap=(uint16*)(cmp+sc*4+16);
    	    while(*scp!=0xffff){
    	    	if((B_BENDIAN_TO_HOST_INT16(*scp)<=g[j].i)&&(g[j].i<=B_BENDIAN_TO_HOST_INT16(*ecp))){
    	    	    if(*irp){
    	    	    	g[j].n=B_BENDIAN_TO_HOST_INT16(*(irp+(B_BENDIAN_TO_HOST_INT16(*irp)/2)+(g[j].i-B_BENDIAN_TO_HOST_INT16(*scp))))+(int16)B_BENDIAN_TO_HOST_INT16(*idp);
    	    	    }else{
    	    	    	g[j].n=g[j].i+(int16)B_BENDIAN_TO_HOST_INT16(*idp);
    	    	    }
    	    	    lcp=(uint32*)lc+g[j].n;
    	    	    g[j].p=B_BENDIAN_TO_HOST_INT32(*lcp);
    	    	    g[j].l=B_BENDIAN_TO_HOST_INT32(*(lcp+1))-g[j].p;
//printf("gno=%04x  i=%04x o=%04x p=%08x l=%08x\n",g[j].n,g[j].i,g[j].o,g[j].p,g[j].l);
    	    	    f=1;
    	    	    break;
    	    	}
    	    	scp++;ecp++;idp++;irp++;
    	    }
    	    if(f)break;
    	}
    }
    delete  cm;
    delete  lc;

printf("xfer start\n");
    fseek(ifp,0,0);
    fread(hd,1,12,ifp);
    tbls=B_BENDIAN_TO_HOST_INT16(*(uint16*)(hd+4));
    fread(hd+12,1,16*tbls,ifp);
    uint32  hdl=12+16*tbls;
printf("open for output\n");
    if((ofp=fopen(argv[3],"wb"))==NULL)quit("cannot open output-ttf-file");
    wpos=fwrite(hd,1,hdl,ofp);	    //toriaezu
    uint32  *tp=(uint32*)(hd+12);
    for(int i=0;i<tbls;i++){
    	uint32	tblname=B_BENDIAN_TO_HOST_INT32(*tp);
printf("table [%c%c%c%c]   ...   ",(uchar)(tblname>>24),(uchar)(tblname>>16),(uchar)(tblname>>8),(uchar)tblname);

    	if(tblname=='head'){
    	    printf("update\n");
    	    uint32  len=B_BENDIAN_TO_HOST_INT32(*(tp+3));
    	    fseek(ifp,B_BENDIAN_TO_HOST_INT32(*(tp+2)),0);
    	    fread(s,1,len,ifp);
    	    *(uint32*)(s+8)=0;	    	    	    	    	//checkSumAdjustment
    	    tsump=wpos+8;
    	    len=set_tbl(tp,s,len);
    	    wpos+=fwrite(s,1,len,ofp);
    	}else if(tblname=='name'){
    	    printf("update\n");
    	    uint32  nal=B_BENDIAN_TO_HOST_INT32(*(tp+3));
    	    uchar   *na=new uchar[nal+4];
    	    fseek(ifp,B_BENDIAN_TO_HOST_INT32(*(tp+2)),0);
    	    fread(na,1,nal,ifp);
/*
//check
    	    uchar   f=0;
    	    uint16  *nap=(uint16*)(na+6);
    	    for(int j=0;j<B_BENDIAN_TO_HOST_INT16(*(uint16*)(na+2));j++){
    	    	uint16	pid=B_BENDIAN_TO_HOST_INT16(*(nap+0));
    	    	uint16	eid=B_BENDIAN_TO_HOST_INT16(*(nap+1));
    	    	uint16	lid=B_BENDIAN_TO_HOST_INT16(*(nap+2));
    	    	if((pid==1)&&(eid==0)&&(lid==0))f|=1;
    	    	if((pid==3)&&(eid==1))f|=2;
    	    	nap+=6;
    	    }
//change
    	    f^=3;
*/
    	    uint16  *nap=(uint16*)(na+6);
    	    for(int j=0;j<B_BENDIAN_TO_HOST_INT16(*(uint16*)(na+2));j++){
    	    	uint16	pid=B_BENDIAN_TO_HOST_INT16(*(nap+0));
    	    	uint16	eid=B_BENDIAN_TO_HOST_INT16(*(nap+1));
    	    	uint16	lid=B_BENDIAN_TO_HOST_INT16(*(nap+2));
//printf("%x-%x-%x\n",pid,eid,lid);
    	    	if(eid!=0){
    	    	    *(uint16*)(nap+0)=B_HOST_TO_BENDIAN_INT16(3);
    	    	    *(uint16*)(nap+1)=B_HOST_TO_BENDIAN_INT16(1);
    	    	}
    	    	nap+=6;
    	    }

    	    nal=set_tbl(tp,na,nal);
    	    wpos+=fwrite(na,1,nal,ofp);
    	    delete  na;
    	}else if(tblname=='hhea'){
    	    printf("update\n");
    	    uint32  len=B_BENDIAN_TO_HOST_INT32(*(tp+3));
    	    fseek(ifp,B_BENDIAN_TO_HOST_INT32(*(tp+2)),0);
    	    fread(s,1,len,ifp);
    	    *(uint16*)(s+34)=B_HOST_TO_BENDIAN_INT16(chrs); //numberOfHMetrics
    	    len=set_tbl(tp,s,len);
    	    wpos+=fwrite(s,1,len,ofp);
    	}else if(tblname=='hmtx'){
    	    printf("remake\n");
    	    uint32  hil=B_BENDIAN_TO_HOST_INT32(*(tp+3));
    	    uchar   *hi=new uchar[hil+4];
    	    fseek(ifp,B_BENDIAN_TO_HOST_INT32(*(tp+2)),0);
    	    fread(hi,1,hil,ifp);
//remake
    	    uint32  hml=chrs*4;
    	    uchar   *hm=new uchar[hml];
    	    uint32  *hmp=(uint32*)hm;
    	    for(int j=0;j<chrs;j++){
    	    	if(g[j].n<hmtc){
    	    	    *(hmp++)=*((uint32*)hi+g[j].n);
    	    	}else{
    	    	    *(hmp++)=*((uint32*)hi+hmtc-1);
//printf("no=%x	: default hmtx\n",j);
    	    	}
    	    }

    	    delete  hi;
    	    hml=set_tbl(tp,hm,hml);
    	    wpos+=fwrite(hm,1,hml,ofp);
    	    delete  hm;
    	}else if(tblname=='maxp'){
    	    printf("update\n");
    	    uint32  len=B_BENDIAN_TO_HOST_INT32(*(tp+3));
    	    fseek(ifp,B_BENDIAN_TO_HOST_INT32(*(tp+2)),0);
    	    fread(s,1,len,ifp);
    	    *(uint16*)(s+4)=B_HOST_TO_BENDIAN_INT16(chrs);  //numGlyphs
    	    len=set_tbl(tp,s,len);
    	    wpos+=fwrite(s,1,len,ofp);
    	}else if(tblname=='cmap'){
    	    printf("create\n");
    	    uint    sc,sr,es;
    	    uint16  *ecp,*scp,*idp,*irp	/*,*gap*/;
    	    sc=1;sr=1;es=-1;
//printf("chrs=%x\n",chrs);
    	    for(int j=1;j<chrs;j++)if((g[j].o)&&((g[j].o==1)||(g[j].o-g[j-1].o!=1)))sc++;
//printf("segcount=%04x\n",sc);
    	    if(sc>0x1ffc)quit("cmap_segcount is too big\n");
    	    while(sr<sc){sr<<=1;es++;}
    	    sc*=2;
    	    uint    cml=4+8+(sc*4+16);
    	    uchar   *cm=new uchar[cml];
//printf("cml=%x\n",cml);

    	    *(uint16*)(cm+0)=B_HOST_TO_BENDIAN_INT16(0);    	//version
    	    *(uint16*)(cm+2)=B_HOST_TO_BENDIAN_INT16(1);    	//tables=1
    	    *(uint16*)(cm+4)=B_HOST_TO_BENDIAN_INT16(3);    	//P-ID=3
    	    *(uint16*)(cm+6)=B_HOST_TO_BENDIAN_INT16(1);    	//E-ID=1(unicode)
    	    *(uint32*)(cm+8)=B_HOST_TO_BENDIAN_INT32(12);   	//offset
    	    uchar   *cmp=cm+12;
    	    *(uint16*)(cmp+0)=B_HOST_TO_BENDIAN_INT16(4);   	//format=4(Segment mapping)
    	    *(uint16*)(cmp+2)=B_HOST_TO_BENDIAN_INT16(sc*4+16);//length
    	    *(uint16*)(cmp+4)=B_HOST_TO_BENDIAN_INT16(0);   	//version
    	    *(uint16*)(cmp+6)=B_HOST_TO_BENDIAN_INT16(sc);  	//segCountX2
    	    *(uint16*)(cmp+8)=B_HOST_TO_BENDIAN_INT16(sr);  	//searchRange
    	    *(uint16*)(cmp+10)=B_HOST_TO_BENDIAN_INT16(es); 	//entrySelector
    	    *(uint16*)(cmp+12)=B_HOST_TO_BENDIAN_INT16(sc-sr);	//rangeShift
    	    ecp=(uint16*)(cmp+14);  	    	    	    //endCount[segCount]
    	    *(uint16*)(cmp+14+sc)=B_HOST_TO_BENDIAN_INT16(0);	//reserved
    	    scp=(uint16*)(cmp+16+sc);	    	    	    //startCount[segCount]
    	    idp=(uint16*)(cmp+16+sc*2);	    	    	    //idDelta[segCount]
    	    irp=(uint16*)(cmp+16+sc*3);	    	    	    //idRangeOffset[segCount]

    	    for(int j=1;j<chrs;j++){
    	    	if(g[j].o==0)continue;
    	    	*(scp++)=B_HOST_TO_BENDIAN_INT16(g[j].o);
    	    	*(idp++)=B_HOST_TO_BENDIAN_INT16(j-g[j].o);
    	    	*(irp++)=B_HOST_TO_BENDIAN_INT16(0);
    	    	while(g[j+1].o-g[j].o==1)j++;
    	    	*(ecp++)=B_HOST_TO_BENDIAN_INT16(g[j].o);
    	    }
    	    *(scp++)=B_HOST_TO_BENDIAN_INT16(0xffff);
    	    *(idp++)=B_HOST_TO_BENDIAN_INT16(1);
    	    *(irp++)=B_HOST_TO_BENDIAN_INT16(0);
    	    *(ecp++)=B_HOST_TO_BENDIAN_INT16(0xffff);

    	    cml=set_tbl(tp,cm,cml);
    	    wpos+=fwrite(cm,1,cml,ofp);
    	    delete  cm;
    	}else if(tblname=='loca'){
    	    printf("create\n");
    	    uint32  lcl=4*(chrs+1);
    	    uchar   *lc=new uchar[lcl];
    	    uint32  *lcp=(uint32*)lc;
    	    uint32  p=0;
    	    for(int j=0;j<chrs;j++){
    	    	*(lcp++)=B_HOST_TO_BENDIAN_INT32(p);
    	    	p+=g[j].l;
    	    }
    	    *lcp=B_HOST_TO_BENDIAN_INT32(p);
    	    lcl=set_tbl(tp,lc,lcl);
    	    wpos+=fwrite(lc,1,lcl,ofp);
    	    delete  lc;
    	}else if(tblname=='glyf'){
    	    printf("remake\n");
    	    uint32  gil=B_BENDIAN_TO_HOST_INT32(*(tp+3));
    	    uchar   *gi=new uchar[gil+4];
    	    fseek(ifp,B_BENDIAN_TO_HOST_INT32(*(tp+2)),0);
    	    fread(gi,1,gil,ifp);

    	    uint32  gfl=0;
    	    for(int j=0;j<chrs;j++)gfl+=g[j].l;
    	    uchar   *gf=new uchar[gfl+4];

    	    uchar   *gfp=gf;
    	    for(int j=0;j<chrs;j++){
    	    	memcpy(gfp,gi+g[j].p,g[j].l);
    	    	if((*gfp)&0x80){
//printf("conposite glyf .. glfno=%04x\n",j);
    	    	    uint16  gn=B_BENDIAN_TO_HOST_INT16(*((uint16*)gfp+6));
    	    	    for(uint16 k=0;k<chrs;k++)if(g[k].n==gn){
    	    	    	*((uint16*)gfp+6)=B_HOST_TO_BENDIAN_INT16(k);
    	    	    	break;
    	    	    }
    	    	}
    	    	gfp+=g[j].l;
    	    }

    	    delete  gi;
    	    gfl=set_tbl(tp,gf,gfl);
    	    wpos+=fwrite(gf,1,gfl,ofp);
    	    delete  gf;
    	}else{
    	    printf("copy\n");
    	    uint32  ttl=B_BENDIAN_TO_HOST_INT32(*(tp+3));
    	    uchar   *tt=new uchar[ttl+4];
    	    fseek(ifp,B_BENDIAN_TO_HOST_INT32(*(tp+2)),0);
    	    fread(tt,1,ttl,ifp);
    	    ttl=set_tbl(tp,tt,ttl);
    	    wpos+=fwrite(tt,1,ttl,ofp);
    	    delete  tt;
    	}
    	tp+=4;
    }
    fclose(ifp);

printf("write table-header\n");
    fseek(ofp,0,0);
    fwrite(hd,1,hdl,ofp);

    uint32  sum=0;
    for(uint i=0;i<hdl;i+=4)sum+=B_BENDIAN_TO_HOST_INT32(*(hd+i));
    for(int i=0;i<tbls;i++)sum+=B_BENDIAN_TO_HOST_INT32(*(hd+12+i*16+4));
    sum=0xb1b0afba-sum;
    *(uint32*)s=B_HOST_TO_BENDIAN_INT32(sum);	//sum adj. in 'head'
    fseek(ofp,tsump,0);
    fwrite(s,1,4,ofp);

    fclose(ofp);
    quit("");
}

int make_xfer_tbl(char *xfn)
{
    FILE    *xfp;
    char    s[256];
    int	    chrs=0;

    for(int i=0;i<0x10000;i++)xt[i]=0;

    for(uint sjis=0x20;sjis<0x7f;sjis++){xt[sjis]=sjis;chrs++;}
    for(uint sjis=0xa0;sjis<0xe0;sjis++){xt[sjis]=sjis-0xa0+0xff60;chrs++;}

    if((xfp=fopen(xfn,"r"))==NULL)quit("cannot open xfer-table");
    while(fgets(s,255,xfp)){
    	if(!strncmp(s,"[end]",5))break;
    	if((*s!='1')&&(*s!='2'))continue;
    	int 	uni,sjis;
    	char	*p;
    	p=strstr(s,"s-");   sscanf(p+2,"%x",&sjis);
    	p=strstr(s,"u-");   sscanf(p+2,"%x",&uni);
    	if((uni==0x0000)||(uni==0xffff))continue;
    	xt[sjis]=uni;
    	chrs++;
    }
    fclose(xfp);
    return(chrs);
}

void sort_glyph(GLYPH *g,int s,int e)
{
    GLYPH   d;
    for(int i=s;i<e;i++){
    	int m=i;
    	for(int	j=i+1;j<=e;j++){
    	    if(g[m].o>g[j].o)m=j;
    	}
    	if(m!=i){
    	    memcpy((void*)&d,(void*)(g+m),sizeof(GLYPH));
    	    memcpy((void*)(g+m),(void*)(g+i),sizeof(GLYPH));
    	    memcpy((void*)(g+i),(void*)&d,sizeof(GLYPH));
    	}
    }
}

bool search_tbl(FILE *fp,uint32	tblname,uchar *s)
{
    bool    f=false;
    fseek(fp,0,0);
    fread(s,1,12,fp);
    int	    tbls=B_BENDIAN_TO_HOST_INT16(*(uint16*)(s+5));
    for(int i=0;i<tbls;i++){
    	fread(s,1,16,fp);
    	if(B_BENDIAN_TO_HOST_INT32(*(uint32*)s)==tblname){f=true;break;}
    }
if(!f){printf("not found\n");exit(1);}
    return(f);
}

uint32 set_tbl(uint32 *tp,uchar	*dt,uint32 len)
{
    while(len&3)dt[len++]=0;
    uint32  sum=sumB32((uint32*)dt,len);
    *(++tp)=B_HOST_TO_BENDIAN_INT32(sum);   //sum
    *(++tp)=B_HOST_TO_BENDIAN_INT32(wpos);  //offset
    *(++tp)=B_HOST_TO_BENDIAN_INT32(len);   //length
    return(len);
}

uint32 sumB32(uint32 *p,int len)
{
    uint32  sum=0;
    len=len/4;
    while(len--)sum+=B_BENDIAN_TO_HOST_INT32(*(p++));
    return(sum);
}

void quit(char *p)
{
    if(*p)printf("error : %s\n",p);
    else printf("complete\n");
    exit(0);
}
