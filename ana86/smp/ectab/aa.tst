# rules		
ASM = TASM		
ANA = ANA	　　　　  		
LINK = tlink	
AFLAGS = -m -ml	
ANAFLAGS =	
LINKFLAGS =		   
.asm.obj:	             	
	$(ASM)　�?$(AFLAGS) $<
.ana.asm:　
	$(ANA) $(ANAFLAGS) -a $<
.ana.obj:
	$(ANA) $(ANAFLAGS) -a$*.asm $<
	$(ASM) $(AFLAGS) $*.asm
	del $*.asm
.anp.obj:
	cpp -o$*.ana $<
	$(ANA) $(ANAFLAGS) -a $*.ana
	$(ASM) $(AFLAGS) $*.asm      	  