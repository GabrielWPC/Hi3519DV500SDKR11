linux:
1. ir_auto need to config pin_mux, it's conflict with vo_intf. when insmod sys_config.ko, need to config vo_intf to be no RGB type, for example:
./load3519dv500 -i -vo_intf bt1120
The default of -ir_auto  is 1, if do not need to open it, can close, eg:
./load3519dv500 -i -vo_intf bt1120 -ir_auto 0
2. This sample will open devices like ISP, VPSS and so on first, then run the IR_CUT switch thread. It will run right when the settings are correct.

Using the below commands for more information:
linux:./sample_ir_auto -h

