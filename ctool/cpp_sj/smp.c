#include <stdio.h>

int main(void)
{
	// �\
	printf("�\");
	printf("�\��\n");
	return 0;
}

/*
  > gcc test.c
  �ŏo���� a.exe �����s�����
  ���
  �̂悤�ɕ\�������(���m�ɂͦ�͕ʂ̕����R�[�h)�B

  > gcc -D_CPP_SJ test.c
  �ŏo���� a.exe �����s�����
  �\�\��
  �ƕ\�������B
*/

