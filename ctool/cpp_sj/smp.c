#include <stdio.h>

int main(void)
{
	// ソ
	printf("ソ");
	printf("表示\n");
	return 0;
}

/*
  > gcc test.c
  で出来た a.exe を実行すると
  侮ｦ
  のように表示される(正確にはｦは別の文字コード)。

  > gcc -D_CPP_SJ test.c
  で出来た a.exe を実行すると
  ソ表示
  と表示される。
*/

