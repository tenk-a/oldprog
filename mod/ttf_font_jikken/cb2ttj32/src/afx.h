/*
	ttfstub.cpp をMFCを使わずに再コンパイルするための
	強引な辻褄合わせヘッダ
*/
#ifndef AFX_H
#define AFX_H

class CObject {
public:
	CObject() {}
	~CObject() {}
};

#include <stdio.h>
#include <io.h>
#define _filelength		filelength
#endif
