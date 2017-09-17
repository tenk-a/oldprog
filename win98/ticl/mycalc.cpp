#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "mycalc.h"

typedef double val_t;
typedef __int64 lint_t;

#define	CC(a,b)	((a)*256+(b))

void __fastcall MyCalc::ch_get()
{
	ch = (unsigned char)*ch_p;
	if (ch)
		ch_p++;
}

void __fastcall MyCalc::sym_get()
{
	do {
		ch_get();
	} while (0 < ch && ch <= 0x20);
	if (isdigit(ch)||ch == '.') {
		char *p;
		sym_val = strtol(ch_p-1, &p, 0);
		if (*p != '.')
			ch_p = p;
		else
			sym_val = strtod(ch_p-1, &ch_p);
		sym = '0';
 #if 0
	} else if (isalpha(ch)) {
		vari_no = toupper(ch) - 'A';
		sym = 'V';
 #endif
	} else {
		sym = ch;
		switch(ch) {
		case '@':
			sym = '0';
			sym_val = ans;
			break;
		case '>':	if (*ch_p == '>')		{ch_p++; sym = CC('>','>');}
					else if (*ch_p == '=')	{ch_p++; sym = CC('>','=');}
					break;
		case '<':	if (*ch_p == '<') 		{ch_p++; sym = CC('<','<');}
					else if (*ch_p == '=')	{ch_p++; sym = CC('<','=');}
					break;
		case '=':	if (*ch_p == '=') {ch_p++; sym = CC('=','=');}
					break;
		case '!':	if (*ch_p == '=') {ch_p++; sym = CC('!','=');}
					break;
		case '&':	if (*ch_p == '&') {ch_p++; sym = CC('&','&');}
					break;
		case '|':	if (*ch_p == '|') {ch_p++; sym = CC('|','|');}
					break;
		case '+':
		case '-':
		case '^':
		case '~':
		case '/':
		case '%':
		case '*':
		case '(':
		case ')':
		case '\0':
			break;
		case '$':
			ch_get();
			if (isalpha(ch)) {
				vari_no = toupper(ch) - 'A';
				sym = 'V';
				break;
			}
			/* not break */
		default:
			errmsg = ("�z�肵�Ă��Ȃ������������ꂽ\n");
		}
	}
}

double __fastcall MyCalc::expr1()
{
	val_t l;

	sym_get();
	l = 0;
	vari_flg = 0;
	if (sym == 'V') {
		l = vari[vari_no];
		vari_flg = 1;
		sym_get();
	} else if (sym == '0') {
		l = sym_val;
		sym_get();
	} else if (sym == '-') {
		l = -expr1();
	} else if (sym == '+') {
		l = expr1();
	} else if (sym == '~') {
		l = ~(lint_t)expr1();
	} else if (sym == '!') {
		l = !(lint_t)expr1();
	} else if (sym == '$') {
		l = ans;
		sym_get();
	} else if (sym == '(') {
		l = expr10();
		if (sym != ')')
			errmsg = ("���ʂ����Ă��Ȃ�");
		sym_get();
	} else {
		errmsg = ("�z�肵�Ă��Ȃ�����");
	}
	return l;
}

double __fastcall MyCalc::expr2()
{
	val_t l,n;

	l = expr1();
	for (; ;) {
		if (sym == '*') {
			l = l * expr1();
		} else if (sym == '/') {
			n = expr1();
			if (n == 0)
				errmsg = ("0�Ŋ��낤�Ƃ���");
			else
				l = l / n;
		} else if (sym == '%') {
			n = expr1();
			if (n == 0)
				errmsg = ("0�Ŋ��낤�Ƃ���");
			else
				l = (lint_t)l % (lint_t)n;
		} else {
			break;
		}
	}
	return l;
}

double __fastcall MyCalc::expr3()
{
	val_t l;

	l = expr2();
	for (; ;) {
		if (sym == '+') {
			l = l + expr2();
		} else if (sym == '-') {
			l = l - expr2();
		} else {
			break;
		}
	}
	return l;
}

double __fastcall MyCalc::expr4()
{
	val_t l;

	l = expr3();
	for (; ;) {
		if (sym == '&') {
			l = (lint_t)l & (lint_t)expr3();
		} else {
			break;
		}
	}
	return l;
}

double __fastcall MyCalc::expr5()
{
	val_t l;

	l = expr4();
	for (; ;) {
		if (sym == '^') {
			l = (lint_t)l ^ (lint_t)expr4();
		} else {
			break;
		}
	}
	return l;
}

double __fastcall MyCalc::expr6()
{
	val_t l;

	l = expr5();
	for (; ;) {
		if (sym == '|') {
			l = (lint_t)l | (lint_t)expr5();
		} else {
			break;
		}
	}
	return l;
}


double __fastcall MyCalc::expr7()
{
	val_t l;

	l = expr6();
	for (; ;) {
		if (sym == CC('<','<')) {
			l = (lint_t)l << (int)expr6();
		} else if (sym == CC('>','>')) {
			l = (lint_t)l >> (int)expr6();
		} else {
			break;
		}
	}
	return l;
}

double __fastcall MyCalc::expr8()
{
	val_t l;

	l = expr7();
	for (; ;) {
		if (sym == '>') {
			l = (l > expr7());
		} else if (sym == '<') {
			l = (l < expr7());
		} else if (sym == CC('>','=')) {
			l = (l >= expr7());
		} else if (sym == CC('<','=')) {
			l = (l <= expr7());
		} else {
			break;
		}
	}
	return l;
}

double __fastcall MyCalc::expr9()
{
	val_t l;

	l = expr8();
	for (; ;) {
		if (sym == CC('=','=')) {
			l = (l == expr8());
		} else if (sym == CC('!','=')) {
			l = (l != expr8());
		} else {
			break;
		}
	}
	return l;
}

double __fastcall MyCalc::expr10()
{
	val_t l;
	int  n;

	l = expr9();
	if (sym == '=') {
		if (vari_flg) {
			n = vari_no;
			l = vari[n] = expr9();
		} else {
			errmsg = ("����̍��ӂ���������");
		}
	}
	return l;
}


double __fastcall MyCalc::Expr(char *s)
{
	errmsg = NULL;
	ch	 = 0;
	ch_p = s;
	sym	 = 0;
	sym_val	= 0;
	if (*s == 0)
		return 0;
	if (*s == '!') {	/* �V�F�����s */
		return system(s + 1);
	}
	return ans = Val = expr10();
}

