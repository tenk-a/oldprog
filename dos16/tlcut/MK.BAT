tcc -mc -O -G -d -DWILDC -DFBUF tlcut.c tree.c direntry.obj wildcmp.obj
echo off
rem TC �ȊO�ź��߲ق���ꍇ��, -DWILDC direntry.obj wildcmp.obj ���w�肵�Ȃ�
rem (ܲ��޶���ٰ�݂��g��Ȃ�����)
rem �X���[���E���f���ŃR���p�C������ꍇ�� -DFBUF ���w�肵�Ȃ�
