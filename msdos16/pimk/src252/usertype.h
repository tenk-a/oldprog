/*
    USERTYPE.H :    User definication type header
*/

/*
    short-unsigned type definications
*/
typedef signed char schar;
/**/
typedef unsigned char	uchar;
typedef unsigned int	uint;
typedef unsigned long	ulong;
/**/
typedef unsigned char	Ibyte;
typedef unsigned char	ibyte;
typedef unsigned int	Iword;
typedef unsigned int	iword;
typedef unsigned long	Idword;
typedef unsigned long	idword;

/*
    l‚É‚æ‚Á‚Äˆá‚¤’è‹`‚ğ‚Ü‚Æ‚ß‚Ä‚ ‚éB
*/
typedef enum { FALSE, TRUE } boolean;
typedef enum { NOERR, ERROR } err_t;

typedef void interrupt (*intr_t)();

