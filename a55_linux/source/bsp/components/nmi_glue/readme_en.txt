[nmi_glue] 
1. Makefile Usage Instructions: 
	(1)Compilation: 
		notice：The kernel needs to be compiled in advance. 
		Execute in the current directory
		make
		The nmi_glue.ko generated in the current directory is the ko file for this module
	(2)Clean:
		make clean
2. Module Usage Instructions: 
	This module supports FIQ function and needs to be used with ot_wdt.ko to monitor system exceptions and complete the action of capturing exceptions. 
	Place the compiled ko on the single board.
Run:
	insmod nmi_glue.ko 
	insmod ot_wdt.ko fiq_support=1 
	(fiq_support parameter is the parameter used to enable fiq for wdg.)
3. NOTICE: 
	(1) Adding fiq_support=1 is a necessary condition for inserting ot_wdt.ko directly. However, this may trigger a watchdog interrupt and cause the system to reboot. 
	(2) ot_wdt.ko depends on ot_osal.ko, so the osal module must be inserted first to use it correctly.