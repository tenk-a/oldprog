ana -c -o tstltoa
asm tstltoa,tstltoa,NUL,NUL
ana -c -o ltoa
asm ltoa,ltoa,NUL,NUL
tlink tstltoa ltoa putstr
