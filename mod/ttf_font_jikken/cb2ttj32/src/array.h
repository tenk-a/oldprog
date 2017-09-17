#ifndef	_INC_NTUTTF
#include "ntuttf.h"
#endif

class array : public CObject
     {
public:
     array() : CObject() {len =	0; data	= NULL;	}
     ~array() {	_ffree(data); }
     short addbyte(unsigned char b);
     unsigned short getlen() { return len; }
     unsigned char __far *getbuf() { return data; }
     short insert(unsigned short s, unsigned char b);
     void remove(unsigned short	s);
     unsigned char operator[](unsigned short s)	{ return (s >= len)?0:data[s]; }
     void clear() { _ffree(data); len =	0; data	= NULL;	}
     short set(unsigned	short s, unsigned char c)
    	   { if	(s >= len) return addbyte(c); else data[s] = c;	return SUCC;}
     void remove(unsigned short	start, unsigned	short size);
private:
     unsigned short len;
     unsigned char __far *data;
     };

struct coor { short x, y, flag;	};

class coorarray: public	CObject
     {
public:
     coorarray() : CObject() {len = 0; data = NULL; }
     ~coorarray() { _ffree(data); }
     short add(struct coor c);
     short add(short x,	short y, short f);
     short getlen() { return len; }
     struct coor __far *getbuf() { return data;	}
     short insert(short	s, struct coor c);
     void remove(short s);
     struct coor operator[](short s)
    	   { if	(s < len) return data[s]; else return nul_item;}
     void clear() { _ffree(data); len =	0; data	= NULL;	}
     short set(short s,	struct coor c)
    	   { if	(s >= len) return add(c); else data[s] = c; return SUCC;}
     short parent;
private:
     short test_exp();
     static struct coor	nul_item;
     short len;	//must less then 10922
     struct coor __far *data;
     };

class bigfirst : public	array
     {
public:
     void addshort(unsigned short s) { array::addbyte(H(s)); array::addbyte(L(s)); }
     void addlong(unsigned long	s) { addshort(HX(s)); addshort(LX(s)); }
     };
