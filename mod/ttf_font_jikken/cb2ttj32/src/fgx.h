#define	__FG_H__
// This	first part is copy from	FONT support by	ETan Co.
// But change all int to short to ensure protable for 32bit envirement
// Not all useful in our application

#define	MaxChSize 4096
#define	MinChSize 24

#define	MaxVectorSize 4096
#define	MinVectorSize  513
#define	MaxVectorRange 512
#define	MaxBitmapSize  512
#define	MinBitmapSize	24


/* ctype : font	style for Chinese */
#define	Ft_Ming	    	  0 	  /* Sung */
#define	Ft_Kai	    	  1
#define	Ft_Round    	  2
#define	Ft_Black    	  3
#define	Ft_Li	    	  4
#define	Ft_Shing    	  5
#define	Ft_FSung    	  6
/* new style from DynaLab */
#define	Ft_Ming3    	  7 	  /* light */
#define	Ft_Ming7    	  8 	  /* bold */
#define	Ft_Black7   	  9 	  /* bold */

#define	Ft_Xstyle   	 31


/* estyle : font style for English */
#define	Ft_Dyna	    	0x80	    	    	// Ft_Dyna can OR with Ft_F?
/* fixed space font */	    	    	    	//    to get Dyna font
#define	Ft_F0	    	0
#define	Ft_F1	    	1
#define	Ft_F2	    	2
#define	Ft_F3	    	3
/* proportional	space font */
#define	Ft_F4	    	4
#define	Ft_F5	    	5
#define	Ft_F6	    	6
#define	Ft_F7	    	7
#define	Ft_F8	    	8
#define	Ft_F9	    	9
#define	Ft_F10	       10
/* user	English	font */
#define	Ft_F30	       30
#define	Ft_F31	       31

/*
    font mode
    the	following constants can	be ORed	together
    	    	    	    	    	    	    */
#define	Ft_Normal    0x0000 	 /* no attribute */
#define	Ft_Vertical  0x0001 	 /* Vertical layout,
    	    	    	    	    Chinese char rotate	90 degrees ccw
    	    	    	    	    English char remain	unchanged   	 */
#define	Ft_Updown    0x0002 	 /* English word rotate	90 degrees cw
    	    	    	    	    Chinese char remain	unchanged   	 */
#define	Ft_Use24     0x0004 	 /* Use	the default 24 system font from	pattern	card */
#define	Ft_Land	     0x0008 	 /* Landscape font
    	    	    	    	    Both Chinese and English char rotate 90 degree cw */
#define	Ft_Rotate    0x0010 	 /* rotate the char accroding to 'degree' variable
    	    	    	    	    only 0,90,180,270 degrees are available so far */
#define	Ft_Big5	     0x8000 	 /* the	input char use Big5 as internal	code,
    	    	    	    	    otherwise the current internal code	is assumed */

/*
    font attribute
    the	following constant can be ORed together
    	    	    	    	    	    	    */
#define	Ft_Normal    0x0000 	 /* no attribute */
#define	Ft_Udline    0x0001 	 /* Underline,	      no effect	so far */
#define	Ft_Upline    0x0002 	 /* Upperline,	      no effect	so far */
#define	Ft_Dline     0x0004 	 /* Double Underline, no effect	so far */
#define	Ft_G2	     0x0008 	 /* Graph char continue, no effect so far */
#define	Ft_Box	     0x0010 	 /* Box,    	      no effect	so far */
#define	Ft_Reverse   0x0020 	 /* Reverse the	char pattern, 1->0 and 0->1 */
#define	Ft_Bold	     0x0100 	 /* Bold    	    */
#define	Ft_Outline   0x0200 	 /* Outline 	    */
#define	Ft_Italic    0x0400 	 /* Italic, 1/3	skew */
#define	Ft_Shadow    0x0800 	 /* Shadow, 	 no effect so far */
#define	Ft_Sub	     0x4000 	 /* Subscript,	 no effect so far */
#define	Ft_Sup	     0x8000 	 /* Superscript, no effect so far */

/* font	mask
   an integer value from 0..7
   0	: no mask
   1..6	: special pattern mask
   7	: random mask	    	     */

