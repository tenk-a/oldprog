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
    人によって違う定義をまとめてある。
*/
typedef enum { FALSE, TRUE } boolean;
typedef enum { NOERR, ERROR } err_t;

typedef void interrupt (*intr_t)();

