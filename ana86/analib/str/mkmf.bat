plcf -f*.ana -- -+ $t$x.obj$t& >myl.lnk
rem plcf -f*.as  -- -+ $t$x.obj$t& >>myl.lnk
type mk1 >makefile
plcf -f*.ana -- $t$x.obj$t\ | ectab -ml >>makefile
rem plcf -f*.as  -- $t$x.obj$t\ | ectab -ml >>makefile
type mk2 >>makefile
plcf -f*.ana -- $x.obj$t: $x.ana | ectab -ml >>makefile
rem plcf -f*.as  -- $x.obj$t: $x.as  | ectab -ml >>makefile
