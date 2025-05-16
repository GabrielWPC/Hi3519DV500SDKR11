This document applies only to socket and demo boards. The usage method is as follows:

AMP is controlled by GPIO2_1 multiplexed with uart and spi. If you need audio output from audio line out, the AMP must be enabled.
1. select GPIO2_1 to control SGM8903
	You need to manually configure the DIP switches SW6300:
	1) SW6300 bit[1] set to 1
	2) SW6300 bit[3] set to 1

2. unmute SGM8903
	when load sys_config.ko, the amp_unmute_pin_mux function will be called to unmute the SGM8903.
