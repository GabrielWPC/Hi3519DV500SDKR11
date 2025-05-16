[nmi_glue]
1、Makefile使用说明
	（1）编译：
		需提前编译好内核
		当前目录make
		在当前目录下生成的nmi_glue.ko即为该模块的ko
	（2）clean:
		当前目录make clean
2、模块使用说明
	该模块可支持FIQ功能，需配合ot_wdt.ko一起使用，可实现监测系统异常，完成异常抓取的动作
将编译好的ko放置到单板上
	insmod nmi_glue.ko
	insmod ot_wdt.ko fiq_support=1
	（fiq_support参数为使wdg支持fiq的参数）
3、注意事项
	（1）ot_wdt.ko添加fiq_support=1为必要条件，直接插入，会因为触发watchdog的中断而导致fiq发生，导致系统重启
	（2）ot_wdt,ko依赖于ot_osal.ko,需先插入osal模块才能正确使用