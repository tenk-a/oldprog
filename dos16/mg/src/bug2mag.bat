@echo off
if exist %1 goto RU
echo usage: bug2mag errmag[.mag]
echo 	v0.98�ȑO��mg���됶������ 256�FMAG�𐶐����Ȃ���.
echo 	�w�肷��̧�ق͌됶��256�FMAG�摜�݂̂Ő���ȉ摜���w�肵�Ȃ�����.
goto :END
:RU
mg -mb .mag.pmt %1
mg -skPMT .pmt.mag %1
del *.pmt
:END