typedef	struct {
    char cstyle;    	     /*	Chinese	font style */
    char estyle;    	     /*	English	font style */
    short xsize;    	       /* font X size in dots */
    short ysize;    	       /* font Y size in dots */
    	    	    	     /*	all variables in Font structure	will remain unchanged except
    	    	    	    	xsize, ysize and yend. xsize and ysize always reflect the
    	    	    	    	return bitmap width and	height in dots */
    short mode;	    	       /* font mode */
    short attr;	    	       /* font attribute */
    short mask;	    	       /* font mask */

    /* the original cgap, lgap are changed to ystart, yend */
    short ystart;   	       /* start	y position of vector data */
    short yend;	    	       /* end y	position of vector data	*/
    	    	    	     /*	0..ysize-1 */

    unsigned char wexp;	     /*	width expanding	factor */
    unsigned char hexp;	     /*	height expanding factor	*/
    unsigned char net;	     /*	net number */

    unsigned char color;     /*	font color */
    short degree;   	       /* rotate degree, ccw direction */
} Font;

/* normal font buffer size in bytes */
#define	FontBufSize1(x)	( (((x)+7)/8*8)	* (x) )

/* font	buffer size with bold, outline and italic */
#define	_fgbo(x)   8	     /*	bold and outline */
#define	_fgit(x)   ((x)/3)   /*	italic */

/* the minimum buffer size for bitmap when Fg_FontGet command is given,
   if size<=128, min. buffer size = 6K,
   if 128<size<=256, min. buffer size =	12K,
   if 256<size>512, min. buffer	size = 44K  	    	    	    */
#define	FontBufSize2(x)	 ( ((x)<=128) ?	6*1024 : (((x)<=256) ? 12*1024 : 44*1024) )

/* Fg function number */
#define	Fg_Reset    	   0x0000    /** available */
#define	Fg_InqSysInfo	   0x0001    /** available */
#define	Fg_InqFontInfo	   0x0002    /** available */
#define	Fg_FontMap1 	   0x0003    /** available */
#define	Fg_FontMap2 	   0x0004    /** available */

#define	Fg_FontGet  	   0x0100    /** available */
#define	Fg_FontGetNext	   0x0101    /** available, for	dyna font only */

typedef	struct {
    short flag;	    	  /* flag=0 Chinese, flag=1 English */
    unsigned char hb;	/* 1st byte of Chinese character or English character
    	    	    	   depend on the flag */
    unsigned char lb;	/* 2nd byte of Chinese character, ignored when flag=1 */
    Font __far *ftp;	  /* pointer to	requested font information */
} FgFont;

/*
    Cache
    	Code Area : The	code of	each char in cache
    	Hit Area  : The	hitting	count of each char in cache
    	Pattern	Area : The pattern of each char	in cache
    	    	    	    	    	    	    	    */
#define	CacheCount 12

/* status */
#define	Cache_Normal   0x00
#define	Cache_Inuse    0x01
/* format */
#define	Cache_Partial  0x01
#define	Cache_All      0x02

typedef	struct {
    /* cache status */
    char status;
    char format;       /* cache	format */
    short emshd;    	 /* EMS	handle for this	cache */
    short pages;    	 /* EMS	pages for this cache */
    /* font used in cache */
    char type;	       /* font type, Ft_Chinese	or Ft_English */
    char style;	       /* font style */
    short xsize;
    short ysize;
    short mode;
    /* structure of cache */
    short maxcp1;   	 /* max. chars in 1st page */
    short maxcp;    	 /* max. chars in normal page */
    short maxcc;    	 /* max. chars in cache	*/
    short hitoff;   	 /* hit	area start offset */
    short patoff;   	 /* pattern area start offset */
    short fbsize;   	 /* fbsize=(xsize+7)/8*ysize  */
    /* chars in	current	cache */
    short avail;    	 /* available chars in cache */
    short hit[8];   	 /* hitting counts in each level */
} Cache;

typedef	struct {
    short ver;	    	  // verion number  180	-> ver 1.80,  182 -> ver 1.82
    	    	    	//  	    	  124 -> ver 1.79s  125	-> ver 1.80s
    short ixhd;	    	  // index buffer EMS handle	 used if  FG /i	/A
    short dyna;	    	  // whether DynaFont is available =1, DynaFont	exist
    Cache __far	*cep;	  // cache of EMS handle    	 used if  FG /i	/CM5,??
    short mixhd;    	  // bitmap autocache EMS handle used if  FG /i	/M??
} FgSysInfo;

typedef	struct {
    char type;	    	/* requested font type,	type=0 Chinese,	type=1 English */
    char style;	    	/* requested font style	*/
    unsigned char __far	*deltax;
    	    	    	/* optional deltax for each ASCII character
    	    	    	   if this is a	proportional English font,
    	    	    	   each	deltax is based	on 128x128 dot bitmap,
    	    	    	   if the requested character is not of	size 128,
    	    	    	   application should adjust the value according
    	    	    	   to the requested size */
    short reserved[4];	  /* reserve for future	use only */
} FgFontInfo;

typedef	struct {
    char fontchar;  /* char 'A'..'Z' */
    char weight;    /* number 1..9 */
    char cstyle;    /* number 0..9 */
} FontMap;

/*
    short FgInit();
    	    	    // judge whether FG	is installed
    	    	    // return 0	:   	FG installed
    	    	    // return nonzero :	FG isn't installed

    short FgCall(Fg_Reset);
    	    	    // set FG to it's initial condition

    short FgCall(Fg_InqSysInfo,	FgSysInfo __far	*);
    	    	    // get the FG  version, ixhd, dyna,	cep and	mixhd

    short FgCall(Fg_InqFontInfo, FgFontInfo __far *);
    	    	    // get the deltax for each ASCII character,
    	    	    // Input:  set FgFontInfo->type  and  FgFontInfo->style
    	    	    // After call:     FgFontInfo->deltax

    short FgCall(Fg_FontGet, FgFont __far *, char __far	*);
    	    	    // the usage statment is at	last part of this file

    short FgCall(Fg_FontGetNext, char __far *);
    	    	    // the usage statment is at	last part of this file

    short FgCall(Fg_FontMap1, FontMap *);
    	    	    // get cstyle from fontchar	and weight
    	    	    // EX:  fontchar='M' weight=5,    after call this function
    	    	    //	    cstyle=0

    short FgCall(Fg_FontMap2, FontMap *);
    	    	    // get fonchar and weight from cstyle
    	    	    // EX:  cstyle=7,	 after call this function
    	    	    //	    fontchar='M' weight=3
    	    	    	    	    	    	    	*/

// This	part is	coded for our application try to work under FG enviroment
// Lin,Y.J. 1993/2/9

class TLfont
     {
public:
     TLfont();
     ~TLfont() {_ffree(FgBuf);}
     short get_font(char hb, char lb = 0); // if lb == 0 means english
     short get_font() //second times for same word
    	   { return FgCall(Fg_FontGet,(char __far *)((FgFont __far *)&fgfont),(char __far *)FgBuf); }
     short get_next()
    	   { return FgCall(Fg_FontGetNext,(char	__far *)FgBuf);	} //font >= 512
     short valid; //true if install FG correctly else false
     unsigned char __far *get_buffer() { return	FgBuf; }
     short get_xsize() { return	font.xsize; }
    	   // In English font, xsize will change during	call, most divide 2
     short get_ystart()	{ return font.ystart; }
     short get_yend() {return font.yend; } // Second times for same word
     //	Setup member
     void set_cstyle(char cstyle) { font.cstyle	= cstyle; }
     void set_estyle(char estyle) { font.estyle	= estyle; }
     void set_size(short x, short y)
    	  { font.xsize = x; font.ysize = y; }
     void set_mode(short mode =	0) { font.mode = mode; }
     void set_mask(short mask =	0) { font.mask = mask; }
     void set_attr(short attr =	0) { font.attr = attr; }
     void set_ystart(short ystart = 0) { font.ystart = ystart; }
     void set_yend(short yend =	-1)
    	  {
    	  if (yend == -1) font.yend = font.ysize - 1; //default
    	  else font.yend = yend;
    	  }
     void set_wexp(unsigned char wexp =	0) { font.wexp = wexp; }
     void set_hexp(unsigned char hexp =	0) { font.hexp = hexp; }
     void set_net(unsigned char	net = 0) { font.net = net; }
     void set_color(unsigned char color	= 1) { font.color = color; }
     void set_degree(short degree = 0) { font.degree = degree; }
private:
     FgFont fgfont;
     Font font;	// pointer by fgfont
     unsigned char __far *FgBuf;
     short FgCall(short	para, char __far *par1 = NULL, char __far *par2	= NULL);
     short (__far *FgSub)();
     };
