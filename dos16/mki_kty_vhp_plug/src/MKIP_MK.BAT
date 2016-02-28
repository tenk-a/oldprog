rem [[[ bcc v3 ]]]
bcc -1- -ms -p -G -O -Oa -Oc -Oe -Oi -Ol -Ot -DTCSML mki_plug.c
	rem [[[ tcc v4 ]]]
	rem tcc -1- -ms -p -G -O -DTCSML mki_plug.c noehs.lib
