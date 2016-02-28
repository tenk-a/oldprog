#include <string.h>
#include <ctype.h>
#include <jctype.h>
/*#include "def.h"*/

int FIL_DirName(char far *path, char far *dir)
{
	int p,sep,c;

	sep = p = 0;
	while ((c = path[p++]) != 0) {
		if (c == '\\' || c == '/'||c == ':') {
			sep = p;
		}
	}
	p = 0;
	if (sep) {
		do {
			dir[p] = path[p];
			p++;
		} while (--sep);
	}
	dir[p] = 0;
	return 0;
}
