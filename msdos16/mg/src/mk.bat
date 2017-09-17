cd alib
call mk.bat
cd ..

call ana2obj.bat mg
call ana2obj.bat cmdlineb
call ana2obj.bat cnvg
call ana2obj.bat q0
call ana2obj.bat b1
call ana2obj.bat b1_sub
call ana2obj.bat b1_read
call ana2obj.bat b1_wrt
call ana2obj.bat bmp_read
call ana2obj.bat bmp_wrt
call ana2obj.bat mag_read
call ana2obj.bat mag_wrt
call ana2obj.bat pi_read
call ana2obj.bat pi_wrt
call ana2obj.bat omake
call ana2obj.bat tif_read
call ana2obj.bat tif_wrt
call ana2obj.bat tif_sub
call ana2obj.bat pmt
call ana2obj.bat djp
call ana2obj.bat gh_read
call ana2obj.bat gh_init
call ana2obj.bat gh_seek
call ana2obj.bat gh_readb
call ana2obj.bat gh_write
call ana2obj.bat gh_wrtb
call ana2obj.bat gh_mem
call ana2obj.bat gh_sub
call ana2obj.bat gh_palbf
call ana2obj.bat gh_gtpal
call ana2obj.bat gh_prinf
call ana2obj.bat gh_deb
call ana2obj.bat gh_a2r
call ana2obj.bat fnm_fsiz
call ana2obj.bat usage

wlink File mg.obj,cmdlineb.obj,cnvg.obj,q0.obj,b1.obj,b1_sub.obj,b1_read.obj,b1_wrt.obj,bmp_read.obj,bmp_wrt.obj,mag_read.obj,mag_wrt.obj,pi_read.obj,pi_wrt.obj,omake.obj,tif_read.obj,tif_wrt.obj,tif_sub.obj,pmt.obj,djp.obj,gh_read.obj,gh_init.obj,gh_seek.obj,gh_readb.obj,gh_write.obj,gh_wrtb.obj,gh_mem.obj,gh_sub.obj,gh_palbf.obj,gh_gtpal.obj,gh_prinf.obj,gh_deb.obj,gh_a2r.obj,fnm_fsiz.obj,usage.obj,alib\alibs.lib Name mg.exe form Dos

md asm
move *.asm asm\
md obj
move *.obj obj\
