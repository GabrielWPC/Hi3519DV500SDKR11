Notice:
1. If the module parameter "mem_process_isolation" of the MMZ is set to "1", the process isolation attribute of the MMZ buffer is enabled.
When you run the sample and then run the tools, tools may fail to run due to process isolation problems. In consideration of this,
the "share all" operation is reserved in the sample, you can set "MEM_SHARE" to "y" during compilation to enable this function,
such as "make MEM_SHARE=y".

2 sample_vio 7 and sample_mcf 2~5 depend i2c4 && i2c5, EMMC 8it not support use i2c5, the circuit configuration needs to be modified and i2c5 pin_mux needs to be enabled.
3 i2c5 pin_mux cfg:
    bspmm 0x0EFF0028 0x1752;
    bspmm 0x0EFF0024 0x1752;
4 HI3516DV500 not support mipi_rx(4lane + 4lane), the following samples will fail to run due to mipi_rx default configuration: os04a10(4lane) + os04a10(4lane).
    sample_vio 7
    sample_stitch 0~3
    sample_aiisp 10
    sample_mcf 2~5
