#ifndef MyCalcH
#define MyCalcH

class MyCalc {
	int			ch;
	char		*ch_p;

	unsigned	sym;
	double		sym_val;

	int			vari_flg;
	unsigned	vari_no;
	double		vari[26];
	double		ans;

	void   __fastcall MyCalc::ch_get();
	void   __fastcall MyCalc::sym_get();
	double __fastcall MyCalc::expr1();
	double __fastcall MyCalc::expr2();
	double __fastcall MyCalc::expr3();
	double __fastcall MyCalc::expr4();
	double __fastcall MyCalc::expr5();
	double __fastcall MyCalc::expr6();
	double __fastcall MyCalc::expr7();
	double __fastcall MyCalc::expr8();
	double __fastcall MyCalc::expr9();
	double __fastcall MyCalc::expr10();

public:
	char   		*errmsg;
	double		Val;
	double	__fastcall	Expr(char *ExprStr);
};

#endif
